#ifndef MQTT_CONTROL_H
#define MQTT_CONTROL_H

void connectMQTT();
void getProductInformation(const char* jsonBuffer);
void callback(char* topic, byte* payload, unsigned int length);

#endif