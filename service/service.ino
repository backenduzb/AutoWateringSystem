#include <WiFi.h>
#include <WebSocketsClient.h>  // WebSocketClient.h emas, WebSocketsClient.h
#include <DHT.h>

// ============ KONFIGURATSIYA ============
// WiFi sozlamalari
const char* ssid = "KOM_105";
const char* password = "*kom105!0";

// Backend server sozlamalari
const char* serverHost = "192.168.1.100";
const int serverPort = 8000;
const char* secretToken = "esp32";

// Sensor sozlamalari
#define DHTPIN 34
#define DHTTYPE DHT11

// ============ GLOBAL O'ZGARUVCHILAR ============
DHT dht(DHTPIN, DHTTYPE);
WebSocketsClient webSocket;  // O'zgartirildi

unsigned long lastSendTime = 0;
const unsigned long sendInterval = 2000;

// ============ WEBSOCKET EVENT HANDLER ============
void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {
  switch(type) {
    case WStype_DISCONNECTED:
      Serial.println("❌ WebSocket uzildi!");
      break;
      
    case WStype_CONNECTED:
      Serial.println("✅ WebSocket ulandi!");
      // Ulanish xabarini yuborish
      webSocket.sendTXT("{\"type\": \"connect\", \"message\": \"ESP32 connected\"}");
      break;
      
    case WStype_TEXT:
      Serial.print("📥 Serverdan kelgan xabar: ");
      Serial.println((char*)payload);
      break;
      
    case WStype_ERROR:
      Serial.println("❌ WebSocket xatosi!");
      break;
      
    case WStype_BIN:
      Serial.printf("📥 Binary data received: %u bytes\n", length);
      break;
  }
}

// ============ FUNKSIYALAR ============

// WiFi ga ulanish
void connectToWiFi() {
  Serial.print("WiFi ga ulanish: ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("\n✅ WiFi ulandi!");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

// WebSocket ga ulanish
void connectToWebSocket() {
  Serial.println("WebSocket ga ulanish...");
  
  // WebSocket ulanish path
  String wsPath = "/ws/sensors/?secret=" + String(secretToken);
  
  // WebSocket sozlamalari
  webSocket.begin(serverHost, serverPort, wsPath);
  webSocket.onEvent(webSocketEvent);
  webSocket.setReconnectInterval(5000);  // 5 sekundda qayta ulanish
}

// Namlikni o'qish
float readHumidity() {
  float humidity = dht.readHumidity();
  
  if (isnan(humidity)) {
    Serial.println("❌ DHT sensordan o'qish xatosi!");
    return -1;
  }
  
  return humidity;
}

// WebSocket orqali ma'lumot yuborish
void sendSensorData(float humidity) {
  // JSON formatda ma'lumot tayyorlash
  String jsonData = "{\"value\": " + String(humidity) + "}";
  
  // Ma'lumotni yuborish
  webSocket.sendTXT(jsonData);
  
  Serial.print("📤 Yuborildi: ");
  Serial.println(jsonData);
}

// ============ SETUP ============
void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("\n=================================");
  Serial.println("ESP32 Sensor WebSocket Client");
  Serial.println("=================================\n");
  
  // DHT sensorini ishga tushirish
  dht.begin();
  Serial.println("✅ DHT sensori tayyor");
  
  // WiFi ga ulanish
  connectToWiFi();
  
  // WebSocket ga ulanish
  connectToWebSocket();
  
  Serial.println("\n✅ Tizim tayyor!");
  Serial.println("=================================\n");
}

// ============ LOOP ============
void loop() {
  // WebSocket ni yangilash (muhim!)
  webSocket.loop();
  
  // Belgilangan vaqt oralig'ida ma'lumot yuborish
  unsigned long currentTime = millis();
  if (currentTime - lastSendTime >= sendInterval) {
    lastSendTime = currentTime;
    
    // Sensor ma'lumotini o'qish
    float humidity = readHumidity();
    
    if (humidity > 0) {
      // Ma'lumotni WebSocket orqali yuborish
      sendSensorData(humidity);
      
      // Serial monitor ga chiqarish
      Serial.print("🌡️ Namlik: ");
      Serial.print(humidity);
      Serial.println("%");
    } else {
      Serial.println("⚠️ Sensor o'qilmadi, keyingi urinishda...");
    }
  }
  
  // Kichik delay
  delay(100);
}