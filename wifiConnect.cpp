#include "wifiConnect.h"
//#include "secret.h"

void connectWiFi() {
  if (WiFi.status() == WL_CONNECTED) return;
  #ifdef DEBUG
    Serial.print("Connecting to WiFi");
  #endif
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    #ifdef DEBUG
      Serial.print(".");
    #endif
    attempts++;
  }
  if (WiFi.status() == WL_CONNECTED) {
    #ifdef DEBUG
      Serial.println("\nWiFi connected! IP: " + WiFi.localIP().toString());
    #endif
  } else {
    #ifdef DEBUG
      Serial.println("\nWiFi connection failed.");
    #endif
  }
}