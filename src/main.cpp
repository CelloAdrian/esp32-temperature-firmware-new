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
char* STA_SSID;
char* STA_PW;

void loop() {
    WiFiClient client = server.available();
    if (client) {
        Serial.println("new client");
        std::string currentLine = "";
        while (client.connected()) {
            if (client.available()) {
                char c = client.read();
                Serial.write(c);
                header += c;
                if (c == '\n') {
                    if (currentLine.length() == 0) {
                        // respond OK so request wont hang
                        client.print("HTTP/1.1 200 OK\nContent-type: text/html\nConnection: close\n\n");
                        client.print("<!DOCTYPE html><html><body><h1>HI FROM ESP32</h1></body></html>\n");

                        // test routes
                        if (header.indexOf("GET /sta_ssid") >= 0) {
                            // client.print("HTTP/1.1 200 OK\nContent-type: text/html\nConnection: close\n\n");
                            // client.print("<!DOCTYPE html><html><body><h1>STA_SSID</h1></body></html>\n");
                            StaticJsonDocument<200> doc;
                            // deserialize the incoming post request
                            DeserializationError error = deserializeJson(doc, header);

                            deserializeJson(doc, header);
                            STA_SSID = doc["sta_ssid"];
                            Serial.println("STA_SSID:");
                            Serial.println(STA_SSID);
                        }

                        if (header.indexOf("GET /sta_pw") >= 0) {
                            // client.print("HTTP/1.1 200 OK\nContent-type: text/html\nConnection: close\n\n");
                            // client.print("<!DOCTYPE html><html><body><h1>STA_PW</h1></body></html>\n");
                            StaticJsonDocument<200> doc;

                            DeserializationError error = deserializeJson(doc, header);

                            deserializeJson(doc, header);
                            STA_PW = doc["sta_pw"];
                            Serial.println("STA_PW:");
                            Serial.println(STA_PW);
                        }

                        if (STA_SSID || STA_PW != "") {
                            WiFi.mode(WIFI_STA);
                            WiFi.begin(STA_SSID, STA_PW);
                            Serial.println("Connecting to WiFi..");
                            while (WiFi.status() != WL_CONNECTED) {
                                delay(500);
                                Serial.print(".");
                            }
                            Serial.println("");
                            Serial.println("Connected to STA WiFi");
                            Serial.println("IP address: ");
                            Serial.println(WiFi.localIP());
                        }
                        break;
                    } else {
                        currentLine = "";
                    }
                } else if (c != '\r') {
                    currentLine += c;
                }
            }
        }

        if (WiFi.getMode() == WIFI_STA) {
            if (WiFi.status() == WL_CONNECTED) {
                if (client.connected()) {
                    // read sensor data
                    DHT dht(DHT_PIN, DHT_TYPE);
                    float h = dht.readHumidity();
                    float t = dht.readTemperature();
                    // check if any reads failed and exit early, then try again
                    if (isnan(h) || isnan(t)) {
                        Serial.println("Failed to read from DHT sensor!");
                        return;
                    }
                }
            }
        }

        header = "";
        client.stop();
        Serial.println("Client disconnected");
    }
}