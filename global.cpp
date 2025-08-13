#include "global.h"

WiFiClientSecure wifiClient;
//WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

const char* productIds[MAX_PRODUCTS];
ProductInfo products[MAX_PRODUCTS];
int productCount = 0;

char ledType[10];
char workerId[10];
