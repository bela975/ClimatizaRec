#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "Adafruit_HTU21DF.h"
#include <Adafruit_APDS9960.h>
#include <WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <FirebaseESP32.h> // BIBLIOTECA CORRETA PARA TOKEN LEGADO

// === Wi-Fi ===
#define WIFI_SSID "Claro_2202-B"
#define WIFI_PASSWORD "lizie2024"

// === Firebase ===
#define DATABASE_URL "https://climatizarecife-2025-default-rtdb.firebaseio.com/"
#define DATABASE_SECRET "CBuc7JuanfWmscKLyIAJyOBoTA0k46PzBar8FJu4" // TOKEN LEGADO

// === OLED ===
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// === Sensores ===
Adafruit_HTU21DF htu;
Adafruit_APDS9960 apds;

// === LED RGB ===
#define RED_LED_PIN    19
#define GREEN_LED_PIN  23
#define BLUE_LED_PIN   18

// === Firebase ===
FirebaseData firebaseData;
FirebaseConfig config;

// === NTP ===
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", -10800, 60000);

bool presencaDetectada = false;

void enviarDadosFirebase(float temp, float umid, bool presenca) {
  timeClient.update();
  String timestamp = timeClient.getFormattedTime();
  String path = "/leituras/" + timestamp;

  bool enviado = true;

  enviado &= Firebase.setFloat(firebaseData, path + "/temperatura", temp);
  if (!enviado) Serial.println("Falha ao enviar temperatura: " + firebaseData.errorReason());

  enviado &= Firebase.setFloat(firebaseData, path + "/umidade", umid);
  if (!enviado) Serial.println("Falha ao enviar umidade: " + firebaseData.errorReason());

  enviado &= Firebase.setString(firebaseData, path + "/presenca", presenca ? "presente" : "ausente");
  if (!enviado) Serial.println("Falha ao enviar presença: " + firebaseData.errorReason());

  enviado &= Firebase.setString(firebaseData, path + "/hora", timestamp);
  if (!enviado) Serial.println("Falha ao enviar hora: " + firebaseData.errorReason());

  if (enviado) {
    Serial.println("Dados enviados ao Firebase com sucesso!");
  } else {
    Serial.println("Erro: nem todos os dados foram enviados ao Firebase.");
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(RED_LED_PIN, OUTPUT);
  pinMode(GREEN_LED_PIN, OUTPUT);
  pinMode(BLUE_LED_PIN, OUTPUT);
  digitalWrite(RED_LED_PIN, LOW);
  digitalWrite(GREEN_LED_PIN, LOW);
  digitalWrite(BLUE_LED_PIN, LOW);

  Wire.begin();

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("Falha no display OLED!"));
    while (true);
  }

  if (!htu.begin()) {
    Serial.println("Sensor HTU21D não detectado!");
    while (true);
  }

  if (!apds.begin()) {
    Serial.println("Sensor APDS-9960 não detectado!");
    while (true);
  }
  apds.enableGesture(true);

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Conectando ao Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println(" Conectado!");

  config.database_url = DATABASE_URL;
  config.signer.tokens.legacy_token = DATABASE_SECRET;

  Firebase.begin(&config, nullptr);
  Firebase.reconnectNetwork(true);

  timeClient.begin();

  delay(2000);
  Serial.println("Firebase configurado!");
}

void loop() {
  uint8_t gesture = apds.readGesture();
if (gesture != 0) {
  switch (gesture) {
    case APDS9960_UP:
    case APDS9960_LEFT:
      presencaDetectada = false;
      Serial.println("Gesto: AUSENTE (UP/LEFT)");
      break;
    case APDS9960_DOWN:
    case APDS9960_RIGHT:
      presencaDetectada = true;
      Serial.println("Gesto: PRESENCA (DOWN/RIGHT)");
      break;
    default:
      Serial.println("Gesto nao reconhecido");
      break;
  }
}

  float temp = htu.readTemperature();
  float umid = htu.readHumidity();

  bool ligarLED = (temp >= 23.0 && umid >= 40.0 && presencaDetectada);

  digitalWrite(RED_LED_PIN, ligarLED ? HIGH : LOW);
  digitalWrite(GREEN_LED_PIN, LOW);
  digitalWrite(BLUE_LED_PIN, LOW);

  display.clearDisplay();
  display.setCursor(0, 0);
  display.printf("Temp: %.1f C\n", temp);
  display.printf("Umid: %.1f %%\n", umid);
  display.printf("Presenca: %s\n", presencaDetectada ? "Detectada" : "Nao");
  display.printf("LED: %s", ligarLED ? "ON" : "OFF");
  display.display();

  Serial.print("Leitura - Temp: ");
  Serial.print(temp);
  Serial.print(" C | Umidade: ");
  Serial.print(umid);
  Serial.print(" % | Presenca: ");
  Serial.println(presencaDetectada ? "Sim" : "Nao");

  enviarDadosFirebase(temp, umid, presencaDetectada);

delay(100);
}