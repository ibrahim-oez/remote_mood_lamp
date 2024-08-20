#include <Arduino.h>
#include <ESP8266WiFi.h>
#include "wifi.h"



int PIN = D6;
bool connected;

void setup() {
  pinMode(PIN,OUTPUT);
  Serial.begin(115200);
  connected = setup_wifi();
  if (connected) {
        Serial.println("WiFi successfully connected.");
    } else {
        Serial.println("Failed to connect to WiFi.");
    }
  
}


 void loop() {
  
 connected = (WiFi.status() == WL_CONNECTED);

  if(connected != true){
    digitalWrite(PIN, HIGH);
    delay(1000);
    digitalWrite(PIN, LOW);
    delay(1000);
  }else{
    digitalWrite(PIN, HIGH); 
  }
    
}


