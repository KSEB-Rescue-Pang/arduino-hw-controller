#ifndef MQTT_CONTROL_H
#define MQTT_CONTROL_H
#include <Arduino.h>
void connectMQTT();
void callback(char* topic, byte* payload, unsigned int length);
void getInformation(const char* jsonBuffer);
#endif