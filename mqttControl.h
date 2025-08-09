#ifndef MQTT_CONTROL_H
#define MQTT_CONTROL_H

#include <PubSubClient.h>
extern PubSubClient mqttClient;
#include <Arduino.h>
void connectMQTT();
void getProductInformation(const char* jsonBuffer);
void callback(char* topic, uint8_t* payload, unsigned int length);
#endif