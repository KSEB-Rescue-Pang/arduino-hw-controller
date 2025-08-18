#include "ledControl.h"
#include "utils.h"
#include "global.h"
#include "loadCell.h"
#include "wifiScan.h"
const int RED_PIN = 25;
const int YELLOW_PIN = 26;
const int GREEN_PIN = 27;

void setLED() {
  pinMode(RED_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(YELLOW_PIN, OUTPUT);
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
    Serial.print("LED 켜짐 - Pin: ");
    Serial.println(pin);
    digitalWrite(pin, HIGH);
  } else {
    Serial.print("LED 켜기 실패 - 잘못된 ledType: ");
    Serial.println(ledType);
  }
  Serial.println("runningHardware 진입시작");
  //runningHardware(ledType);
  runningHardware(pin);
}

void getReadyBeforeFlickering(int led){
    Serial.println("깜박거리기 직전");
    flickerLED(led, 50);
}

void flickerLED(int ledPin, int delayTime) {
    Serial.println("flickerLED 호출");
    unsigned long previousMillis = millis();
    bool ledState = LOW;

    int idx = 0;
    float outBoundStandard = 0.0f;
    bool isInbound = (ledPin == 27);

    // ✅ 출고일 때 기준 무게 검증
    if (!isInbound) {
        outBoundStandard = 0.0f;
        int totalQuantity = 0;
        for (int i = 0; i < productCount; i++) {
            outBoundStandard += products[i].weight * products[i].quantity;
            totalQuantity += products[i].quantity;
        }

        Serial.printf("출고 기준 총 무게: %.2f\n", outBoundStandard);
        Serial.println("선반 위 무게 측정 중...");
        delay(4000);

        float measured = myFabs(myRoundf2(scale.get_units(10)));
        Serial.printf("현재 측정 무게: %.2f\n", measured);

        if (myFabs(measured - outBoundStandard) > 0.1f) {
            Serial.println("출고 불가 (기준 무게 불일치)");
            return; // LED 안 켜고 종료
        }
        Serial.println("출고 작업 시작");
    } else {
        Serial.println("입고 작업 시작");
    }

    measuring = true;

    while(measuring) {
        unsigned long currentMillis = millis();
        if (currentMillis - previousMillis >= delayTime) {
            previousMillis = currentMillis;
            ledState = !ledState;
            digitalWrite(ledPin, ledState);
        }

        if (idx < productCount) {
            if (calibrateWeightStep(ledPin, idx, outBoundStandard)) {
                idx++; // 다음 제품으로 이동
            }
        } else {
            measuring = false;
            turnOffLED(ledPin);
        }

        //delay(50);
    }
}

void turnOffLED(int ledType) {
  if (ledType != -1) {
    digitalWrite(ledType, LOW);
  }
}
