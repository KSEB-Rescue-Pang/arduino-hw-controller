#include "global.h"

WiFiClientSecure espClient;
PubSubClient client(espClient);
const char* productIds[MAX_PRODUCTS];
ProductInfo products[MAX_PRODUCTS];
bool measuring = true;
int productCount = 0;
const char* position;
const char* workerId;
HX711 scale;