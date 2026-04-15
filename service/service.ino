#include <Arduino.h>
#include <cstring>
#include <ArduinoJson.h>

#include "MotorController.h"
#include "SoilSensor.h"
#include "WebSocketManager.h"
#include "WiFiManager.h"
#include "config.h"

// ================= GLOBAL =================
WiFiManager wifiManager(ssid, password);
WebSocketManager webSocketManager(serverHost, serverPort, secretToken);
SoilSensor soilSensor(SOIL_SENSOR_PIN, DRY_VALUE, WET_VALUE);

unsigned long lastSendTime = 0;
volatile bool wsConnected = false;
volatile bool motorState = false;

TaskHandle_t sensorTaskHandle = NULL;
TaskHandle_t systemTaskHandle = NULL;

// ================= WEB SOCKET EVENT =================
void webSocketEvent(WStype_t type, uint8_t *payload, size_t length) {
  switch (type) {
    case WStype_DISCONNECTED:
      Serial.println("❌ WebSocket disconnected!");
      wsConnected = false;
      webSocketManager.setConnected(false);
      break;

    case WStype_CONNECTED:
      Serial.println("✅ WebSocket connected!");
      wsConnected = true;
      webSocketManager.setConnected(true);
      webSocketManager.sendData(-1);
      break;

    case WStype_TEXT:
      {
        Serial.print("📥 Server response: ");
        Serial.println((char *)payload);
        
        // JSON formatidagi xabarlarni parse qilish
        StaticJsonDocument<200> doc;
        DeserializationError error = deserializeJson(doc, payload);
        
        if (!error) {
          const char* type = doc["type"];
          
          if (type && strcmp(type, "motor_command") == 0) {
            const char* command = doc["command"];
            
            Serial.println("========================================");
            Serial.println("🎯 MOTOR COMMAND RECEIVED!");
            Serial.printf("   Command: %s\n", command);
            Serial.println("========================================");
            
            if (command && strcmp(command, "ON") == 0) {
              if (!motorState) {
                Serial.println("🔛 EXECUTING: Turning motor ON...");
                motorState = true;
                startMotor();
                Serial.println("✅ Motor ON command executed successfully!");
              } else {
                Serial.println("⚠️ Motor is already ON, ignoring command");
              }
            }
            else if (command && strcmp(command, "OFF") == 0) {
              if (motorState) {
                Serial.println("🔴 EXECUTING: Turning motor OFF...");
                motorState = false;
                stopMotor();
                Serial.println("✅ Motor OFF command executed successfully!");
              } else {
                Serial.println("⚠️ Motor is already OFF, ignoring command");
              }
            }
            Serial.println("========================================\n");
          }
          else if (type && strcmp(type, "motor_state") == 0) {
            bool state = doc["state"];
            Serial.println("========================================");
            Serial.println("📊 MOTOR STATE UPDATE RECEIVED!");
            Serial.printf("   New state: %s\n", state ? "ON" : "OFF");
            Serial.printf("   Current state: %s\n", motorState ? "ON" : "OFF");
            Serial.println("========================================");
            
            if (state != motorState) {
              Serial.println("🔄 State mismatch! Updating motor...");
              motorState = state;
              if (motorState) {
                Serial.println("🔛 Starting motor...");
                startMotor();
              } else {
                Serial.println("🔴 Stopping motor...");
                stopMotor();
              }
              Serial.println("✅ Motor state updated!");
            } else {
              Serial.println("✅ Motor state already matches");
            }
            Serial.println("========================================\n");
          }
          else {
            Serial.println("📝 Unknown message type received");
          }
        }
        else {
          Serial.println("⚠️ Failed to parse JSON, trying plain text...");
          
          // Eski format (oddiy string)
          if (strncmp((char *)payload, "ON", length) == 0) {
            Serial.println("========================================");
            Serial.println("🎯 PLAIN TEXT MOTOR COMMAND: ON");
            Serial.println("========================================");
            if (!motorState) {
              motorState = true;
              startMotor();
              Serial.println("✅ Motor started via plain text command");
            } else {
              Serial.println("⚠️ Motor already ON");
            }
            Serial.println("========================================\n");
          }
          else if (strncmp((char *)payload, "OFF", length) == 0) {
            Serial.println("========================================");
            Serial.println("🎯 PLAIN TEXT MOTOR COMMAND: OFF");
            Serial.println("========================================");
            if (motorState) {
              motorState = false;
              stopMotor();
              Serial.println("✅ Motor stopped via plain text command");
            } else {
              Serial.println("⚠️ Motor already OFF");
            }
            Serial.println("========================================\n");
          }
          else {
            Serial.printf("📝 Unknown plain text: %s\n", (char *)payload);
          }
        }
        break;
      }

    case WStype_ERROR:
      Serial.println("❌ WebSocket error!");
      wsConnected = false;
      webSocketManager.setConnected(false);
      break;

    default:
      break;
  }
}

