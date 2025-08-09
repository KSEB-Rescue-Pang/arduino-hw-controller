#ifndef LOAD_CELL_H
#define LOAD_CELL_H
#include <map>
#include <vector>
//#include <string>
#include <Arduino.h>
#include <PubSubClient.h>
#include <WiFiClientSecure.h>
void setLoadCell();
void calibrateWeight(const char* led);

struct ProductInfo {
  float weight;
  int quantity;
};

extern WiFiClientSecure secureClient;
extern PubSubClient mqttClient; 
extern std::vector<const char*> myVector;
extern std::map<const char*, ProductInfo> productMap;
#endif
