#ifndef WEBSOCKET_MANAGER_H
#define WEBSOCKET_MANAGER_H

#include <WebSocketsClient.h>
#include <ArduinoJson.h>

class WebSocketManager {
private:
  WebSocketsClient webSocket;
  bool connected;
  const char* host;
  int port;
  const char* token;
  
public:
  WebSocketManager(const char* serverHost, int serverPort, const char* secretToken);
  void begin();
  void loop();
  void sendData(int value);
  bool isConnected();
  void reconnect();
  void setConnected(bool state);
  WebSocketsClient* getClient();
};

#endif