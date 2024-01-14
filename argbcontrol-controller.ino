#include <WiFi.h>
#include <Adafruit_NeoPixel.h>
#include <ArduinoWebsockets.h>
#include <HTTPClient.h>

using namespace websockets;

/* Configurações de conexão */
const char* ssid = "xpto";
const char* password = "qwer@1234";

/* Configurações de servidor */
const String serverHost = "144.217.94.131:5000";
const String webSocketPath = "";
const String authPath = "/api/token";

/* Configurações do dispositivo */
const String clientId = "u8lf0zhHv0aEpHRbPMaAiA";
const String clientSecret = "9a02d1e835264f6fa7f3d0ede49cea5a";

/* [Pin, Qtd. Leds] */
const int strip1[] = {13, 300};
const int strip2[] = {12, 300};

/* Instância das Fitas de led */
Adafruit_NeoPixel ledStrip1 = Adafruit_NeoPixel(strip1[1], strip1[0], NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel ledStrip2 = Adafruit_NeoPixel(strip2[1], strip2[0], NEO_GRB + NEO_KHZ800);

/* Task Handles */
TaskHandle_t fadeHandle;
TaskHandle_t rainbowHandle;

/* Variáveis de controle do modo fade */
int r, g, b;
int increase, delayChange;

/* Variáveis de controle da verificação de conexão */
/* 100 ~ 1seg */
int loops = 500;
int count = loops;
int restartAt = 1440;
int restartCount = 0;

WebsocketsClient client;

void setup() {
  Serial.begin(115200);

  pinMode(strip1[0], OUTPUT);
  pinMode(strip2[0], OUTPUT);

  ledStrip1.begin();
  ledStrip1.setBrightness(255);
  ledStrip2.begin();
  ledStrip2.setBrightness(255);

  xTaskCreatePinnedToCore(Fade, "Fade Task", 1024, NULL, 1, &fadeHandle, 1);
  vTaskSuspend(fadeHandle);
  xTaskCreatePinnedToCore(Rainbow, "Rainbow Task", 1024, NULL, 1, &rainbowHandle, 1);
  vTaskSuspend(rainbowHandle);

  client.onMessage([&](WebsocketsMessage message)
  {
    Serial.print("Mensagem recebida: ");
    Serial.println(message.data());

    ProcessMessage(message.data());
  });
}

void loop() {
  if (count >= loops) {
    VerifyConnections();
    restartCount++;
    count = 0;

    if (restartCount >= restartAt) {
      Serial.println("Restarting...");
      ESP.restart();
    }
  }

  count++;
  client.poll();
  delay(10);
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
  String token = GetAuthToken();

  bool connected = client.connect("ws://" + serverHost + webSocketPath + "/?client_id=" + clientId + "&access_token=" + token);

  if (connected) {
    Serial.println("WebSockets Connected!");
  }
  else {
    Serial.println("WebSockets Not Connected!");
  }

}

String GetAuthToken() {
  HTTPClient http;

  while (true) {
    http.begin("http://" + serverHost + authPath);
    http.addHeader("X-Client-Secret", clientSecret);
    http.addHeader("X-Client-Id", clientId);

    int httpResponseCode = http.GET();

    if (httpResponseCode == 200)
    {
      return http.getString();
    }
    else
    {
      Serial.print("Error code: ");
      Serial.println(httpResponseCode);
    }
    delay(1000);
  }
  http.end();
}

void ProcessMessage(String message) {

  SuspendTasks();

  switch (message.substring(0, 1).toInt()) {
    case 0:
      setColor(message.substring(1, 4).toInt(), message.substring(4, 7).toInt(), message.substring(7, 10).toInt());
      break;
    case 1:
      r = 255;
      g = 0;
      b = 0;
      increase = message.substring(1, 3).toInt();
      delayChange = message.substring(3, 6).toInt();
      vTaskResume(fadeHandle);
      break;
    case 2:
      delayChange = message.substring(1, 5).toInt();
      vTaskResume(rainbowHandle);
      break;
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

void Rainbow(void * parameter) {
  while (true) {
    for (long firstPixelHue = 0; firstPixelHue < 5 * 65536; firstPixelHue += 256) {
      ledStrip1.rainbow(firstPixelHue);
      ledStrip2.rainbow(firstPixelHue);

      ledStrip1.show();
      ledStrip2.show();

      delay(delayChange);
    }
  }
}

void setColor(int r, int g, int b) {
  ledStrip1.fill(ledStrip1.Color(r, g, b), 0);
  ledStrip2.fill(ledStrip2.Color(r, g, b), 0);

  ledStrip1.show();
  ledStrip2.show();
}

void SuspendTasks() {
  vTaskSuspend(fadeHandle);
  vTaskSuspend(rainbowHandle);
}
