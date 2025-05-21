; PlatformIO Project Configuration File
;
;   Build options: build flags, source filter
;   Upload options: custom upload port, speed and extra flags
;   Library options: dependencies, extra library storages
;   Advanced options: extra scripting
;
; Please visit documentation for the other options and examples
; https://docs.platformio.org/page/projectconf.html

[env:upesy_wroom]
platform = espressif32
board = upesy_wroom
framework = arduino
monitor_speed = 115200
lib_deps = 
	adafruit/Adafruit GFX Library@^1.11.9
	adafruit/Adafruit SSD1306@^2.5.7
	robocore/RoboCore - MMA8452Q@^1.0.0
	adafruit/Adafruit HTU21DF Library@^1.0.4
	adafruit/Adafruit APDS9960 Library@^1.1.5
	arduino-libraries/NTPClient@^3.2.1
	mobizt/Firebase ESP32 Client@^4.4.17
