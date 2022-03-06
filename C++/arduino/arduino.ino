#include <WiFi.h>
#include <Adafruit_NeoPixel.h>
#include <ArduinoWebsockets.h>
#include <ESPDateTime.h>

using namespace websockets;

/* Configurações de conexão */
const char* ssid = "xpto";
const char* password = "qwer@1234";
const char* webSocketsServerHost = "services.franciscosantos.net";
const int webSocketsServerPort = 3000;
WebsocketsClient client;

/* [Pin, Qtd. Leds] */
const int bed[] = {13, 107};
const int tableA[] = {12, 89};
const int tableB[] = {14, 70};

/* Instância das Fitas de led */
Adafruit_NeoPixel ledBed = Adafruit_NeoPixel(bed[1], bed[0], NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel ledTableA = Adafruit_NeoPixel(tableA[1], tableA[0], NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel ledTableB = Adafruit_NeoPixel(tableB[1], tableB[0], NEO_GRB + NEO_KHZ800);

/* [modo, r, g, g] */
/* 0 = Estático | 1 = Fade */
String previousState[4];
String currentState[4];
int seconds = -5;

/* Task Handles */
TaskHandle_t fadeHandle;

/* Variáveis de controle do modo fade */
int r, g, b;
int increase, delayChange;

void setup() {
  Serial.begin(115200);

  pinMode(bed[0], OUTPUT);
  pinMode(tableA[0], OUTPUT);
  pinMode(tableB[0], OUTPUT);

  ledBed.begin();
  ledBed.setBrightness(255);
  ledTableA.begin();
  ledTableA.setBrightness(255);
  ledTableB.begin();
  ledTableB.setBrightness(255);

  xTaskCreatePinnedToCore(Fade, "Fade Task", 1024, NULL, 1, &fadeHandle, 1);

  client.onMessage([&](WebsocketsMessage message)
  {
    Serial.print("Mensagem recebida: ");
    Serial.println(message.data());

    ProcessMessage(message.data());
  });
}

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

void ProcessMessage(String message) {
  currentState[0] = message.substring(0, 1);

  switch (currentState[0].toInt()) {
    case 0:
      vTaskSuspend(fadeHandle);
      currentState[1] = message.substring(1, 4);
      currentState[2] = message.substring(4, 7);
      currentState[3] = message.substring(7, 10);
      if (previousState != currentState) {
        previousState[0] = currentState[0];
        previousState[1] = currentState[1];
        previousState[2] = currentState[2];
        previousState[3] = currentState[3];

        setColor(currentState[1].toInt(), currentState[2].toInt(), currentState[3].toInt());
      }
    case 1:
      r = 255;
      g = 0;
      b = 0;
      increase = message.substring(1, 3).toInt();
      delayChange = message.substring(3, 6).toInt();

      vTaskResume(fadeHandle);
  }
}

void Fade(void * parameter) {
  while (true) {
    if (r > 0 && b == 0) {
      r -= increase;
      g += increase;
    }
    if (g > 0 && r == 0) {
      g -= increase;
      b += increase;
    }
    if (b > 0 && g == 0) {
      r += increase;
      b -= increase;
    }
    setColor(r, g, b);
    delay(delayChange);
  }
}

void setColor(int r, int g, int b) {
  ledBed.fill(ledBed.Color(r, g, b), 0);
  ledTableA.fill(ledTableA.Color(r, g, b), 0);
  ledTableB.fill(ledTableB.Color(r, g, b), 0);

  ledBed.show();
  ledTableA.show();
  ledTableB.show();
}
