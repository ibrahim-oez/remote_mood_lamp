#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
#include "wifi.h"

void handleRoot();
void sendHttpRequest(const String& path);

// Konfiguration der Pins
const int buttonPin = A0;  // Button an A0
const int ledPin = D6;    // LED an D6

// IP-Adresse des anderen ESP8266
const char* otherESPAddress = "http://192.168.84.43"; // Ersetze durch die IP-Adresse des anderen ESP8266

ESP8266WebServer server(80); // HTTP-Server auf Port 80

// Entprellung
unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 50; // ms
int lastButtonState = HIGH;
int ledState = LOW; // Zustand der LED

void setup() {
    pinMode(buttonPin, INPUT); // Button-Pin als Eingang
    pinMode(ledPin, OUTPUT);   // LED-Pin als Ausgang
    Serial.begin(115200);

    // WLAN-Verbindung herstellen
    if (setup_wifi()) {
        Serial.println("WiFi erfolgreich verbunden.");
    } else {
        Serial.println("WLAN-Verbindung fehlgeschlagen.");
    }

    // HTTP-Routen definieren
    server.on("/", HTTP_GET, handleRoot);
    server.on("/led/on", HTTP_GET, []() {
        digitalWrite(ledPin, HIGH); // LED einschalten
        server.send(200, "text/plain", "LED eingeschaltet");
    });
    server.on("/led/off", HTTP_GET, []() {
        digitalWrite(ledPin, LOW); // LED ausschalten
        server.send(200, "text/plain", "LED ausgeschaltet");
    });

    server.begin();
}

void loop() {
    server.handleClient(); // Verarbeite eingehende Anfragen

    // Lese den Zustand des Buttons
    int reading = analogRead(buttonPin);
    int buttonState = (reading < 100) ? LOW : HIGH;

    // Entprellung des Buttons
    unsigned long currentTime = millis();
    if (buttonState != lastButtonState) {
        lastDebounceTime = currentTime;
    }

    if ((currentTime - lastDebounceTime) > debounceDelay) {
        if (buttonState != ledState) {
            ledState = buttonState;
            if (ledState == LOW) {
                sendHttpRequest("/led/on");
            } else {
                sendHttpRequest("/led/off");
            }
        }
    }

    lastButtonState = buttonState;

    delay(50); // Kürzere Verzögerung
}

void handleRoot() {
    server.send(200, "text/plain", "Willkommen beim LED-Steuerungsserver");
}

void sendHttpRequest(const String& path) {
    if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;
        WiFiClient client;
        String url = String(otherESPAddress) + path;

        http.begin(client, url);
        int httpCode = http.GET();

        if (httpCode > 0) {
            String payload = http.getString();
            Serial.println(payload);
        } else {
            Serial.println("Fehler bei der GET-Anfrage");
        }

        http.end();
    }
}
