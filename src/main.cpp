#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

const char* ssid = "FragNach";
const char* password = "12345678";

// Pin Definition
const int switchButtonPin = A0;  // Umschalt-Button
const int ledPinRed = D6;  // Rote LED
const int ledPinGreen = D7;  // Grüne LED

ESP8266WebServer server(80);

// Funktionsdeklarationen
void checkButton();
void updateLEDs(int state);
void notifyOtherESP(const char* action);

void setup() {
  Serial.begin(115200);

  // Setup der Pins
  pinMode(switchButtonPin, INPUT);  // Umschalt-Button Pin als Eingang
  pinMode(ledPinRed, OUTPUT);
  pinMode(ledPinGreen, OUTPUT);

  // Initialer Zustand der LEDs
  digitalWrite(ledPinRed, LOW);
  digitalWrite(ledPinGreen, LOW);

  // WLAN Verbindung
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Verbindung wird hergestellt...");
  }
  Serial.println("WLAN verbunden!");
  Serial.print("IP-Adresse: ");
  Serial.println(WiFi.localIP());

  // Server-Routen definieren
  server.on("/red", [](){
    Serial.println("Empfangen: /red");
    updateLEDs(0);  // Setzt Rot an
    server.send(200, "text/plain", "Red LED ON, Green LED OFF");
  });

  server.on("/green", [](){
    Serial.println("Empfangen: /green");
    updateLEDs(1);  // Setzt Grün an
    server.send(200, "text/plain", "Green LED ON, Red LED OFF");
  });

  server.begin();
  Serial.println("HTTP Server gestartet");
}

void loop() {
  server.handleClient();
  checkButton();  // Button-Zustand überprüfen
}

void checkButton() {
  // Button-Status
  int buttonValue = analogRead(switchButtonPin);
  Serial.print("Button-Wert: ");
  Serial.println(buttonValue);

  // 0 = Rot, 1 = Grün
  if (buttonValue > 1000) {  // A0 liest analoge Werte; Schwellenwert für HIGH
    updateLEDs(1);  // Setzt Grün an
    notifyOtherESP("/green");  // Informiere den anderen ESP
  } else {
    updateLEDs(0);  // Setzt Rot an
    notifyOtherESP("/red");  // Informiere den anderen ESP
  }

  delay(1500);  // Debounce für den Taster
}

void updateLEDs(int state) {
  if (state == 0) {
    digitalWrite(ledPinRed, HIGH);  // Rote LED an
    digitalWrite(ledPinGreen, LOW);  // Grüne LED aus
    Serial.println("LED Zustand: Rot an, Grün aus");
  } else {
    digitalWrite(ledPinRed, LOW);  // Rote LED aus
    digitalWrite(ledPinGreen, HIGH);  // Grüne LED an
    Serial.println("LED Zustand: Rot aus, Grün an");
  }
}

void notifyOtherESP(const char* action) {
  WiFiClient client;
  const char* otherESPIP = "192.168.84.43";  // IP-Adresse des anderen ESP
  
  Serial.print("Sende Anfrage an ");
  Serial.println(otherESPIP);

  if (client.connect(otherESPIP, 80)) {
    client.print(String("GET ") + action + " HTTP/1.1\r\n" +
                 "Host: " + otherESPIP + "\r\n" +
                 "Connection: close\r\n\r\n");
    client.stop();
    Serial.println("Anfrage gesendet: " + String(action));
  } else {
    Serial.println("Verbindung zum anderen ESP fehlgeschlagen");
  }
}
