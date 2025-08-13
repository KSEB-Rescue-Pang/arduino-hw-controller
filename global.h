#ifndef GLOBAL_H
#define GLOBAL_H
#define MAX_PRODUCTS 10 

#include <WiFiClientSecure.h>//와이파이
//#include <WiFiClient>
//#include <WiFi.h> 
//extern WiFiClient wifiClient;
extern WiFiClientSecure wifiClient;

#include <PubSubClient.h>//mqtt통신
extern PubSubClient mqttClient;

struct ProductInfo {
  float weight;
  int quantity;
};

extern const char* productIds[MAX_PRODUCTS];
extern ProductInfo products[MAX_PRODUCTS];
extern int productCount;

extern char ledType[10];
extern char workerId[10];
 
#endif