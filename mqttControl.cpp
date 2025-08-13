#include "global.h"
#include "ledControl.h"
#include "utils.h"
#include "secret.h"
#include "mqttControl.h"
#include "certificates.h"
#include "bleScan.h"
#include <time.h> // getLocalTime / configTime

//#include <Arduino.h>
#define DEBUG
const char* ledControlTopic = "server/A01-R01/assign";
//const char* ledControlTopic = "test/topic";

// static void ensureTimeSynced() {
//   // RTC가 맞지 않으면 TLS 인증서 검증 실패
//   // NTP로 시간 동기화 시도 (비동기/블로킹 단순 구현)
//   configTime(0, 0, "pool.ntp.org", "time.google.com");
//   Serial.println("[MQTT] NTP 동기화 시도...");
//   struct tm timeinfo;
//   // 5초 내에 시간 못 받아오면 실패로 처리(경고는 남김)
//   if (!getLocalTime(&timeinfo, 5000)) {
//     Serial.println("[MQTT] 시간 동기화 실패 (getLocalTime timeout). 인증서 검증 실패 가능성 있음.");
//   } else {
//     Serial.print("[MQTT] 현재 시간: ");
//     Serial.print(asctime(&timeinfo)); // asctime 포함된 개행 있음
//   }
// }

void connectMQTT() {
  if (mqttClient.connected()) return;

  // (1) TLS 인증서 검증을 위해 시간 먼저 맞춤
  //ensureTimeSynced();

  char clientId[25];
  sprintf(clientId, "ESP-%04X", random(0xFFFF));
  wifiClient.setInsecure();
  // (2) 루트 CA 등록 (ISRG Root X1)
  //wifiClient.setCACert(root_ca);

  // (3) SNI 설정: ESP32 최신 core에서 지원하는 setServerName() 사용
  // 컴파일 에러 시, setSSLHostname()로 바꿔보세요
  //wifiClient.setServerName(MQTT_BROKER);

  mqttClient.setServer(MQTT_BROKER, MQTT_PORT);
  mqttClient.setCallback(callback);
  // Keep-alive 시간 설정 (기본 15초를 30초로 늘림)
  mqttClient.setKeepAlive(30);
  
  Serial.print("[MQTT] 브로커 연결 시도: ");
  Serial.print(MQTT_BROKER);
  Serial.print(":");
  Serial.println(MQTT_PORT);
  Serial.println("[MQTT] Callback 함수가 설정됨 ✅");

  if (mqttClient.connect(clientId, MQTT_USER, MQTT_PASSWORD)) {
    #ifdef DEBUG  
      Serial.println("[MQTT] 연결 성공");
    #endif
    // QoS 1로 구독 시도 (메시지 전달 보장)
    bool subscribed = mqttClient.subscribe(ledControlTopic, 1);
    if (subscribed) {
        Serial.print("[MQTT] ✅ 구독 성공 (QoS 1) → 토픽: ");
        Serial.println(ledControlTopic);
        
        // 추가로 와일드카드 토픽도 구독해서 테스트
        String wildcardTopic = "server/A01-R01/+";
        bool wildcardSubscribed = mqttClient.subscribe(wildcardTopic.c_str(), 1);
        Serial.print("[MQTT] 와일드카드 구독: ");
        Serial.print(wildcardSubscribed ? "✅ 성공" : "❌ 실패");
        Serial.print(" → ");
        Serial.println(wildcardTopic);
    } else {
        Serial.print("[MQTT] ❌ 구독 실패 → 토픽: ");
        Serial.println(ledControlTopic);
    }
    //Serial.println(ledControlTopic);
  } else {
    #ifdef DEBUG
      Serial.print("[MQTT] ❌ 연결 실패 (state: ");
      Serial.print(mqttClient.state());
      Serial.println(")");
      
      // MQTT 연결 실패 코드 설명
      switch (mqttClient.state()) {
        case -4: Serial.println("   → 타임아웃"); break;
        case -3: Serial.println("   → 연결 끊어짐"); break;
        case -2: Serial.println("   → 네트워크 실패"); break;
        case -1: Serial.println("   → 연결 중단됨"); break;
        case 1: Serial.println("   → 잘못된 프로토콜 버전"); break;
        case 2: Serial.println("   → 잘못된 클라이언트 ID"); break;
        case 3: Serial.println("   → 서버 사용 불가"); break;
        case 4: Serial.println("   → 잘못된 인증 정보"); break;
        case 5: Serial.println("   → 권한 없음"); break;
        default: Serial.println("   → 알 수 없는 오류"); break;
      }
      
      // 시간 재확인 로그 추가
      struct tm timeinfo;
      if (getLocalTime(&timeinfo, 1000)) {
        Serial.print("[MQTT] 현재 시간(재확인): ");
        Serial.print(asctime(&timeinfo));
      } else {
        Serial.println("[MQTT] 현재 시간 확인 불가");
      }
    #endif
  }
}

