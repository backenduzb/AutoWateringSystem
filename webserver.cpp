#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>

#include "webserver.h"

WebServer server(80);

void send()

void WSHandleRoot() {
    StaticJsonDocument<200> doc;

    doc["device"] = "ESP32";
    doc["status"] = "ok";
    doc["uptime"] = millis();

    String output;
    serializeJson(doc, output);

    server.send(200, "application/json", output);
}

void WSSetup() {
    server.on("/", WSHandleRoot);
    server.begin();
}

void WSClient() {
    server.handleClient();
}