#include <WiFi.h>
#include <Adafruit_NeoPixel.h>
#include <ArduinoWebsockets.h>
#include <ArduinoJson.h>
#include <ESPDateTime.h>

using namespace websockets;

#define PIN 13
#define NUM 20

const char* ssid = "xpto";
const char* password = "qwer@1234";
const char* webSocketsServerHost = "services.franciscosantos.net";
const int webSocketsServerPort = 3000;

WebsocketsClient client;
StaticJsonDocument<100> doc;
Adafruit_NeoPixel fita = Adafruit_NeoPixel(NUM, PIN, NEO_GRB + NEO_KHZ800);

void setup() {
  Serial.begin(115200);
  pinMode(PIN, OUTPUT);

  client.onMessage([&](WebsocketsMessage message)
  {
    Serial.print("Mensagem recebida: ");
    Serial.println(message.data());

    DeserializationError error = deserializeJson(doc, message.data());
    if (error) {
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(error.f_str());
      return;
    }

    int red = doc["R"].as<int>();
    int green = doc["G"].as<int>();
    int blue = doc["B"].as<int>();

    setColor(red, green, blue);
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
