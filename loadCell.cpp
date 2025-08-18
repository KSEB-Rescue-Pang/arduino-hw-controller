#include "loadCell.h"
#include "ledControl.h"
#include "utils.h"
#include "global.h"
const int LOADCELL_DOUT_PIN = 4;
const int LOADCELL_SCK_PIN = 5;

float myFabs(float x) {
  return (x < 0) ? -x : x;
}

float myRoundf2(float x) {
  if (x >= 0) {
    return (float)((int)(x * 100 + 0.5f)) / 100.0f;
  } else {
    return (float)((int)(x * 100 - 0.5f)) / 100.0f;
  }
}

void setLoadCell(){
  int calibration_factor = 430000;
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  scale.set_scale(calibration_factor);
  scale.tare();
}

bool calibrateWeightStep(int led, int idx, float outBoundStandard) {
    bool isInbound = (led == 27);
    float measured = myFabs(myRoundf2(scale.get_units(10)));
    Serial.print("현재 측정된 무게: ");
    Serial.println(measured);

    StaticJsonDocument<128> doc;
    doc["worker_id"] = workerId;
    char posUpper = (*position >= 'a' && *position <= 'z') ? (*position - 'a' + 'A') : *position;
    char locationId[16];
    snprintf(locationId, sizeof(locationId), "A01-R01-%c", posUpper);
    doc["location_id"] = locationId;

    const char* sendTopic = "esp/ack";
    char buffer[128];

    if (isInbound) {  
        
        float cumulativeExpectedWeight = 0.0f;
        for (int i = 0; i <= idx; i++) {
            cumulativeExpectedWeight += products[i].weight * products[i].quantity;
        }

        Serial.printf("입고 idx=%d, 기대 누적 무게=%.2f, 측정 무게=%.2f\n",
                      idx, cumulativeExpectedWeight, measured);

        if (myFabs(measured - cumulativeExpectedWeight) < 0.01f) {
            doc["code"] = "good";
            serializeJson(doc, buffer);
            client.publish(sendTopic, buffer);
            Serial.printf("제품 %d 입고 완료\n", idx);
            return true;
        } else {
            Serial.printf("제품 %d 입고 대기 중...\n", idx);
            return false;
        }

    } else {  
        
        float cumulativeRemovedWeight = 0.0f;
        for (int i = 0; i <= idx; i++) {
            cumulativeRemovedWeight += products[i].weight * products[i].quantity;
        }

        float targetWeight = outBoundStandard - cumulativeRemovedWeight;

        Serial.printf("출고 idx=%d, 기대 출고 후 무게=%.2f, 측정 무게=%.2f\n",
                      idx, targetWeight, measured);

        if (myFabs(measured - targetWeight) < 0.01f) {
            doc["code"] = "good";
            serializeJson(doc, buffer);
            client.publish(sendTopic, buffer);
            Serial.printf("제품 %d 출고 완료\n", idx);
            return true;
        } else {
            Serial.printf("제품 %d 출고 대기 중...\n", idx);
            return false;
        }
    }
}





