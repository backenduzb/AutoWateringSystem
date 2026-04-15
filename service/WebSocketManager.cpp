#include "WebSocketManager.h"
#include "config.h"

extern void webSocketEvent(WStype_t type, uint8_t * payload, size_t length);

WebSocketManager::WebSocketManager(const char* serverHost, int serverPort, const char* secretToken) {
  this->host = serverHost;
  this->port = serverPort;
  this->token = secretToken;
  this->connected = false;
}

void WebSocketManager::begin() {
  Serial.println("\n🔌 Connecting to WebSocket...");
  Serial.print("🌐 Server: ");
  Serial.println(host);
  Serial.print("🔌 Port: ");
  Serial.println(port);
  
  String wsPath = "/ws/sensors/?secret=" + String(token);
  Serial.print("📡 Path: ");
  Serial.println(wsPath);
  
  webSocket.beginSSL(host, port, wsPath);
  webSocket.onEvent(webSocketEvent);
  webSocket.setReconnectInterval(5000);
}

void WebSocketManager::loop() {
  webSocket.loop();
}

void WebSocketManager::sendData(int value) {
  if (connected && value >= 0 && value <= 100) {
    String jsonData = "{\"value\": " + String(value) + "}";
    webSocket.sendTXT(jsonData);
    
    Serial.print("📤 Sent: ");
    Serial.print(value);
    Serial.println("%");
  } else if (value == -1) {
    // Connection message
    webSocket.sendTXT("{\"type\":\"connect\",\"message\":\"ESP32 soil sensor connected\"}");
  }
}

bool WebSocketManager::isConnected() {
  return connected;
}

void WebSocketManager::reconnect() {
  if (!connected) {
    begin();
  }
}

void WebSocketManager::setConnected(bool state) {
  this->connected = state;
}

WebSocketsClient* WebSocketManager::getClient() {
  return &webSocket;
}