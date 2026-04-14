#include <Arduino.h>
#include "wifi.h"
#include "webserver.h"

void setup() {
    Serial.begin(9600);
    Wificonnect("KOM_105", "*kom105!0");
    WSSetup();
}

void loop() {
    WSClient();
}