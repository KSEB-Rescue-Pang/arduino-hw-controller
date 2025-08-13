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
  if (mqttClient.connected()) {
    Serial.println("MQTT already connected");
    return;
  }
  
  // WiFi 연결 상태 확인
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi not connected, cannot connect to MQTT");
    return;
  }
  
  Serial.println("Attempting MQTT connection...");
  
  // TLS 설정 - HiveMQ Cloud는 TLS 필요
  secureClient.setInsecure(); // 개발용 - 프로덕션에서는 적절한 인증서 사용
  
  char clientId[25];
  sprintf(clientId, "ESP-%04X", random(0xFFFF));
  Serial.print("Client ID: ");
  Serial.println(clientId);

  mqttClient.setServer(MQTT_BROKER, MQTT_PORT);
  mqttClient.setCallback(callback);
  
  Serial.print("Connecting to MQTT broker: ");
  Serial.println(MQTT_BROKER);

  if (mqttClient.connect(clientId, MQTT_USER, MQTT_PASSWORD)) {
    Serial.println("MQTT connected successfully!");
    mqttClient.subscribe(ledControlTopic);
    Serial.print("Subscribed to topic: ");
    Serial.println(ledControlTopic);
  } else {
    Serial.print("MQTT connection failed, rc=");
    Serial.print(mqttClient.state());
    Serial.println(" retrying in next loop...");
    
    // MQTT 에러 코드 설명
    switch(mqttClient.state()) {
      case -4: Serial.println("Error: Connection timeout"); break;
      case -3: Serial.println("Error: Connection lost"); break;
      case -2: Serial.println("Error: Connect failed"); break;
      case -1: Serial.println("Error: Disconnected"); break;
      case 1: Serial.println("Error: Bad protocol"); break;
      case 2: Serial.println("Error: Bad client ID"); break;
      case 3: Serial.println("Error: Unavailable"); break;
      case 4: Serial.println("Error: Bad credentials"); break;
      case 5: Serial.println("Error: Unauthorized"); break;
    }
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