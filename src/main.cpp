#include <Arduino.h>
#include <ArduinoJson.h>
#include <DHT.h>
#include <WiFi.h>

#define BAUD_RATE 115200

#define AP_PORT 80
#define AP_SSID "ESP32-AP"
#define AP_PW "123456789"
#define DHT_TYPE DHT21

const int DHT_PIN = 13;

WiFiServer server(AP_PORT);
DHT dht(DHT_PIN, DHT_TYPE);

void setup() {
    Serial.begin(BAUD_RATE);
    Serial.println("Hello world");
    WiFi.mode(WIFI_AP_STA);
    WiFi.softAP(AP_SSID, AP_PW);
    IPAddress IP = WiFi.softAPIP();
    Serial.println("AP IP:");
    Serial.println(IP.toString());
    server.begin();
}

String header;
const char* STA_SSID;
const char* STA_PW;

void loop() {
    WiFiClient client = server.available();

    if (WiFi.getMode() == WIFI_AP) {
        if (client) {
            Serial.println("new client");
            String currentLine = "";
            while (client.connected()) {
                if (client.available()) {
                    char c = client.read();
                    Serial.write(c);
                    header += c;
                    if (c == '\n') {
                        if (currentLine.length() == 0) {
                            // respond OK so request wont hang
                            client.print("HTTP/1.1 200 OK\nContent-type: text/html\nConnection: close\n\n");
                            client.print("<!DOCTYPE html><html><body><h1>ESP32 Webserver</h1></body></html>\n");

                            // test routes
                            if (header.indexOf("GET /sta_ssid") >= 0) {
                                StaticJsonDocument<200> doc;
                                DeserializationError error = deserializeJson(doc, header);

                                deserializeJson(doc, header);
                                STA_SSID = doc["sta_ssid"];
                                // test
                                Serial.println("STA_SSID: ");
                                Serial.println(STA_SSID);
                                // test
                            }

                            if (header.indexOf("GET /sta_pw") >= 0) {
                                StaticJsonDocument<200> doc;
                                DeserializationError error = deserializeJson(doc, header);

                                deserializeJson(doc, header);
                                STA_PW = doc["sta_pw"];
                                // test
                                Serial.println("STA_PW: ");
                                Serial.println(STA_PW);
                                // test
                            }
                            // test routes

                            if (STA_SSID != NULL && STA_PW != NULL) {
                                WiFi.mode(WIFI_STA);
                            }
                        }
                    }
                }
            }
        }
    } else if (WiFi.getMode() == WIFI_STA) {
        WiFi.begin(STA_SSID, STA_PW);
        Serial.println("Connecting to WiFi");

        while (WiFi.status() != WL_CONNECTED) {
            delay(100);
            Serial.print(".");
        }

        if (WiFi.status() == WL_CONNECTED) {
            Serial.println("Connected to STA WiFi");
            Serial.println("IP address: ");
            Serial.println(WiFi.localIP());
            if (client.connected()) {
                float humidity = dht.readHumidity();
                float temperature = dht.readTemperature();

                if (isnan(humidity) || isnan(temperature)) {
                    Serial.println("Failed to read from DHT sensor!");
                    return;
                }

                // send temperature as json data to route /temperature
                StaticJsonDocument<200> doc;
                doc["temperature"] = temperature;
                String output;
                serializeJson(doc, output);
                client.print("POST /temperature HTTP/1.1\r");

                // send humidity as json data to route /humidity
                StaticJsonDocument<200> doc2;
                doc2["humidity"] = humidity;
                String output2;
                serializeJson(doc2, output2);
                client.print("POST /humidity HTTP/1.1\r");
            }
        }
    }
}