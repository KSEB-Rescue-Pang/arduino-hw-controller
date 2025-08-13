#include "ledControl.h"
#include "wifiConnect.h"
#include "loadCell.h"
#include "bleScan.h"
#include "mqttControl.h"
#include "global.h"

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  setLED();
  setLoadCell();
  connectWiFi();
}

void loop() {
  // put your main code here, to run repeatedly:
  static unsigned long lastStatusCheck = 0;
  unsigned long currentTime = millis();
  
  if (!mqttClient.connected()) {
    Serial.println("🔄 MQTT 연결 끊어짐, 재연결 시도...");
    connectMQTT();//mqtt 연결이 안 되었을 떄 재 연결 시도
  }
  
  mqttClient.loop(); // mqtt 연결 유지
  
  // 30초마다 상태 체크
  if (currentTime - lastStatusCheck > 30000) {
    Serial.print("📊 MQTT 상태 체크 - 연결됨: ");
    Serial.print(mqttClient.connected() ? "✅" : "❌");
    Serial.print(", State: ");
    Serial.println(mqttClient.state());
    lastStatusCheck = currentTime;
  }
  
  processTasks();
  delay(2000); 
}
