#include <WiFi.h>
#include <Adafruit_NeoPixel.h>
#include <ArduinoWebsockets.h>
#include <ESPDateTime.h>

using namespace websockets;

/* Dados de conexão */
const char* ssid = "xpto";
const char* password = "qwer@1234";
const char* webSocketsServerHost = "services.franciscosantos.net";
const int webSocketsServerPort = 3000;

/* [Pin, Qtd. Leds] */
const int cama[] = {13, 107};
const int mesa1[] = {12, 89};
const int mesa2[] = {14, 70};

/* Instância das Fitas de led */
Adafruit_NeoPixel fitaCama = Adafruit_NeoPixel(cama[1], cama[0], NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel fitaMesa1 = Adafruit_NeoPixel(mesa1[1], mesa1[0], NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel fitaMesa2 = Adafruit_NeoPixel(mesa2[1], mesa2[0], NEO_GRB + NEO_KHZ800);
WebsocketsClient client;

/* [modo, r, g, g] */
/* 0 = Estático | 1 = Fade */
int corAnterior[] = {0, 0, 0, 0};
int cor[] = {0, 0, 0, 0};
int seconds = -5;

//Task Handles
TaskHandle_t fadeHandle;

void setup() {
  Serial.begin(115200);

  pinMode(cama[0], OUTPUT);
  pinMode(mesa1[0], OUTPUT);
  pinMode(mesa2[0], OUTPUT);

  fitaCama.begin();
  fitaCama.setBrightness(255);
  fitaMesa1.begin();
  fitaMesa1.setBrightness(255);
  fitaMesa2.begin();
  fitaMesa2.setBrightness(255);

  client.onMessage([&](WebsocketsMessage message)
  {
    Serial.print("Mensagem recebida: ");
    Serial.println(message.data());

    cor[0] = message.data().substring(0, 1).toInt();
    cor[1] = message.data().substring(1, 4).toInt();
    cor[2] = message.data().substring(4, 7).toInt();
    cor[3] = message.data().substring(7, 10).toInt();

    if (cor[0] == 0) {
      if (corAnterior != cor) {
        corAnterior[1] = cor[0];
        corAnterior[1] = cor[1];
        corAnterior[2] = cor[2];
        corAnterior[3] = cor[3];

        setColor(cor[1], cor[2], cor[3]);
      }
    }
    else {
      if (fadeHandle == NULL || eTaskGetState(fadeHandle) != 2) {
        xTaskCreatePinnedToCore(Fade, "Fade Task", 8192, NULL, 1, &fadeHandle, 1);
      }
    }
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

void Fade(void * parameter) {
  int r = cor[1];
  int g = cor[2];
  int b = cor[3];
  int increase = 1;
  int fadeDelay = 20;

  while (cor[0] == 1) {
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
    delay(fadeDelay);
  }
  vTaskDelete(NULL);
}

void setColor(int r, int g, int b) {
  fitaCama.fill(fitaCama.Color(r, g, b), 0);
  fitaMesa1.fill(fitaMesa1.Color(r, g, b), 0);
  fitaMesa2.fill(fitaMesa2.Color(r, g, b), 0);

  fitaCama.show();
  fitaMesa1.show();
  fitaMesa2.show();
}
