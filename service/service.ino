#include "config.h"
#include "WiFiManager.h"
#include "WebSocketManager.h"
#include "SoilSensor.h"

// Global objects
WiFiManager wifiManager(ssid, password);
WebSocketManager webSocketManager(serverHost, serverPort, secretToken);
SoilSensor soilSensor(SOIL_SENSOR_PIN, DRY_VALUE, WET_VALUE);

// Global variables
unsigned long lastSendTime = 0;

// WebSocket event handler
void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {
  switch(type) {
    case WStype_DISCONNECTED:
      Serial.println("❌ WebSocket disconnected!");
      webSocketManager.setConnected(false);
      break;
      
    case WStype_CONNECTED:
      Serial.println("✅ WebSocket connected!");
      webSocketManager.setConnected(true);
      webSocketManager.sendData(-1); // Send connection message
      break;
      
    case WStype_TEXT:
      Serial.print("📥 Server response: ");
      Serial.println((char*)payload);
      break;
      
    case WStype_ERROR:
      Serial.println("❌ WebSocket error!");
      webSocketManager.setConnected(false);
      break;
      
    case WStype_PING:
      Serial.println("📡 Ping received");
      break;
      
    case WStype_PONG:
      Serial.println("📡 Pong received");
      break;
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("\n=================================");
  Serial.println("ESP32 Soil Moisture Sensor");
  Serial.println("=================================\n");
  
  // Connect to WiFi
  if (wifiManager.connect()) {
    webSocketManager.begin();
  } else {
    Serial.println("❌ Cannot connect to WebSocket - WiFi not connected");
  }
  
  Serial.println("\n✅ System ready!\n");
}

void loop() {
  webSocketManager.loop();
  
  // Check WiFi connection
  if (!wifiManager.isConnected()) {
    Serial.println("⚠️ WiFi lost, reconnecting...");
    if (wifiManager.connect()) {
      webSocketManager.begin();
    }
  }
  
  // Send soil moisture data periodically
  unsigned long now = millis();
  if (now - lastSendTime >= sendInterval) {
    lastSendTime = now;
    
    if (webSocketManager.isConnected()) {
      int moisture = soilSensor.readMoisture();
      webSocketManager.sendData(moisture);
    } else {
      static unsigned long lastReconnect = 0;
      if (now - lastReconnect > 10000) {
        lastReconnect = now;
        webSocketManager.reconnect();
      }
    }
  }
  
  delay(80);
}