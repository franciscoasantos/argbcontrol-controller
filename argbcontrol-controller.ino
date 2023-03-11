#include <WiFi.h>
#include <Adafruit_NeoPixel.h>
#include <ArduinoWebsockets.h>
#include <HTTPClient.h>

using namespace websockets;

/* Configurações de conexão */
const char* ssid = "xpto";
const char* password = "qwer@1234";

/* Configurações de servidor */
const String serverHost = "services.franciscosantos.net:5000";
const String webSocketPath = "/";
const String authPath = "/api/authentication/token";

/* Configurações do dispositivo */
const String secret = "9a02d1e835264f6fa7f3d0ede49cea5a";

/* [Pin, Qtd. Leds] */
const int bed[] = {13, 300};
const int tableA[] = {12, 89};
const int tableB[] = {14, 70};

/* Instância das Fitas de led */
Adafruit_NeoPixel ledBed = Adafruit_NeoPixel(bed[1], bed[0], NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel ledTableA = Adafruit_NeoPixel(tableA[1], tableA[0], NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel ledTableB = Adafruit_NeoPixel(tableB[1], tableB[0], NEO_GRB + NEO_KHZ800);

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

String Authenticate() {
  HTTPClient http;

  while (true) {
    http.begin("http://" + serverHost + authPath);
    http.addHeader("Authorization", secret);

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
  }
  http.end();
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
  String token = Authenticate();

  bool connected = client.connect("ws://" + serverHost + webSocketPath + "?token=" + token);

  if (connected) {
    Serial.println("WebSockets Connected!");
  }
  else {
    Serial.println("WebSockets Not Connected!");
  }

}

void ProcessMessage(String message) {
  vTaskSuspend(fadeHandle);
  vTaskSuspend(rainbowHandle);
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
      ledBed.rainbow(firstPixelHue);
      ledTableA.rainbow(firstPixelHue);
      ledTableB.rainbow(firstPixelHue);

      ledBed.show();
      ledTableA.show();
      ledTableB.show();

      delay(delayChange);
    }
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
