#include <Arduino.h>
#include "ledControl.h"
#include "loadCell.h"
#include "utils.h"
#include "global.h"

const int RED_PIN = 25;
const int GREEN_PIN = 26;
const int YELLOW_PIN = 27;

void setLED() {
  pinMode(RED_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(YELLOW_PIN, OUTPUT);
  digitalWrite(RED_PIN, LOW);
  digitalWrite(GREEN_PIN, LOW);
  digitalWrite(YELLOW_PIN, LOW);
}

void flickerLED(int ledPin, int delayTime) {
  digitalWrite(ledPin, HIGH);
  delay(delayTime);
  digitalWrite(ledPin, LOW);
  delay(delayTime);
}

void manageHardWare(int rssi, const char* led) {
  if (rssi > -45) {
    int pinToFlicker = getPinForLED(led);
    if (pinToFlicker != -1) {
      flickerLED(pinToFlicker, 500);
      calibrateWeight(led);
    }
    digitalWrite(YELLOW_PIN, LOW);
    digitalWrite(GREEN_PIN, LOW);
  }
}

int getPinForLED(const char* ledType) {
  if (strcmp(ledType, "OB") == 0) {
    Serial.println("출고");
    return RED_PIN;
  } else if (strcmp(ledType, "IB") == 0) {
    Serial.println("입고");
    return GREEN_PIN;
  }
  return -1;
}

void turnOnLED(const char* ledType) {
  Serial.print("turnOnLED() 호출됨. ledType: ");
  Serial.println(ledType);
  
  int pin = getPinForLED(ledType);
  if (pin != -1) {
    Serial.print("✅ LED 켜짐 - Pin: ");
    Serial.println(pin);
    digitalWrite(pin, HIGH);
  } else {
    Serial.print("❌ LED 켜기 실패 - 잘못된 ledType: ");
    Serial.println(ledType);
  }
}

void turnOffLED(const char* ledType) {
  int pin = getPinForLED(ledType);
  if (pin != -1) {
    digitalWrite(pin, LOW);
  }
}

void ledControl(const char* jsonBuffer) {
  Serial.println("=== ledControl() 시작 ===");
  Serial.print("수신된 JSON: ");
  Serial.println(jsonBuffer);
  
  StaticJsonDocument<150> doc;
  if (!parseJson(jsonBuffer, doc)) {
    Serial.println("❌ JSON 파싱 실패");
    return;
  }
  Serial.println("✅ JSON 파싱 성공");

  memset(ledType, 0, sizeof(ledType));
  memset(workerId, 0, sizeof(workerId));

  if (!doc.containsKey("work_type")) {
    Serial.println("❌ work_type 필드가 없음");
    return;
  }
  if (!doc.containsKey("worker_id")) {
    Serial.println("❌ worker_id 필드가 없음");
    return;
  }

  strncpy(ledType, doc["work_type"] | "", sizeof(ledType) - 1);
  ledType[sizeof(ledType) - 1] = '\0';

  strncpy(workerId, doc["worker_id"] | "", sizeof(workerId) - 1);
  workerId[sizeof(workerId) - 1] = '\0';

  Serial.print("파싱된 work_type: ");
  Serial.println(ledType);
  Serial.print("파싱된 worker_id: ");
  Serial.println(workerId);

  if (strcmp(workerId, "1237") != 0) {
    Serial.print("❌ Worker ID 불일치. 예상: 1237, 실제: ");
    Serial.println(workerId);
    return;
  }
  Serial.println("✅ Worker ID 검증 성공");
  
  Serial.print("LED 켜기 시도: ");
  Serial.println(ledType);
  turnOnLED(ledType);
  Serial.println("=== ledControl() 완료 ===");
}
