#include <Arduino.h>
#include "ledControl.h"
#include "loadCell.h"
#include "utils.h"

extern int lastRSSI;

String ledType;
String workerId;

const int RED_PIN = 26;
const int GREEN_PIN = 27;
const int YELLOW_PIN = 14;


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

void manageHardWare(int rssi, String led) {
  if (rssi > -45) {
    int pinToFlicker = getPinForLED(led);
    if (pinToFlicker != -1) {
      flickerLED(pinToFlicker, 500);
      calibrateWeight(led);
    }
    digitalWrite(YELLOW_PIN, LOW);
    digitalWrite(GREEN_PIN, LOW);
  } else {
    Serial.println("가까이 있는 핸드폰이 잡히지 않습닌다.")
  }
}

int getPinForLED(String ledType){
  if(ledType == "OB"){
    ledPin = RED_PIN;
  }else if(ledType == "IB"){
    ledPin = GREEN_PIN;
  }
  return ledPin;
}

void turnOnLED(String ledType){
  int pin = getPinForLED(ledType);
  if(pin !=-1){
    digitalWrite(pin,HIGH);
    Serial.println("LED ON: " + ledType + " (핀번호: " + String(pin) + ")");
  }
}

void turnOffLED(String ledType){
  int pin = getPinForLED(ledType);
  if(pin !=-1){
    digitalWrite(pin,LOW);
    Serial.println("LED OFF: " + ledType + " (핀번호: " + String(pin) + ")");
  } 
}

void ledControl(const char* jsonBuffer) {
  StaticJsonDocument<100> doc;
  if (!parseJson(jsonBuffer, doc)) {
    // 파싱 실패 처리
    return;
  }
  
  ledType = doc["work_type"].as<String>();
  workerId = doc["worker_id"].as<String>();

  if (workerId != "2011") {
    Serial.println("작업자 아이디가 일치하지 않아서 동작하지 않습니다.");
    return;
  }
  getPinForLED();
  turnOnLED(ledType);
  Serial.println("현재 ESP위치의 LED를 켭니다.");
}