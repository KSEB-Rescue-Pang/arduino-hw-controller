#include "ledControl.h"
#include "wifiConnect.h"
#include "loadCell.h"
#include "bleScan.h"
#include "mqttControl.h"
 
void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  setLED();
  setLoadCell();
}

void loop() {
  // put your main code here, to run repeatedly:
  connectWiFi();
  if (!mqttClient.connected()) {
    connectMQTT();//mqtt 연결이 안 되었을 떄 재 연결 시도
  }
  mqttClient.loop(); // mqtt 연결 유지
  processTasks();
  delay(2000); 
}
