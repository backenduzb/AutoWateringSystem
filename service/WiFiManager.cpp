#include "WiFiManager.h"

WiFiManager::WiFiManager(const char* wifiSSID, const char* wifiPassword) {
  this->ssid = wifiSSID;
  this->password = wifiPassword;
}

bool WiFiManager::connect() {
  Serial.print("📡 Connecting to WiFi: ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 30) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n✅ WiFi connected!");
    printStatus();
    return true;
  } else {
    Serial.println("\n❌ WiFi connection failed!");
    return false;
  }
}

bool WiFiManager::isConnected() {
  return WiFi.status() == WL_CONNECTED;
}

void WiFiManager::disconnect() {
  WiFi.disconnect();
}

void WiFiManager::printStatus() {
  Serial.print("📡 IP: ");
  Serial.println(WiFi.localIP());
  Serial.print("📡 Gateway: ");
  Serial.println(WiFi.gatewayIP());
}