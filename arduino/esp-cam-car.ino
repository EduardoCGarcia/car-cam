#include<esp32cam.h>
#include <WiFi.h>
#include<WebServer.h>
#include<WiFi.h>

// Definiciones de pines usados para los motores
#define PIN_MOTOR_R_FWD 12
#define PIN_MOTOR_R_BWD 13
#define PIN_MOTOR_L_FWD 15
#define PIN_MOTOR_L_BWD 14
#define FLASH 4

enum Comandos { CMD_FORWARD = 'w', CMD_BACKWARD = 's', CMD_RIGHT = 'd', CMD_LEFT = 'a', CMD_STOP = 'q' }; // Enumeración de comandos recibidos

const char* ssid = "POCO X3 NFC";
const char* password = "09092000";

WiFiServer TCPServer(8266); //Servidor del ESP32
WiFiClient TCPClient; //Cliente TCP (PC)
WebServer server(80);

static auto loRes = esp32cam::Resolution::find(320, 240);
static auto hiRes = esp32cam::Resolution::find(800, 600);

void serveJpg() {
  auto frame = esp32cam::capture();
  if (frame == nullptr)
  {
    Serial.println("CAPTURE FAIL");
    server.send(530, "", "");
    return;
  }
  Serial.printf("CAPTURE OK %dx%d %db\n", frame->getWidth(), frame->getHeight(), static_cast<int>(frame->size()));
  server.setContentLength(frame->size());
  server.send(200, "image/jpeg");
  WiFiClient client = server.client();
  frame->writeTo(client);
}

void handleJpgLo() {
  if (!esp32cam::Camera.changeResolution(loRes))
  {
    Serial.println("SET-LO-RES-FAIL");
  }
  serveJpg();
}

void handleJpgHi() {
  if (!esp32cam::Camera.changeResolution(hiRes))
  {
    Serial.println("SET-HI-RES-FAIL");
  }
  serveJpg();
}

void byteReceived(byte byteRecibido) {

  switch (byteRecibido) {

    case CMD_FORWARD:
      Serial.println("Forward");
      digitalWrite(PIN_MOTOR_R_FWD, LOW);
      digitalWrite(PIN_MOTOR_R_BWD, HIGH);
      digitalWrite(PIN_MOTOR_L_FWD, LOW);
      digitalWrite(PIN_MOTOR_L_BWD, HIGH);

      break;

    case CMD_BACKWARD:

      Serial.println("Backward");
      digitalWrite(PIN_MOTOR_R_FWD, HIGH);
      digitalWrite(PIN_MOTOR_R_BWD, LOW);
      digitalWrite(PIN_MOTOR_L_FWD, HIGH);
      digitalWrite(PIN_MOTOR_L_BWD, LOW);


      break;

    case CMD_RIGHT:

      Serial.println("Right");
      digitalWrite(PIN_MOTOR_R_FWD, HIGH);
      digitalWrite(PIN_MOTOR_R_BWD, LOW);
      digitalWrite(PIN_MOTOR_L_FWD, LOW);
      digitalWrite(PIN_MOTOR_L_BWD, HIGH  );




      break;


    case CMD_LEFT:
      Serial.println("Left");
      digitalWrite(PIN_MOTOR_R_FWD, LOW);
      digitalWrite(PIN_MOTOR_R_BWD, HIGH);
      digitalWrite(PIN_MOTOR_L_FWD, HIGH);
      digitalWrite(PIN_MOTOR_L_BWD, LOW);;

      break;
    case CMD_STOP:

      Serial.println("Stop ");

      digitalWrite(PIN_MOTOR_R_FWD, LOW);
      digitalWrite(PIN_MOTOR_R_BWD, LOW);
      digitalWrite(PIN_MOTOR_L_FWD, LOW);
      digitalWrite(PIN_MOTOR_L_BWD, LOW);

      break;

    default: break;

  }

}

void setup() {
  pinMode(FLASH, OUTPUT);
  
  pinMode(PIN_MOTOR_R_FWD, OUTPUT);
  pinMode(PIN_MOTOR_R_BWD, OUTPUT);
  pinMode(PIN_MOTOR_L_FWD, OUTPUT);
  pinMode(PIN_MOTOR_L_BWD, OUTPUT);

  digitalWrite(PIN_MOTOR_R_FWD, LOW);
  digitalWrite(PIN_MOTOR_R_BWD, LOW);
  digitalWrite(PIN_MOTOR_L_FWD, LOW);
  digitalWrite(PIN_MOTOR_L_BWD, LOW);
  digitalWrite(FLASH, HIGH);
  Serial.begin(115200);
  Serial.println();
  {
    using namespace esp32cam;
    Config cfg;
    cfg.setPins(pins::AiThinker);
    cfg.setResolution(hiRes);
    cfg.setBufferCount(2);
    cfg.setJpeg(80);

    bool ok = Camera.begin(cfg);
    Serial.println(ok ? "CAMERA OK" : "CAMERA FAIL");
  }

  Serial.printf("Conectando a: %s\n", ssid);
  WiFi.persistent(false);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  // Intentamos que se conecte a la red wifi
  while (WiFi.status() != WL_CONNECTED) {
    Serial.println("Conectando...");
    delay(2000);
  }

  Serial.print("Conectado.  ");
  Serial.print("http://");
  Serial.print(WiFi.localIP());
  Serial.println("/cam-lo.jpg");
  server.on("/cam-lo.jpg", handleJpgLo);
  server.on("/cam-hi.jpg", handleJpgHi);
  server.begin();
  
  Serial.print(" Dirección IP del módulo: ");
  Serial.println(WiFi.localIP());

  TCPServer.begin();
}

void loop() {
  server.handleClient();
  if (!TCPClient.connected()) {
    // try to connect to a new client
    TCPClient = TCPServer.available();
  } else {
    // read data from the connected client
    if (TCPClient.available() > 0) {
      byteReceived(TCPClient.read());

    }
  }
  
}
