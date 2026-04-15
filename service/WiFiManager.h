#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <WiFi.h>

class WiFiManager {
private:
  const char* ssid;
  const char* password;
  
public:
  WiFiManager(const char* wifiSSID, const char* wifiPassword);
  bool connect();
  bool isConnected();
  void disconnect();
  void printStatus();
};

#endif