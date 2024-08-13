/*
LCD clock with Temperature and Humidity sensors and threshold LEDs.
by Aleksandr Jakovlev
aleks.jakovlev[x]outlook.com
08/08/2024
v 0.3
*/

#include <Wire.h>
#include <DS3231.h>
#include <LiquidCrystal_I2C.h>
#include <dht_nonblocking.h>

#define DHT_SENSOR_TYPE DHT_TYPE_11
#define DHT_SENSOR_PIN 2
#define LCD_ADDRESS 0x27
#define LCD_COLUMNS 16
#define LCD_ROWS 2
#define SERIAL_BAUD_RATE 9600
#define GREEN_LED_PIN 3
#define RED_LED_PIN 4

DS3231 clock;
RTCDateTime dt;
LiquidCrystal_I2C lcd(LCD_ADDRESS, LCD_COLUMNS, LCD_ROWS);
DHT_nonblocking dht_sensor(DHT_SENSOR_PIN, DHT_SENSOR_TYPE);

const unsigned long UPDATE_INTERVAL = 1000;  // Update interval in milliseconds
unsigned long previousMillis = 0;

float temperature;
float humidity;

void setup() {
  Serial.begin(SERIAL_BAUD_RATE);
  pinMode(GREEN_LED_PIN, OUTPUT);
  pinMode(RED_LED_PIN, OUTPUT);
  clock.begin();
  lcd.init();
  lcd.backlight();
  lcd.clear();
  // Uncomment to set the date and time
  // clock.setDateTime(__DATE__, __TIME__);
}

void loop() {
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= UPDATE_INTERVAL) {
    previousMillis = currentMillis;
    dt = clock.getDateTime();  // Only get the time once per interval
    updateSerial();
    updateLCD();
  }

  if (measureEnvironment(&temperature, &humidity)) {
    tempHumToLCD();
    updateLEDs();
  }
}

bool measureEnvironment(float *temperature, float *humidity) {
  static unsigned long measurement_timestamp = millis();

  if (millis() - measurement_timestamp > 750) {
    if (dht_sensor.measure(temperature, humidity)) {
      measurement_timestamp = millis();
      return true;
    }
  }
  return false;
}

void updateSerial() {
  printTimeToSerial();
  printDateToSerial();
  printTempHumToSerial();
}

void printTimeToSerial() {
  // Time
  Serial.print(dt.hour < 10 ? "0" : "");
  Serial.print(dt.hour);
  Serial.print(":");
  Serial.print(dt.minute < 10 ? "0" : "");
  Serial.print(dt.minute);
  Serial.print(":");
  Serial.print(dt.second < 10 ? "0" : "");
  Serial.print(dt.second);
  Serial.print(" ");
}

void printDateToSerial() {
  // Date
  Serial.print(dt.day < 10 ? "0" : "");
  Serial.print(dt.day);
  Serial.print("/");
  Serial.print(dt.month < 10 ? "0" : "");
  Serial.print(dt.month);
  Serial.print("/");
  Serial.print(dt.year);
  Serial.print(" ");
}

void printTempHumToSerial() {
  // Temperature and humidity
  Serial.print("T:");
  Serial.print(temperature, 0);
  Serial.print("° H:");
  Serial.print(humidity, 0);
  Serial.print("% ");

  // Good or bad range
  Serial.println(isEnvironmentGood() ? "Good" : "Bad");
}

bool isEnvironmentGood() {
  return (temperature >= 19 && temperature <= 27 && humidity >= 20 && humidity <= 85);
}

void updateLCD() {
  // Time
  lcd.setCursor(0, 0);
  lcd.print(dt.hour < 10 ? "0" : "");
  lcd.print(dt.hour);
  lcd.print(":");
  lcd.print(dt.minute < 10 ? "0" : "");
  lcd.print(dt.minute);
  lcd.print(":");
  lcd.print(dt.second < 10 ? "0" : "");
  lcd.print(dt.second);

  // Date
  lcd.setCursor(0, 1);
  lcd.print(dt.day < 10 ? "0" : "");
  lcd.print(dt.day);
  lcd.print("/");
  lcd.print(dt.month < 10 ? "0" : "");
  lcd.print(dt.month);
  lcd.print("/");
  lcd.print(dt.year);
}

void tempHumToLCD() {
  lcd.setCursor(11, 0);
  lcd.print("T:");
  lcd.print(temperature, 0);
  lcd.print((char)223);  // Degree symbol
  lcd.setCursor(11, 1);
  lcd.print("H:");
  lcd.print(humidity, 0);
  lcd.print("%");
}

void updateLEDs() {
  if (isEnvironmentGood()) {
    digitalWrite(GREEN_LED_PIN, HIGH);
    digitalWrite(RED_LED_PIN, LOW);
  } else {
    digitalWrite(GREEN_LED_PIN, LOW);
    digitalWrite(RED_LED_PIN, HIGH);
  }
}
