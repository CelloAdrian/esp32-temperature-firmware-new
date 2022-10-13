#include <Arduino.h>
#include <ArduinoJson.h>
#include <DHT.h>
#include <WebServer.h>
#include <WiFi.h>

#define BAUD_RATE 115200

#define DHTTYPE DHT21

const int DHTPIN = 15;

const char* STA_SSID = "STA_SSID";
const char* STA_PW = "STA_PW";

WebServer server(80);
DHT dht(DHTPIN, DHTTYPE);

IPAddress ip(192, 168, 0, 4);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 0, 0);

void handleRoot() {
  String s = "Hello from ESP32!";
  server.send(200, "text/plain", s);
}

// handle the /temperature endpoint
void handleTemperature() {
  // Get the temperature from the DHT sensor
  float t = dht.readTemperature();

  // Create a JSON object
  StaticJsonDocument<200> doc;
  doc["temperature"] = t;

  // Serialize the JSON object
  String output;
  serializeJson(doc, output);

  // Send the JSON object
  server.send(200, "application/json", output);
}

void handleHumidity() {
  float h = dht.readHumidity();
  StaticJsonDocument<200> doc;
  doc["humidity"] = h;
  String output;
  serializeJson(doc, output);
  server.send(200, "application/json", output);
}

void setup() {
  Serial.begin(BAUD_RATE);

  WiFi.config(ip, gateway, subnet);

  WiFi.begin(STA_SSID, STA_PW);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi connected");
  Serial.println(WiFi.localIP());

  server.on("/", handleRoot);
  server.on("/temperature", handleTemperature);
  server.on("/humidity", handleHumidity);

  server.begin();
  dht.begin();
}

void loop() {
  server.handleClient();

  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  Serial.print("Temperature: ");
  Serial.println(temperature);

  Serial.print("Humidity: ");
  Serial.println(humidity);

  delay(2000);
}