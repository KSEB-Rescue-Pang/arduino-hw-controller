#ifndef LOAD_CELL_H
#define LOAD_CELL_H
#include <Arduino.h>
#include <PubSubClient.h>
#include <WiFiClientSecure.h>
void setLoadCell();
void calibrateWeight(const char* led);

struct ProductInfo {
  float weight;
  int quantity;
};

#define MAX_PRODUCTS 10
extern WiFiClientSecure secureClient;
extern PubSubClient mqttClient; 
extern const char* productIds[MAX_PRODUCTS];
extern ProductInfo products[MAX_PRODUCTS];
extern int productCount;
#endif
