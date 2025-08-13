#include "wifiConnect.h"
#include "global.h"
#include "secret.h"
#include <Arduino.h>
#include <WiFi.h>

void connectWiFi() {
  if (WiFi.status() == WL_CONNECTED) {
    //Serial.println("WiFi 이미 연결됨");
    return;
  } 

  //Serial.print("WiFi 연결 시도 중...");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi 연결 성공");
  } else {
    Serial.println("\nWiFi 연결 실패");
  }
}
