#include "ledControl.h"
#include "wifiConnect.h"
#include "loadCell.h"
#include "bleScan.h"
#include "mqttControl.h"

unsigned long lastConnectionCheck = 0;
const unsigned long CONNECTION_CHECK_INTERVAL = 10000; // 10초마다 연결 상태 확인

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println("Hardware Controller Starting...");
  Serial.println("Version: 2.0 with enhanced debugging");
  
  setLED();
  setLoadCell();
  
  Serial.println("Setup completed.");
}

void loop() {
  // put your main code here, to run repeatedly:
  unsigned long currentTime = millis();
  
  // WiFi 연결 확인 및 연결
  connectWiFi();
  
  // MQTT 연결 상태 확인 및 재연결
  if (WiFi.status() == WL_CONNECTED) {
    if (!mqttClient.connected()) {
      // 주기적으로만 재연결 시도 (너무 자주 시도하지 않도록)
      if (currentTime - lastConnectionCheck >= CONNECTION_CHECK_INTERVAL) {
        Serial.println("MQTT disconnected, attempting reconnection...");
        connectMQTT();
        lastConnectionCheck = currentTime;
      }
    } else {
      // MQTT 연결 유지
      mqttClient.loop();
    }
  } else {
    Serial.println("WiFi not connected, skipping MQTT operations");
  }
  
  // BLE 작업 처리
  processTasks();
  
  // 연결 상태 주기적 출력 (60초마다)
  static unsigned long lastStatusPrint = 0;
  if (currentTime - lastStatusPrint >= 60000) {
    Serial.println("=== Status Report ===");
    Serial.print("WiFi: ");
    Serial.println(WiFi.status() == WL_CONNECTED ? "Connected" : "Disconnected");
    Serial.print("MQTT: ");
    Serial.println(mqttClient.connected() ? "Connected" : "Disconnected");
    Serial.print("Free Heap: ");
    Serial.println(ESP.getFreeHeap());
    lastStatusPrint = currentTime;
  }
  
  delay(2000); 
}