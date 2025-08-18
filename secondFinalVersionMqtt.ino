#include "global.h"
#include "wifiConnect.h"
#include "secret.h"
#include "certificate.h"
#include "mqttControl.h"
#include "ledControl.h"
#include "loadCell.h"
void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  setLED();
  setLoadCell();
  setup_wifi(WIFI_SSID, WIFI_PASSWORD);
  espClient.setCACert(root_ca);
  client.setServer(MQTT_BROKER, MQTT_PORT);
  client.setCallback(callback);
}

void loop() {
  // put your main code here, to run repeatedly:
  if (!client.connected()) {
    connectMQTT();
  }
  client.loop();
}