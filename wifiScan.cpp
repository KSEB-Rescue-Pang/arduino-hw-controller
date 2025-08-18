#include "wifiScan.h"
#include "ledControl.h"
#include <WiFi.h>
#include <Arduino.h>
const char* MY_PHONE_KEYWORD = "";

void runningHardware(int ledType) {
  Serial.println("runningHardware() 호출");

  int n = WiFi.scanNetworks(); // 주변 Wi-Fi AP 스캔
  int maxRSSI = -100;
  bool phoneFound = false;

  for (int i = 0; i < n; i++) {
    String ssid = WiFi.SSID(i);
    int rssi = WiFi.RSSI(i);

    Serial.print("  SSID: "); Serial.print(ssid);
    Serial.print(" | RSSI: "); Serial.println(rssi);

    if (ssid.indexOf(MY_PHONE_KEYWORD) >= 0) {
      phoneFound = true;
      if (rssi > maxRSSI) maxRSSI = rssi;
      Serial.print("📱 내 핸드폰 발견! RSSI: "); Serial.println(rssi);
    }
  }
  Serial.println("깜빡거릴 준비하러 감");
  getReadyBeforeFlickering(ledType);
}
