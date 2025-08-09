#include <Arduino.h>
#include "ledControl.h"
#include "loadCell.h"
#include "utils.h"

extern int lastRSSI;

char ledType[10] = {0};
char workerId[10] = {0};

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
  } else {
    Serial.println(F("가까이 있는 핸드폰이 잡히지 않습닌다."));
  }
}

int getPinForLED(const char* ledType) {
  if (strcmp(ledType, "OB") == 0) {
    return RED_PIN;
  } else if (strcmp(ledType, "IB") == 0) {
    return GREEN_PIN;
  }
  return -1;
}

void turnOnLED(const char* ledType) {
  int pin = getPinForLED(ledType);
  if (pin != -1) {
    digitalWrite(pin, HIGH);
    #ifdef DEBUG
      Serial.print("LED ON: ");
      Serial.print(ledType);
    #endif
    //Serial.print(" (핀번호: ");
    //Serial.print(pin);
    //Serial.println(")");
  }
}

void turnOffLED(const char* ledType) {
  int pin = getPinForLED(ledType);
  if (pin != -1) {
    digitalWrite(pin, LOW);
    #ifdef DEBUG
      Serial.print("LED OFF: ");
      Serial.print(ledType);
    #endif
    //Serial.print(" (핀번호: ");
    //Serial.print(pin);
    //Serial.println(")");
  }
}

void ledControl(const char* jsonBuffer) {
  StaticJsonDocument<100> doc;
  if (!parseJson(jsonBuffer, doc)) {
    // 파싱 실패 처리
    return;
  }

  // JSON에서 값 복사 (최대 크기 -1 까지 복사 후 null 문자 추가)
  strncpy(ledType, doc["work_type"] | "", sizeof(ledType) - 1);
  ledType[sizeof(ledType) - 1] = '\0';

  strncpy(workerId, doc["worker_id"] | "", sizeof(workerId) - 1);
  workerId[sizeof(workerId) - 1] = '\0';

  if (strcmp(workerId, "2011") != 0) {
    #ifdef DEBUG
      Serial.println(F("작업자 아이디가 일치하지 않아서 동작하지 않습니다."));
    #endif
    return;
  }

  turnOnLED(ledType);
  #ifdef DEBUG
    Serial.println(F("현재 ESP위치의 LED를 켭니다."));
  #endif
}
