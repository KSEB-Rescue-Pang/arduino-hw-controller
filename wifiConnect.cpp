#include "wifiConnect.h"

void connectWiFi() {
  if (WiFi.status() == WL_CONNECTED) {
    Serial.print("WiFi already connected. IP: ");
    Serial.println(WiFi.localIP());
    return;
  }
  
  Serial.println("Connecting to WiFi...");
  Serial.print("SSID: ");
  Serial.println(WIFI_SSID);
  
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  int attempts = 0;
  
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println();
    Serial.println("WiFi connected successfully!");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    Serial.print("Signal strength (RSSI): ");
    Serial.print(WiFi.RSSI());
    Serial.println(" dBm");
  } else {
    Serial.println();
    Serial.println("WiFi connection failed!");
    Serial.print("Status code: ");
    Serial.println(WiFi.status());
    
    // WiFi 상태 코드 설명
    switch(WiFi.status()) {
      case WL_NO_SHIELD: Serial.println("Error: No WiFi shield"); break;
      case WL_IDLE_STATUS: Serial.println("Status: Idle"); break;
      case WL_NO_SSID_AVAIL: Serial.println("Error: SSID not available"); break;
      case WL_SCAN_COMPLETED: Serial.println("Status: Scan completed"); break;
      case WL_CONNECT_FAILED: Serial.println("Error: Connection failed"); break;
      case WL_CONNECTION_LOST: Serial.println("Error: Connection lost"); break;
      case WL_DISCONNECTED: Serial.println("Status: Disconnected"); break;
    }
  }
}