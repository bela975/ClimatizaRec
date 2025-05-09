#include <Arduino.h>
#include <WiFi.h>
#include <DHT.h>
#include <Firebase_ESP_Client.h>  

// DEFINIÇÕES 
#define WIFI_SSID "uaifai-tiradentes"
#define WIFI_PASSWORD "bemvindoaocesar"

#define API_KEY "CBuc7JuanfWmscKLyIAJyOBoTA0k46PzBar8FJu4" 
#define DATABASE_URL "https://climatizarecife-2025-default-rtdb.firebaseio.com"

// Pinos físicos conectados 
#define DHTPIN        13  // DHT11 dados conectado ao GPIO13 (D13)
#define DHTTYPE       DHT11

#define TEMP_POT_PIN  32    // Potenciômetro de temperatura simulada no GPIO4 (D2)
#define HUM_POT_PIN   33    // Potenciômetro de umidade simulada no GPIO2 (D4)
#define PRES_POT_PIN  34   // Potenciômetro de presença simulada no GPIO15 (D15)

#define FAN_LED_PIN   25     // LED/futura ventoinha no GPIO5

// OBJETOS 
DHT dht(DHTPIN, DHTTYPE);
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

unsigned long previousMillis = 0;
const unsigned long interval = 2000;  // a cada 2 segundos

// SETUP
void setup() {
  Serial.begin(115200); // Taxa de transmissão de dados por segundo

  pinMode(FAN_LED_PIN, OUTPUT);
  digitalWrite(FAN_LED_PIN, LOW);

  dht.begin();

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Conectando ao WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi conectado");

  // Configuração Firebase
  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;

  // Sem autenticação de usuário, usar anônimo
  auth.user.email = "";
  auth.user.password = "";

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
}

// LOOP 
void loop() {
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

    // Leitura do DHT11
    float tempReal = dht.readTemperature();
    float humReal = dht.readHumidity();

    // Leitura dos potenciômetros
    int rawTemp = analogRead(TEMP_POT_PIN);
    int rawHum = analogRead(HUM_POT_PIN);
    int rawPres = analogRead(PRES_POT_PIN);

    // Exibição dos valores brutos no Serial Monitor
    Serial.printf("RawTemp: %d | RawHum: %d | RawPres: %d\n", rawTemp, rawHum, rawPres);

    // Conversão para faixas realistas com ajustes nas faixas de mapeamento
    float tempSim = map(rawTemp, 0, 4095, 0, 100); // Faixa de 0°C a 100°C
    float humSim = map(rawHum, 0, 4095, 0, 100);   // Faixa de 0% a 100% de umidade

    // Determinação de presença baseada no valor do potenciômetro de presença
    bool presenca = rawPres > 2000; // Se o valor lido for maior que 2000, há presença

    // Debug
    Serial.printf("TempReal: %.1f°C | HumReal: %.1f%% | TempSim: %.1f°C | HumSim: %.1f%% | Presença: %s\n",
                  tempReal, humReal, tempSim, humSim, presenca ? "SIM" : "NÃO");

    // Envio para Firebase
    Firebase.RTDB.setFloat(&fbdo, "/sensores/temperatura_real", tempReal);
    Firebase.RTDB.setFloat(&fbdo, "/sensores/umidade_real", humReal);
    Firebase.RTDB.setFloat(&fbdo, "/sensores/temperatura_simulada", tempSim);
    Firebase.RTDB.setFloat(&fbdo, "/sensores/umidade_simulada", humSim);
    Firebase.RTDB.setBool(&fbdo, "/sensores/presenca_simulada", presenca);

    // Lógica para acionar ventoinha (LED) com a nova condição
    if (tempSim > 30 && humSim > 65) {
      digitalWrite(FAN_LED_PIN, HIGH);  // Ventoinha (LED) ligado
    } else {
      digitalWrite(FAN_LED_PIN, LOW);   // Ventoinha (LED) desligado
    }
  }
}