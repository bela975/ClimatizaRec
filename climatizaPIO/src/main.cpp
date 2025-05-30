#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "Adafruit_HTU21DF.h"
#include <SparkFun_APDS9960.h>
#include <WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <FirebaseESP32.h>

// === Wi-Fi ===
#define WIFI_SSID "Galaxy A54 5G 3B44"
#define WIFI_PASSWORD "teamobeijos"

// === Firebase ===
#define DATABASE_URL "https://climatizarecife-2025-default-rtdb.firebaseio.com/"
#define DATABASE_SECRET "CBuc7JuanfWmscKLyIAJyOBoTA0k46PzBar8FJu4"

// === OLED ===
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// === Sensores ===
Adafruit_HTU21DF htu;
SparkFun_APDS9960 apds = SparkFun_APDS9960();

// === LED RGB ===
#define RED_LED_PIN    19
#define GREEN_LED_PIN  23
#define BLUE_LED_PIN   18

// === FAN / RELÉ ===
#define RELAY_PIN 33

// === Firebase ===
FirebaseData firebaseData;
FirebaseConfig config;

// === NTP ===
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", -10800, 60000);

bool presencaDetectada = false;

// === NOVO: Variáveis de override ===
bool overrideControl = false;
bool overrideTurnOn = false;

void enviarDadosFirebase(float temp, float umid, bool presenca) {
  timeClient.update();
  String timestamp = timeClient.getFormattedTime();
  String path = "/leituras/" + timestamp;

  bool enviado = true;

  enviado &= Firebase.setFloat(firebaseData, path + "/temperatura", temp);
  enviado &= Firebase.setFloat(firebaseData, path + "/umidade", umid);
  enviado &= Firebase.setString(firebaseData, path + "/presenca", presenca ? "presente" : "ausente");
  enviado &= Firebase.setString(firebaseData, path + "/hora", timestamp);

  if (enviado) {
    Serial.println("Dados enviados ao Firebase com sucesso!");
  } else {
    Serial.println("Erro ao enviar dados: " + firebaseData.errorReason());
  }
}

void lerOverrideFirebase() {  // === NOVO: Função para ler o override ===
  if (Firebase.getBool(firebaseData, "/control/override")) {
    overrideControl = firebaseData.boolData();
  } else {
    Serial.println("Erro ao ler override: " + firebaseData.errorReason());
  }

  if (Firebase.getBool(firebaseData, "/control/turnOn")) {
    overrideTurnOn = firebaseData.boolData();
  } else {
    Serial.println("Erro ao ler turnOn: " + firebaseData.errorReason());
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(RED_LED_PIN, OUTPUT);
  pinMode(GREEN_LED_PIN, OUTPUT);
  pinMode(BLUE_LED_PIN, OUTPUT);
  pinMode(RELAY_PIN, OUTPUT);

  digitalWrite(RED_LED_PIN, LOW);
  digitalWrite(GREEN_LED_PIN, LOW);
  digitalWrite(BLUE_LED_PIN, LOW);
  digitalWrite(RELAY_PIN, LOW);

  Wire.begin();

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("Falha no display OLED!"));
    while (true);
  }

  if (!htu.begin()) {
    Serial.println("Sensor HTU21D não detectado!");
    while (true);
  }

  if (apds.init()) {
    Serial.println("APDS-9960 iniciado com sucesso.");
    if (apds.enableGestureSensor(true)) {
      Serial.println("Sensor de gesto ativado.");
    } else {
      Serial.println("Falha ao ativar sensor de gesto.");
    }
  } else {
    Serial.println("Sensor APDS-9960 não detectado!");
    while (true);
  }

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

  if (Firebase.deleteNode(firebaseData, "/leituras")) {
    Serial.println("Todas as leituras anteriores foram deletadas com sucesso.");
  } else {
    Serial.println("Falha ao deletar leituras anteriores: " + firebaseData.errorReason());
  }

  timeClient.begin();

  delay(2000);
  Serial.println("Firebase configurado!");
}

void loop() {
  // === NOVO: ler override antes de tudo ===
  lerOverrideFirebase();

  if (apds.isGestureAvailable()) {
    int gesture = apds.readGesture();
    switch (gesture) {
      case DIR_UP:
      case DIR_LEFT:
        presencaDetectada = false;
        Serial.println("Gesto: AUSENTE (UP/LEFT)");
        break;
      case DIR_DOWN:
      case DIR_RIGHT:
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

  bool ligarLED = false;

  if (overrideControl) {  
    // === NOVO: Se override, usa comando do Firebase
    ligarLED = overrideTurnOn;
    Serial.println("Override ATIVO: Forçando estado: " + String(ligarLED ? "ON" : "OFF"));
  } else {
    // === Mantém sua lógica original
    ligarLED = (temp >= 19.0 && umid >= 40.0 && presencaDetectada);
  }

  digitalWrite(RED_LED_PIN, ligarLED ? HIGH : LOW);
  digitalWrite(GREEN_LED_PIN, LOW);
  digitalWrite(BLUE_LED_PIN, LOW);
  digitalWrite(RELAY_PIN, ligarLED ? LOW : HIGH);

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

  delay(1000);  // Melhor para evitar sobrecarga do Firebase
}
