#include "loadCell.h"
#include "mqttControl.h"
#include "loadCell.h"
#include "ledControl.h"
#include "wifiConnect.h"
#include "utils.h"
//using namespace std;
std::map<const char*, ProductInfo> productMap;
std::vector<const char*> myVector;

const char* ledControlTopic = "server/A01-R01/assign"; 
WiFiClientSecure secureClient;
PubSubClient mqttClient(secureClient);

// MQTT 연결 함수
void connectMQTT() {
  if (mqttClient.connected()) return;
  #ifdef DEBUG
    Serial.print("Connecting to MQTT (TLS)...");
  #endif
  char clientId[30];  // 충분한 크기로 선언
  sprintf(clientId, "ArduinoClient-%04X", random(0xFFFF));

  mqttClient.setServer(MQTT_BROKER, MQTT_PORT);
  mqttClient.setCallback(callback);

  if (mqttClient.connect(clientId, MQTT_USER, MQTT_PASSWORD)) {
    #ifdef DEBUG
      Serial.println("connected");
    #endif
    mqttClient.subscribe(ledControlTopic);
  }else {
    #ifdef DEBUG
      Serial.print(" failed, rc=");
      Serial.println(mqttClient.state());
    #endif
  }
}

void callback(char *topic,uint8_t* payload,unsigned int length){
  
  #ifdef DEBUG
    Serial.print("메시지 도착완료!");
    Serial.println(topic);
  #endif
  const char *receivedTopic = topic;

  char jsonBuffer[length + 1];
  memcpy(jsonBuffer, payload, length);
  jsonBuffer[length] = '\0';
  #ifdef DEBUG
    Serial.println(jsonBuffer);
  #endif
  ledControl(jsonBuffer);
  getProductInformation(jsonBuffer);
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
    
    //std::string productId_str(productId_c_str);
    myVector.push_back(productId_c_str);

    ProductInfo info;
    info.weight = weight;
    info.quantity = quantity;

    productMap[productId_c_str] = info;
  } 
}