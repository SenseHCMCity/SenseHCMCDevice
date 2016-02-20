/*
 * Read values from a DHT11 sensor and push them to a private Blynk server via WIFI.
 */
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <SimpleTimer.h>
#include <ThingSpeak.h>
#include <DHT.h>

// IMPORTANT: set auth properties in here first
#include "AuthProperties.h" 

// DHT11 sensor. Connect:
//   pin 1 (vcc) to 3v3
//   pin 2 (data) to pin defined DHTPIN below
//   pin 4 (ground) to GND
#define DHTPIN D6
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// Blynk
#define BLYNK_PRINT Serial    // Comment this out to disable prints and save space
#define PIN_TEMPERATURE 1
#define PIN_HUMIDITY 2

// Thingspeak (set key in AuthProperties.h)
const unsigned long TS_CHANNEL = 88894;
const unsigned int TS_FIELD_TEMPERATURE = 1;
const unsigned int TS_FIELD_HUMIDITY = 2;
WiFiClient  client;

// Misc
const boolean RUN_LOCAL = false;
const boolean USE_FAHRENHEIT = false;
const short PUSH_INTERVAL = 30000;

SimpleTimer timer;

float readTemperature() {
  return dht.readTemperature(USE_FAHRENHEIT);
}

float readHumidity() {
  return dht.readHumidity();
}

float readHeatIndex(float temperature, float humidity) {
  return dht.computeHeatIndex(temperature, humidity, USE_FAHRENHEIT);
}

void sendValues()
{
  float temperature = readTemperature();
  float humidity = readHumidity();
  if (isnan(temperature) || isnan(humidity)) {
    Serial.println("Failed to read dht11 values");
    return;
  }
  Serial.print("temperature: ");
  Serial.print(temperature);
  Serial.print("; humidity: ");
  Serial.println(humidity);
  if (!RUN_LOCAL) {
    Blynk.virtualWrite(PIN_TEMPERATURE, temperature);
    Blynk.virtualWrite(PIN_HUMIDITY, humidity);
    ThingSpeak.setField(TS_FIELD_TEMPERATURE, temperature);
    ThingSpeak.setField(TS_FIELD_HUMIDITY, humidity);
    ThingSpeak.writeFields(TS_CHANNEL, TS_WRITE_KEY);
  }
}

void setup() {
  dht.begin();
  Serial.begin(9600);
  if (!RUN_LOCAL) {
    Blynk.begin(BLYNK_AUTH, WIFI_SSID, WIFI_PASSWORD, BLYNK_HOST);
    ThingSpeak.begin(client);
  }
  timer.setInterval(PUSH_INTERVAL, sendValues);
}

void loop() {
  if (!RUN_LOCAL)
    Blynk.run();
  timer.run();
}

