#include "ledControl.h"
#include "wifiConnect.h"
#include "loadCell.h"
#include "bleScan.h"
#include "mqttControl.h"
#include "global.h"

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  
  Serial.println("=====================================");
  Serial.println("🔧 ESP32 하드웨어 컨트롤러 시작");
  Serial.println("=====================================");
  
  // ESP32 고유 정보 출력
  Serial.println("📟 ESP32 정보:");
  Serial.print("   MAC Address: ");
  Serial.println(WiFi.macAddress());
  
  uint64_t chipid = ESP.getEfuseMac();
  Serial.printf("   Chip ID: %04X%08X\n", (uint16_t)(chipid>>32), (uint32_t)chipid);
  
  Serial.print("   Flash Size: ");
  Serial.print(ESP.getFlashChipSize() / (1024 * 1024));
  Serial.println(" MB");
  
  Serial.println("-------------------------------------");
  
  setLED();
  setLoadCell();
  connectWiFi();
  
  // WiFi 연결 후 네트워크 정보 출력
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("🌐 네트워크 정보:");
    Serial.print("   연결된 SSID: ");
    Serial.println(WiFi.SSID());
    Serial.print("   IP Address: ");
    Serial.println(WiFi.localIP());
    Serial.print("   Gateway: ");
    Serial.println(WiFi.gatewayIP());
    Serial.print("   DNS: ");
    Serial.println(WiFi.dnsIP());
    Serial.print("   Signal Strength: ");
    Serial.print(WiFi.RSSI());
    Serial.println(" dBm");
  }
  
  Serial.println("=====================================");
  Serial.println("✅ 초기화 완료 - 메인 루프 시작");
  Serial.println("=====================================");
}

void loop() {
  // put your main code here, to run repeatedly:
  static unsigned long lastStatusCheck = 0;
  static unsigned long connectionTime = 0;
  static bool testMessageSent = false;
  unsigned long currentTime = millis();
  
  if (!mqttClient.connected()) {
    Serial.println("🔄 MQTT 연결 끊어짐, 재연결 시도...");
    connectMQTT();//mqtt 연결이 안 되었을 떄 재 연결 시도
    connectionTime = currentTime; // 연결 시간 기록
    testMessageSent = false; // 테스트 메시지 플래그 리셋
  }
  
  mqttClient.loop(); // mqtt 연결 유지
  
  // 연결 후 30초 뒤 자가 테스트 메시지 발송 (한 번만)
  if (mqttClient.connected() && !testMessageSent && (currentTime - connectionTime > 30000)) {
    Serial.println("🧪 자가 테스트 메시지 발송 중...");
    String testMessage = "{\"work_type\":\"IB\",\"worker_id\":\"1237\",\"products\":[{\"product_id\":\"TEST001\",\"weight\":10.5,\"quantity\":1}]}";
    bool published = mqttClient.publish("server/A01-R01/assign", testMessage.c_str());
    Serial.print("📤 테스트 메시지 발송: ");
    Serial.println(published ? "✅ 성공" : "❌ 실패");
    Serial.print("📄 발송 내용: ");
    Serial.println(testMessage);
    testMessageSent = true;
  }
  
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
