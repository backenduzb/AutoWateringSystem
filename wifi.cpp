#include <Arduino.h>
#include <WiFi.h>

#include "wifi.h"

void Wificonnect(const char* name, const char* password) {
    WiFi.begin(name, password);

    Serial.print("Connecting");

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println("\nConnected!");
    Serial.println(WiFi.localIP());
}