// ================= SENSOR + WS TASK =================
void sensorTask(void *parameter) {
  TickType_t lastWakeTime = xTaskGetTickCount();
  const TickType_t frequency = pdMS_TO_TICKS(100);
  
  for (;;) {
    webSocketManager.loop();
    
    if (!wifiManager.isConnected()) {
      Serial.println("⚠️ WiFi lost, reconnecting...");
      if (wifiManager.connect()) {
        webSocketManager.begin();
        vTaskDelay(pdMS_TO_TICKS(2000));
      }
    }
    
    unsigned long now = millis();
    if (now - lastSendTime >= sendInterval) {
      lastSendTime = now;
      
      if (wsConnected) {
        int moisture = soilSensor.readMoisture();
        webSocketManager.sendData(moisture);
        Serial.printf("📤 Sent moisture: %d%%\n", moisture);
      }
    }
    
    vTaskDelayUntil(&lastWakeTime, frequency);
  }
}

// ================= SYSTEM MONITOR =================
void systemTask(void *parameter) {
  TickType_t lastWakeTime = xTaskGetTickCount();
  const TickType_t frequency = pdMS_TO_TICKS(10000);
  
  for (;;) {
    Serial.println("\n--- SYSTEM STATUS ---");
    Serial.printf("📊 Free Heap: %d bytes\n", ESP.getFreeHeap());
    Serial.printf("🔄 Motor State: %s\n", motorState ? "ON ✅" : "OFF ❌");
    Serial.printf("📡 WiFi: %s\n", wifiManager.isConnected() ? "Connected ✅" : "Disconnected ❌");
    Serial.printf("🔌 WebSocket: %s\n", wsConnected ? "Connected ✅" : "Disconnected ❌");
    Serial.println("---------------------\n");
    
    vTaskDelayUntil(&lastWakeTime, frequency);
  }
}

// ================= SETUP =================
void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("\n=================================");
  Serial.println("ESP32 SMART SYSTEM START");
  Serial.println("=================================\n");
  
  setupMotor();
  Serial.println("✅ Motor controller initialized");
  
  if (wifiManager.connect()) {
    Serial.println("✅ WiFi connected, initializing WebSocket...");
    webSocketManager.begin();
    webSocketManager.getClient()->onEvent(webSocketEvent);
  }
  
  delay(2000);
  
  xTaskCreatePinnedToCore(
    sensorTask,
    "SensorTask",
    8192,
    NULL,
    2,
    &sensorTaskHandle,
    0
  );
  
  xTaskCreatePinnedToCore(
    systemTask,
    "SystemTask",
    4096,
    NULL,
    1,
    &systemTaskHandle,
    1
  );
  
  Serial.println("\n✅ System ready!");
  Serial.println("📡 Waiting for motor commands via WebSocket...\n");
}

void loop() {
  vTaskDelay(pdMS_TO_TICKS(1000));
}