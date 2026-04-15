#include <WiFi.h>
#include <WebSocketsClient.h>

// ============ KONFIGURATSIYA ============
const char* ssid = "KOM_105";
const char* password = "*kom105!0";

// Railway backend - HTML testda ishlagan URL
const char* serverHost = "autowateringapi.up.railway.app";
const int serverPort = 80;  // HTTP port (ws://)
const char* secretToken = "esp32_secret_token_123";  // HTML dagi bilan bir xil!

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
      webSocket.sendTXT("{\"type\":\"connect\",\"message\":\"ESP32 connected\"}");
      break;
      
    case WStype_TEXT:
      Serial.print("📥 Server: ");
      Serial.println((char*)payload);
      break;
      
    case WStype_ERROR:
      Serial.println("❌ WebSocket error!");
      wsConnected = false;
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
  Serial.print("Connecting to WiFi: ");
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
  } else {
    Serial.println("\n❌ WiFi failed!");
  }
}

// ============ WEBSOCKET ============
void connectToWebSocket() {
  Serial.println("🔌 Connecting to WebSocket...");
  
  String wsPath = "/ws/sensors/?secret=" + String(secretToken);

  webSocket.beginSSL(serverHost, 443, wsPath); // 🔥 MUHIM
  webSocket.onEvent(webSocketEvent);
  webSocket.setReconnectInterval(5000);
}

// ============ SETUP ============
void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("\n=================================");
  Serial.println("ESP32 Soil Moisture Sensor");
  Serial.println("=================================\n");
  
  pinMode(SOIL_SENSOR_PIN, INPUT);
  Serial.println("✅ Soil sensor ready");
  
  connectToWiFi();
  
  if (WiFi.status() == WL_CONNECTED) {
    connectToWebSocket();
  }
  
  Serial.println("\n✅ System ready!\n");
}

// ============ LOOP ============
void loop() {
  webSocket.loop();
  
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("⚠️ WiFi lost, reconnecting...");
    connectToWiFi();
    if (WiFi.status() == WL_CONNECTED) {
      connectToWebSocket();
    }
  }
  
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
        Serial.println("⚠️ Sensor error!");
      }
    } else {
      Serial.println("⏳ Waiting for WebSocket...");
    }
  }
  
  delay(100);
}