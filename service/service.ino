#include <WiFi.h>
#include <WebSocketsClient.h>

// ============ KONFIGURATSIYA ============
const char* ssid = "KOM_105";
const char* password = "*kom105!0";

// Railway backend
const char* serverHost = "autowateringapi.up.railway.app";
const int serverPort = 443;  // SSL port
const char* secretToken = "esp32";

// Fingerprint (ixtiyoriy, SSL tekshiruvi uchun)
// const char* fingerprint = "YOUR_CERT_FINGERPRINT";

#define SOIL_SENSOR_PIN 34

// ============ GLOBAL ============
WebSocketsClient webSocket;
unsigned long lastSendTime = 0;
const unsigned long sendInterval = 5000;
bool wsConnected = false;

const int DRY_VALUE = 3000;
const int WET_VALUE = 1200;

// ============ WEBSOCKET EVENT ============
void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {
  switch(type) {
    case WStype_DISCONNECTED:
      Serial.println("❌ WebSocket disconnected!");
      wsConnected = false;
      break;
      
    case WStype_CONNECTED:
      Serial.println("✅ WebSocket connected!");
      wsConnected = true;
      // Ulanish xabarini yuborish
      webSocket.sendTXT("{\"type\":\"connect\",\"message\":\"ESP32 soil sensor connected\"}");
      break;
      
    case WStype_TEXT:
      Serial.print("📥 Server response: ");
      Serial.println((char*)payload);
      break;
      
    case WStype_ERROR:
      Serial.println("❌ WebSocket error!");
      wsConnected = false;
      break;
      
    case WStype_PING:
      Serial.println("📡 Ping received");
      break;
      
    case WStype_PONG:
      Serial.println("📡 Pong received");
      break;
  }
}

// ============ SENSOR ============
int readSoilMoisture() {
  int sum = 0;
  for (int i = 0; i < 10; i++) {
    sum += analogRead(SOIL_SENSOR_PIN);
    delay(5);
  }
  
  int rawValue = sum / 10;
  float moisture = (float)(rawValue - WET_VALUE) / (DRY_VALUE - WET_VALUE) * 100.0;
  
  if (moisture > 100) moisture = 100;
  if (moisture < 0) moisture = 0;
  
  Serial.print("📊 Raw: ");
  Serial.print(rawValue);
  Serial.print(" → Moisture: ");
  Serial.print(moisture);
  Serial.println("%");
  
  return (int)moisture;
}

// ============ WIFI ============
void connectToWiFi() {
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
    Serial.print("📡 IP: ");
    Serial.println(WiFi.localIP());
    Serial.print("📡 Gateway: ");
    Serial.println(WiFi.gatewayIP());
  } else {
    Serial.println("\n❌ WiFi connection failed!");
  }
}

// ============ WEBSOCKET ============
void connectToWebSocket() {
  Serial.println("\n🔌 Connecting to WebSocket...");
  Serial.print("🌐 Server: ");
  Serial.println(serverHost);
  Serial.print("🔌 Port: ");
  Serial.println(serverPort);
  
  String wsPath = "/ws/sensors/?secret=" + String(secretToken);
  Serial.print("📡 Path: ");
  Serial.println(wsPath);
  
  // SSL ulanish (wss://)
  webSocket.beginSSL(serverHost, serverPort, wsPath);
  
  // Agar SSL ishlamasa, HTTP ni sinab ko'ring:
  // webSocket.begin(serverHost, 80, wsPath);
  
  webSocket.onEvent(webSocketEvent);
  webSocket.setReconnectInterval(5000);
  
  // Extra headers (agar kerak bo'lsa)
  // webSocket.setExtraHeaders("User-Agent: ESP32\r\n");
}

// ============ SETUP ============
void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("\n=================================");
  Serial.println("ESP32 Soil Moisture Sensor");
  Serial.println("=================================\n");
  
  // Sensor
  pinMode(SOIL_SENSOR_PIN, INPUT);
  Serial.println("✅ Soil moisture sensor ready");
  
  // WiFi
  connectToWiFi();
  
  // WebSocket
  if (WiFi.status() == WL_CONNECTED) {
    connectToWebSocket();
  } else {
    Serial.println("❌ Cannot connect to WebSocket - WiFi not connected");
  }
  
  Serial.println("\n✅ System ready!\n");
}

// ============ LOOP ============
void loop() {
  // WebSocket loop
  webSocket.loop();
  
  // WiFi tekshirish
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("⚠️ WiFi lost, reconnecting...");
    connectToWiFi();
    if (WiFi.status() == WL_CONNECTED) {
      connectToWebSocket();
    }
  }
  
  // Ma'lumot yuborish
  unsigned long now = millis();
  if (now - lastSendTime >= sendInterval) {
    lastSendTime = now;
    
    if (wsConnected) {
      int moisture = readSoilMoisture();
      
      if (moisture >= 0 && moisture <= 100) {
        String jsonData = "{\"value\": " + String(moisture) + "}";
        webSocket.sendTXT(jsonData);
        
        Serial.print("📤 Sent: ");
        Serial.print(moisture);
        Serial.println("%");
        
        // Status
        if (moisture < 30) {
          Serial.println("💧 Status: DRY 🌵");
        } else if (moisture < 70) {
          Serial.println("💧 Status: NORMAL 🌱");
        } else {
          Serial.println("💧 Status: WET 💧");
        }
      } else {
        Serial.println("⚠️ Sensor read error!");
      }
    } else {
      Serial.println("⏳ Waiting for WebSocket connection...");
      
      // Qayta ulanish
      static unsigned long lastReconnect = 0;
      if (now - lastReconnect > 10000) {
        lastReconnect = now;
        Serial.println("🔄 Attempting to reconnect WebSocket...");
        connectToWebSocket();
      }
    }
  }
  
  delay(100);
}