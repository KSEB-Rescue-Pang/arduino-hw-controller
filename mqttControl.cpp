#include <PubSubClient.h>
#include "mqttControl.h"
#include <map> 
#include "secret.h"
#include <vector>
#include <string>
#include "ledControl.h"

std::map<std::string, ProductInfo> productMap;
std::vector<string> myVector;

struct ProductInfo {
  float weight;
  int quantity;
};

const char* ledControlTopic = "server/z01-s01/assign"; 
WiFiClientSecure secureClient;
PubSubClient mqttClient(secureClient);

// MQTT 연결 함수
void connectMQTT() {
  if (mqttClient.connected()) return;
  Serial.print("Connecting to MQTT (TLS)...");

  String clientId = "ArduinoClient-" + String(random(0xffff), HEX);

  mqttClient.setServer(MQTT_BROKER, MQTT_PORT);
  mqttClient.setCallback(callback);

  if (mqttClient.connect(clientId.c_str(), MQTT_USER, MQTT_PASSWORD)) {
    Serial.println("connected");
    mqttClient.subscribe(ledControlTopic);
  }else {
    Serial.print(" failed, rc=");
    Serial.println(mqttClient.state());
  }
}

void callback(char *topic,byte* payload,unsigned int length){
  
  Serial.print("메시지 도착완료!");
  Serial.println(topic);
  String receivedTopic = String(topic);

  char jsonBuffer[length + 1];
  memcpy(jsonBuffer, payload, length);
  jsonBuffer[length] = '\0';
  Serial.println(jsonBuffer);

  ledControl(jsonBuffer);
  getProductInformation(jsonBuffer)
}

void getProductInformation(const char* jsonBuffer){
  StaticJsonDocument<100> doc;
  if (!parseJson(jsonBuffer, doc)) {
    // 파싱 실패 처리
    return;
  }

  // 이전에 저장된 데이터가 있다면 초기화합니다.
  productMap.clear();
  myVector.clear();

  JsonArray products = doc["products"];
  for(JsonObject product : products){
    const char* productId_c_str = product["product_id"];
    float weight = product["weight"].as<float>();
    int quantity = product["quantity"].as<int>();
    
    std::string productId_str(productId_c_str);
    myVector.push_back(productId_str);

    ProductInfo info;
    info.weight = weight;
    info.quantity = quantity;

    productMap[productId_str] = info;
  }
  
}