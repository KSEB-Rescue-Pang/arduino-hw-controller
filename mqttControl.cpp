#include "loadCell.h"
#include "mqttControl.h"
#include "ledControl.h"
#include "wifiConnect.h"
#include "utils.h"

const char* productIds[MAX_PRODUCTS];
ProductInfo products[MAX_PRODUCTS];
int productCount = 0;

const char* ledControlTopic = "server/A01-R01/assign"; 
WiFiClientSecure secureClient;
PubSubClient mqttClient(secureClient);

// MQTT 연결 함수
void connectMQTT() {
  if (mqttClient.connected()) return;
  char clientId[25];
  sprintf(clientId, "ESP-%04X", random(0xFFFF));

  mqttClient.setServer(MQTT_BROKER, MQTT_PORT);
  mqttClient.setCallback(callback);

  if (mqttClient.connect(clientId, MQTT_USER, MQTT_PASSWORD)) {
    mqttClient.subscribe(ledControlTopic);
  }
}

void callback(char *topic,uint8_t* payload,unsigned int length){

  char jsonBuffer[length + 1];
  memcpy(jsonBuffer, payload, length);
  jsonBuffer[length] = '\0';
  ledControl(jsonBuffer);
  getProductInformation(jsonBuffer);
}

void getProductInformation(const char* jsonBuffer){
  StaticJsonDocument<200> doc;
  if (!parseJson(jsonBuffer, doc)) {
    return;
  }

  productCount = 0;

  JsonArray productsArray = doc["products"];
  for(JsonObject product : productsArray){
    if(productCount >= MAX_PRODUCTS) break;
    
    productIds[productCount] = product["product_id"];
    products[productCount].weight = product["weight"].as<float>();
    products[productCount].quantity = product["quantity"].as<int>();
    productCount++;
  } 
}