#include <Arduino.h>
#include "ledControl.h"
#include "loadCell.h"
#include "utils.h"


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
  }
}

void turnOffLED(const char* ledType) {
  int pin = getPinForLED(ledType);
  if (pin != -1) {
    digitalWrite(pin, LOW);
  }
}

void ledControl(const char* jsonBuffer) {
  StaticJsonDocument<200> doc;
  if (!parseJson(jsonBuffer, doc)) {
    return;
  }

  strncpy(ledType, doc["work_type"] | "", sizeof(ledType) - 1);
  ledType[sizeof(ledType) - 1] = '\0';

  strncpy(workerId, doc["worker_id"] | "", sizeof(workerId) - 1);
  workerId[sizeof(workerId) - 1] = '\0';

  if (strcmp(workerId, "2011") != 0) {
    return;
  }

  turnOnLED(ledType);
}
