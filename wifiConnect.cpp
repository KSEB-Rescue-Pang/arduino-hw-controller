#include "wifiConnect.h"
#include <WiFi.h>

void setup_wifi(const char* ssid, const char* password) {
  Serial.printf("WiFi 연결 중: %s\n", ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi 연결 완료");
  Serial.print("IP 주소: ");
  Serial.println(WiFi.localIP());
}