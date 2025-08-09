#include "HX711.h"
#include "loadCell.h"
#include "ledControl.h"
#include "utils.h"

const int LOADCELL_DOUT_PIN = 4;
const int LOADCELL_SCK_PIN = 5;
int calibration_factor = 430000;
HX711 scale;
void setLoadCell(){
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  scale.set_scale(calibration_factor);
  scale.tare();
}


  //입고일 때
  //총 누적합 == 총 기대치 누적합, (총 누적합 - 이전까지의 누적합) == (총 기대치 누적합 - 이전까지의 기대치 누적합)
  //증가량 / 증가한 갯수 == 예상 증가량 / 기대 증가 갯수
  
  //출고일 때
  //로드셀에 전체 물건이 올라가져 있는 상태에서 물건을 빼야함
  //1. 물품을 빼고 난 후 로드셀에 측정되어지는 값 == 물품을 빼기전 로드셀에서 측정된 무게 - (전달 받은 물건의 수량 * 전달 물건의 무게) 
  //2. (물품을 빼기전 로드셀에서 측정된 무게 - 물품을 빼고 난 후 로드셀에 측정되어지는 값) == (전달 받은 물건의 수량 * 전달 물건의 무게)
  //3. 실제 감소량 / 감소한 갯수 == 예상 감소량 / 감소한 갯수 
  // 1,2,3 번을 물품을 다 뺄때까지 반복

void calibrateWeight(const char* led){
  int idx = 0;
  const char* sendTopic = "esp/ack";
  bool isInbound = strcmp(led, "IB") == 0;
  float cumulativeExpectedWeight = 0.0f;
  float previousMeasuredWeight = 0.0f;
  int totalQuantity = 0;
  if (!isInbound) {
    for (int i = 0; i < productCount; i++) {
      cumulativeExpectedWeight += products[i].weight * products[i].quantity;
      totalQuantity += products[i].quantity;
    }
    previousMeasuredWeight = fabsf(roundf(scale.get_units(10) * 100.0f) / 100.0f);
    if (fabsf(previousMeasuredWeight - cumulativeExpectedWeight) > 1.0f) {
      return;
    }
  }

  while (idx < productCount) {
    const char* productId_std = productIds[idx];
    if (idx >= productCount) return;
    
    ProductInfo productInfo = products[idx];
    float weight = productInfo.weight;
    int currentQuantity = productInfo.quantity;
    float currentProductExpectedWeight = weight * currentQuantity;
    float currentMeasuredWeight = fabsf(roundf(scale.get_units(10) * 100.0f) / 100.0f);
    if (previousMeasuredWeight == 0) {
      previousMeasuredWeight = currentMeasuredWeight;
    }

    StaticJsonDocument<128> doc;
    doc["product_id"] = productId_std;
    doc["worker_id"] = workerId;
    doc["location_id"] = "A01-R01";

    bool isMatched = false;
    float weightChange = 0.0f;
    float expectedTotalWeight = 0.0f;
    if (isInbound) {
      expectedTotalWeight = cumulativeExpectedWeight + currentProductExpectedWeight;
      weightChange = currentMeasuredWeight - previousMeasuredWeight;
      bool isTotalWeightMatched = (fabsf(currentMeasuredWeight - expectedTotalWeight) < 0.5f);
      bool isDeltaWeightMatched = (fabsf(weightChange - currentProductExpectedWeight) < 0.5f);
      bool isWeightPerItem = (fabsf((weightChange / currentQuantity) - (currentProductExpectedWeight / currentQuantity)) < 0.5f);
      if (isTotalWeightMatched && isDeltaWeightMatched && isWeightPerItem) {
        isMatched = true;
        cumulativeExpectedWeight += currentProductExpectedWeight;
      }
    } else {
      expectedTotalWeight = cumulativeExpectedWeight - currentProductExpectedWeight;
      weightChange = previousMeasuredWeight - currentMeasuredWeight;
      int decreasedQuantity = totalQuantity - currentQuantity;

      bool isTotalWeightMatched = (fabsf(currentMeasuredWeight - expectedTotalWeight) < 0.5f);
      bool isDeltaWeightMatched = (fabsf(weightChange - currentProductExpectedWeight) < 0.5f);
      bool isWeightPerItem = (fabsf((weightChange / decreasedQuantity) - (currentProductExpectedWeight / decreasedQuantity)) < 0.5f);
      if (isTotalWeightMatched && isDeltaWeightMatched && isWeightPerItem) {
        isMatched = true;
        cumulativeExpectedWeight -= currentProductExpectedWeight;
      }
    }
    char jsonBuffer[128];
    if (isMatched) {
      doc["code"] = "good";
      serializeJson(doc, jsonBuffer, sizeof(jsonBuffer));
      mqttClient.publish(sendTopic, jsonBuffer);
      previousMeasuredWeight = currentMeasuredWeight;
      idx++;
    } else {
      doc["code"] = "bad";
      serializeJson(doc, jsonBuffer, sizeof(jsonBuffer));
      mqttClient.publish(sendTopic, jsonBuffer);
      delay(2000);
    }
  }
  turnOffLED(led);
}