// void connectMQTT() {
//     if (mqttClient.connected()) return;

//     char clientId[25];
//     sprintf(clientId, "ESP-%04X", random(0xFFFF));

//     mqttClient.setServer(MQTT_BROKER, MQTT_PORT);
//     mqttClient.setCallback(callback);

//     Serial.print("[MQTT] 브로커 연결 시도: ");
//     Serial.print(MQTT_BROKER);
//     Serial.print(":");
//     Serial.println(MQTT_PORT);

//     if (mqttClient.connect(clientId)) {
//         Serial.println("[MQTT] 연결 성공");
//         // ✅ 연결 성공 시 항상 구독
//         mqttClient.subscribe(ledControlTopic, 2);
//         Serial.print("[MQTT] 구독완료 → 토픽: ");
//         Serial.println(ledControlTopic);
//     } else {
//         Serial.print("[MQTT] 연결 실패 (state: ");
//         Serial.print(mqttClient.state());
//         Serial.println(")");
//     }
// }

// void connectMQTT() {
//     if (mqttClient.connected()) return;

//     char clientId[25];
//     sprintf(clientId, "ESP-%04X", random(0xFFFF));

//     mqttClient.setServer(MQTT_BROKER, MQTT_PORT);
//     mqttClient.setCallback(callback);

//     Serial.print("[MQTT] 브로커 연결 시도: ");
//     Serial.print(MQTT_BROKER);
//     Serial.print(":");
//     Serial.println(MQTT_PORT);

//     if (mqttClient.connect(clientId)) {
//         Serial.println("[MQTT] 연결 성공");

//         bool subscribed = mqttClient.subscribe(ledControlTopic, 0);
//         if (subscribed) {
//             Serial.print("[MQTT] 구독 성공 → 토픽: ");
//             Serial.println(ledControlTopic);
//         } else {
//             Serial.print("[MQTT] 구독 실패 → 토픽: ");
//             Serial.println(ledControlTopic);
//         }
//     } else {
//         Serial.print("[MQTT] 연결 실패 (state: ");
//         Serial.print(mqttClient.state());
//         Serial.println(")");
//     }
// }


void callback(char *topic, uint8_t* payload,unsigned int length){
  Serial.println("🔔🔔🔔 CALLBACK 함수 호출됨! 🔔🔔🔔");
  Serial.println("========================================");
  Serial.print("📍 수신된 토픽: ");
  Serial.println(topic);
  Serial.print("📏 메시지 길이: ");
  Serial.println(length);
  
  char jsonBuffer[length + 1];
  memcpy(jsonBuffer, payload, length);
  jsonBuffer[length] = '\0';
  
  Serial.print("📄 수신된 메시지 내용: ");
  Serial.println(jsonBuffer);
  Serial.println("========================================");
  
  ledControl(jsonBuffer);
  getProductInformation(jsonBuffer);
  processTasks();
  
  Serial.println("✅ Callback 처리 완료");
}

void getProductInformation(const char* jsonBuffer){
  StaticJsonDocument<150> doc;
  if (!parseJson(jsonBuffer, doc)) {
    return;
  }
  productCount = 0;
  
  for (int i = 0; i < MAX_PRODUCTS; i++) {
    productIds[i] = ""; // 혹은 ""로 초기화 (문자열이라면)
    products[i].weight = 0.0;
    products[i].quantity = 0;
  }

  JsonArray productsArray = doc["products"];
  for(JsonObject product : productsArray){
    if(productCount >= MAX_PRODUCTS) break;
    
    productIds[productCount] = product["product_id"];
    products[productCount].weight = product["weight"].as<float>();
    products[productCount].quantity = product["quantity"].as<int>();
    productCount++;
  } 
}
