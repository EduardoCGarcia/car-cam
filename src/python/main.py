import cv2
from urllib import request
import numpy as np
import socket
from math import ceil
import os


ESP_IP = '192.168.8.106'  # IP de nuestro modulo
ESP_PORT = 8266  # puerto que hemos configurado para que abra el ESP
URL_HI = "http://192.168.100.154/cam-hi.jpg"  # url de la imagen en calidad alta
URL_LO = "http://192.168.8.106/cam-lo.jpg"  # url de la imagen en calidad baja

winName = "ESP32CAM"
cv2.namedWindow(winName, cv2.WINDOW_AUTOSIZE)

scale_percent = 80
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM) #creamos el objeto socket
s.connect((ESP_IP, ESP_PORT)) #Nos conectamos a la IP y el puerto que hemos declarado


area_max = np.array([[160, 0], [180, 0], [180, 319], [160, 319]])

while 1:
    imgResponse = request.urlopen(URL_LO)
    imgNp = np.array(bytearray(imgResponse.read()), dtype=np.uint8)
    img = cv2.imdecode(imgNp, -1)
    img = cv2.rotate(img,cv2.ROTATE_180)

    gray = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)
    gray = cv2.GaussianBlur(gray, (5, 5), 0)
    t, binary = cv2.threshold(gray, 0, 255, cv2.THRESH_BINARY+cv2.THRESH_OTSU)

    binary = 255-binary
    opening = cv2.morphologyEx(binary, cv2.MORPH_CLOSE, np.ones((7,7),np.uint8))

    imAux1 = np.zeros(shape=(img.shape[:2]), dtype=np.uint8)
    imAux1 = cv2.drawContours(imAux1, [area_max], -1, (255), -1)
    image_area = cv2.bitwise_and(opening, opening, mask=imAux1)
    image_area = cv2.dilate(image_area, None, iterations=2)
    
    contours,_ = cv2.findContours(image_area.copy(),cv2.RETR_EXTERNAL,cv2.CHAIN_APPROX_SIMPLE)
    cx=0
    cy=0

    for cnt in contours:
        momentos = cv2.moments(cnt)
        area = momentos['m00']
        if (area>800):
            cv2.drawContours(img, [cnt], 0, (0,255,0), 3)
            cx = int(momentos['m10']/momentos['m00'])
            cy = int(momentos['m01']/momentos['m00'])
            isObject = True
    cv2.circle(img,(cx,cy),5,(255,255,0),-1)

    error = img.shape[0]/2-cy
    K = 0.4
    m1=40
    m2=40
    if error > 0:
        m1 = ceil(40 + (error*K))
        m2 = ceil(40 - (error*K)/1.5) 
    elif error < 0:
        m1 = ceil(40 + (error*K)/1.5)
        m2 = ceil(40 - (error*K)) 
    print(m1)
    if m1>=100:
        print("entra 100")
        m1=100
    elif m1<=0:
        print("entra 0")
        m1=0
    if m2>=100:
        m2=100
    elif m2<=0:
        m2=0
        
    #print("m1:"+str(m1)+", m2:"+str(m2)+", Y:"+str(error))
    
    





    cv2.imshow(winName, img)
    tecla = cv2.waitKey(5) & 0xFF

    if tecla == 27:
        break

cv2.destroyAllWindows()
