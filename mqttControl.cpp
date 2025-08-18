#include "mqttControl.h"
#include "secret.h"
#include "ledControl.h"
#include "global.h"
#include "utils.h"

const char* ledControlTopic = "server/A01-R02/assign";
//const char* completeTopic = "esp/ack";

void connectMQTT() {
  while (!client.connected()) {
    Serial.print("MQTT 연결 시도...");
    if (client.connect("ESP32-A02", MQTT_USER, MQTT_PASSWORD)) {
      Serial.println("연결 성공");
      if (client.subscribe(ledControlTopic)) {
        Serial.print("[MQTT] 구독 성공 → 토픽: ");
        Serial.println(ledControlTopic);
      }else {
        Serial.print("[MQTT] 구독 실패 → 토픽: ");
        Serial.println(ledControlTopic);
      }
    } else {
      Serial.printf("실패, 상태코드: %d. 5초 후 재시도\n", client.state());
      delay(5000);
    }
  }
}

void callback(char* topic, byte* payload, unsigned int length){
  Serial.println("call back 함수 호출");
  Serial.print("메시지 수신 [");
  Serial.print(topic);
  Serial.print("]: ");
  Serial.print("📏 메시지 길이: ");
  Serial.println(length);
  char jsonBuffer[length + 1];
  memcpy(jsonBuffer, payload, length);
  jsonBuffer[length] = '\0';
  Serial.println(jsonBuffer);
  getInformation(jsonBuffer);
}

void getInformation(const char* jsonBuffer) {
    Serial.println("=== getInformation() 시작 ===");

    // StaticJsonDocument 크기 충분히 확보
    StaticJsonDocument<512> doc;

    // --- JSON 파싱 ---
    if (!parseJson(jsonBuffer, doc)) {
        Serial.println("parseJson() 실패, 종료");
        return;
    }

    // --- 필수 필드 검증 ---
    if (!doc.containsKey("work_type") || !doc.containsKey("worker_id") || !doc.containsKey("position")) {
        Serial.println("필수 필드(work_type, worker_id, position) 중 하나 이상이 누락됨.");
        return;
    }

    const char* ledType = doc["work_type"] | "";
    workerId = doc["worker_id"] | "";
    position = doc["position"] | "";

    Serial.print("파싱된 work_type: ");
    Serial.println(ledType);
    Serial.print("파싱된 worker_id: ");
    Serial.println(workerId);
    Serial.print("파싱된 position: ");
    Serial.println(position);

    // --- Worker ID 검증 ---
    if (strcmp(workerId, "1237") != 0) {
        Serial.print("Worker ID 불일치. 예상: 1237, 실제: ");
        Serial.println(workerId);
        return;
    }
    Serial.println("Worker ID 검증 성공");

    // --- 제품 배열 초기화 및 파싱 ---
    productCount = 0;
    JsonArray productsArray = doc["products"];
    if (productsArray) {
        for (JsonObject product : productsArray) {
            if (productCount >= MAX_PRODUCTS) break;

            productIds[productCount] = product["product_id"].as<String>().c_str();
            products[productCount].weight = product["weight"] | 0.0f;
            products[productCount].quantity = product["quantity"] | 0;

            productCount++;
        }
    }

    // --- LED 동작 ---
    Serial.print("LED 켜기 시도: ");
    Serial.println(ledType);
    // measuring = true; // 필요하면 여기서 측정 시작
    turnOnLED(ledType);
}

