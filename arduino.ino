#include <WiFi.h>
#include <Adafruit_NeoPixel.h>
#include <ArduinoWebsockets.h>
#include <ESPDateTime.h>

using namespace websockets;

#define PIN 13
#define NUM 20

const char* ssid = "xpto";
const char* password = "qwer@1234";
const char* webSocketsServerHost = "services.franciscosantos.net";
const int webSocketsServerPort = 3000;

Adafruit_NeoPixel fita = Adafruit_NeoPixel(NUM, PIN, NEO_GRB + NEO_KHZ800);
WebsocketsClient client;
int ledMode, red, green, blue;

void setup() {
  Serial.begin(115200);
  pinMode(PIN, OUTPUT);

  client.onMessage([&](WebsocketsMessage message)
  {
    Serial.print("Mensagem recebida: ");
    Serial.println(message.data());

    ledMode = message.data().substring(0, 1).toInt();
    if (ledMode == 0) {
      red = message.data().substring(1, 4).toInt();
      green = message.data().substring(4, 7).toInt();
      blue = message.data().substring(7, 10).toInt();
    
      setColor(red, green, blue);
    }
  });
}

int seconds = -10;
void loop() {
  if (DateTime.getTime() - 5 >= seconds) {
    VerifyConnections();
    seconds = DateTime.getTime();
  }
  client.poll();
}

void VerifyConnections() {
  if (WiFi.status() != WL_CONNECTED)
    ConnectWifi();

  if (!client.available())
    ConnectServer();
}

void ConnectWifi() {
  Serial.println("Connecting WiFi");
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("Connected to WiFi network!");
}

void ConnectServer() {
  bool connected = client.connect(webSocketsServerHost, webSocketsServerPort, "/?clientId=0");

  if (connected)
    Serial.println("WebSockets Connected!");
  else
    Serial.println("WebSockets Not Connected!");

  return;
}

void setColor(int r, int g, int b) {
  for (int i = 0; i < NUM; i++) {
    fita.setPixelColor(i, r, g, b);
  }
  fita.show();
}
