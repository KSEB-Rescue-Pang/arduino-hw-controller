#ifndef GLOBAL_H
#define GLOBAL_H
#define MAX_PRODUCTS 10

#include <WiFiClientSecure.h>
extern WiFiClientSecure espClient;
#include "HX711.h"
#include <PubSubClient.h>//mqtt통신
extern PubSubClient client;

struct ProductInfo {
  float weight;
  int quantity;
};
extern const char* position;
extern const char* productIds[MAX_PRODUCTS];
extern ProductInfo products[MAX_PRODUCTS];
extern int productCount;
extern bool measuring;
extern const char* workerId;
extern HX711 scale;
#endif