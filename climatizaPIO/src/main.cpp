#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "Adafruit_HTU21DF.h"
#include <Adafruit_APDS9960.h>

// OLED
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// Sensor de temperatura/umidade
Adafruit_HTU21DF htu;

// Sensor de gestos APDS9960
Adafruit_APDS9960 apds;

// Pinos do LED RGB do sensor de som
#define RED_LED_PIN    19  // R
#define GREEN_LED_PIN  23  // G
#define BLUE_LED_PIN   18  // B

bool presencaDetectada = false;

void setup() {
  Serial.begin(115200);

  // Inicializa LEDs RGB
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

  apds.enableGesture(true);  // Garante que gestos estejam ativados

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
}

void loop() {
  // Leitura contínua de gestos
  if (apds.gestureValid()) {
    uint8_t gesture = apds.readGesture();

    switch (gesture) {
      case APDS9960_UP:
      case APDS9960_LEFT:
        presencaDetectada = false;
        Serial.println("Gesto: AUSENTE (UP/LEFT)");
        break;

      case APDS9960_DOWN:
      case APDS9960_RIGHT:
        presencaDetectada = true;
        Serial.println("Gesto: PRESENÇA (DOWN/RIGHT)");
        break;

      default:
        Serial.println("Gesto não reconhecido");
        break;
    }
  }

  // Leitura de sensores ambientais
  float temp = htu.readTemperature();
  float umid = htu.readHumidity();

  bool ligarLED = (temp >= 23.0 && umid >= 60.0 && presencaDetectada);

  // Controla o LED RGB (vermelho apenas)
  digitalWrite(RED_LED_PIN, ligarLED ? HIGH : LOW);
  digitalWrite(GREEN_LED_PIN, LOW);
  digitalWrite(BLUE_LED_PIN, LOW);

  // Atualiza display
  display.clearDisplay();
  display.setCursor(0, 0);
  display.printf("Temp: %.1f C\n", temp);
  display.printf("Umid: %.1f %%\n", umid);
  display.printf("Presenca: %s\n", presencaDetectada ? "Detectada" : "Nao");
  display.printf("LED: %s", ligarLED ? "ON" : "OFF");
  display.display();

  delay(100);  // Atraso pequeno para manter responsividade
}