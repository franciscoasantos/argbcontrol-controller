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

    ledMode = hexToDec(message.data().substring(0, 1));
    if (ledMode == 0) {
      red = hexToDec(message.data().substring(1, 3));
      green = hexToDec(message.data().substring(3, 5));
      blue = hexToDec(message.data().substring(5, 7));
    
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

unsigned int hexToDec(String hexString) {

  unsigned int decValue = 0;
  int nextInt;

  for (int i = 0; i < hexString.length(); i++) {

    nextInt = int(hexString.charAt(i));
    if (nextInt >= 48 && nextInt <= 57) nextInt = map(nextInt, 48, 57, 0, 9);
    if (nextInt >= 65 && nextInt <= 70) nextInt = map(nextInt, 65, 70, 10, 15);
    if (nextInt >= 97 && nextInt <= 102) nextInt = map(nextInt, 97, 102, 10, 15);
    nextInt = constrain(nextInt, 0, 15);

    decValue = (decValue * 16) + nextInt;
  }

  return decValue;
}
