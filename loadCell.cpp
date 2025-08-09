#include "HX711.h"
#include "loadCell.h"
//#include <UnorderedMap.h>
#include "ledControl.h"
#include "utils.h"

const int LOADCELL_DOUT_PIN = 4;
const int LOADCELL_SCK_PIN = 5;
int calibration_factor = 430000;
HX711 scale;
void setLoadCell(){
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);

  #ifdef DEBUG
    Serial.println(F("Initializing scale..."));
  #endif
  // 한번만 주요 값 출력
  //Serial.print(F("Raw reading: "));
 // Serial.println(F(scale.read()));

  //Serial.print(F("Average reading (20 samples): "));
  //Serial.println(F(scale.read_average(20)));

  scale.set_scale(calibration_factor);
  scale.tare();  // reset scale to zero

  //Serial.println("Scale calibrated and tared.");

  // 한두 개 값만 출력해서 상태 확인
  //Serial.print(F("Current units (5 samples): "));
  //Serial.println(scale.get_units(5), 1);
  #ifdef DEBUG
    Serial.println(F("Setup complete."));
  #endif
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
  const char* sendMessageTopic = "esp/ack";
  bool isInbound = (led == "IB") ? true : false;
  float cumulativeExpectedWeight = 0.0f; // 기대 누적 변수
  float previousMeasuredWeight = 0.0f;// 이전 단계의 실제 측정 무게 
  int totalQuantity = 0;
  if(!isInbound){
    #ifdef DEBUG
      Serial.println(F("출고 과정을 시작합니다."));
    #endif
    for (const auto& productId_std : myVector) {
      auto it = productMap.find(productId_std);
      if (it != productMap.end()) {
        ProductInfo productInfo = it->second;
        cumulativeExpectedWeight += productInfo.weight * productInfo.quantity;
        totalQuantity += productInfo.quantity;
      }
    }
    previousMeasuredWeight = fabsf(roundf(scale.get_units(10) * 100.0f) / 100.0f);
    if(fabsf(previousMeasuredWeight - cumulativeExpectedWeight) > 1.0f){
      Serial.println(F("입고를 제대로 하지 않아서 출고 불가입니다."));
      return;
    }
    #ifdef DEBUG
      Serial.println(F("출고 하기전 초기 상태 점검 완료!! 첫번째 품목의 물건들을 빼주세요"));
    #endif
  }else{
    #ifdef DEBUG
      Serial.println(F("입고 작업 준비 완료!! 첫번째 품목의 물건들을 올려주세요"));
    #endif;
  }

  while(idx < myVector.size()){
    const char* productId_std = myVector[idx];
    auto it = productMap.find(productId_std);

    if(it == productMap.end()){return;}
    ProductInfo productInfo = it->second;
    float weight = productInfo.weight;
    int currentQuantity = productInfo.quantity;
    float currentProductExpectedWeight = weight * currentQuantity;
    float currentMeasuredWeight = fabsf(roundf(scale.get_units(10) * 100.0f) / 100.0f);
    if(previousMeasuredWeight == 0){ //입고를 처음하는 경우 말고는 이전 측정치를 가지고 있어야 비교 가능
      previousMeasuredWeight = currentMeasuredWeight;
    }

    StaticJsonDocument<256> doc;
    doc["product_id"] = productId_std;
    doc["worker_id"] = workerId;
    doc["location_id"] = "A01-R01";

    bool isMatched = false;
    float weightChange = 0.0f; // 무게 변화량 (입고 시 증가량, 출고 시 감소량)
    float expectedTotalWeight = 0.0f;
    if(isInbound){
      expectedTotalWeight = cumulativeExpectedWeight + currentProductExpectedWeight;
      weightChange = currentMeasuredWeight - previousMeasuredWeight;
      bool isTotalWeightMatched = (fabsf(currentMeasuredWeight - expectedTotalWeight) < 0.5f);//총 누적합 == 총 기대치 누적합
      bool isDeltaWeightMatched = (fabsf(weightChange - currentProductExpectedWeight) < 0.5f);//(총 누적합 - 이전까지의 누적합(실제변화값)) == (총 기대치 누적합 - 이전까지의 기대치 누적합)
      bool isWeightPerItem = (fabsf((weightChange / currentQuantity) - (currentProductExpectedWeight / currentQuantity)) < 0.5f);//증가량 / 증가한 갯수 == 예상 증가량 / 기대 증가 갯수
      if(isTotalWeightMatched && isDeltaWeightMatched && isWeightPerItem){
        isMatched = true;
        cumulativeExpectedWeight += currentProductExpectedWeight;
      }
    }else{
      expectedTotalWeight = cumulativeExpectedWeight - currentProductExpectedWeight;
      weightChange = previousMeasuredWeight - currentMeasuredWeight;
      int decreasedQuantity = totalQuantity - currentQuantity;

      bool isTotalWeightMatched =  (fabsf(currentMeasuredWeight - expectedTotalWeight) < 0.5f);//출고 1번
      bool isDeltaWeightMatched = (fabsf(weightChange - currentProductExpectedWeight) < 0.5f);//출고 2번
      bool isWeightPerItem = (fabsf((weightChange / decreasedQuantity) - (currentProductExpectedWeight / decreasedQuantity)) < 0.5f); //출고 3번
      if (isTotalWeightMatched && isDeltaWeightMatched && isWeightPerItem) {
        isMatched = true;
        cumulativeExpectedWeight -= currentProductExpectedWeight; // 기대치 감소
      }
    }
    char jsonBuffer[256];
    if(isMatched){
      #ifdef DEBUG
        Serial.println(F("성공적으로 수행하였습니다!"));
      #endif
      doc["code"] = "good";
      serializeJson(doc,jsonBuffer,sizeof(jsonBuffer));
      mqttClient.publish(sendMessageTopic,jsonBuffer);
      previousMeasuredWeight = currentMeasuredWeight;
      idx++;
      if(idx < myVector.size()){
        if(isInbound){
          #ifdef DEBUG
            Serial.println(F("다음 물품을 올려주세요"));
          #endif;
        }else{
          #ifdef DEBUG
            Serial.println(F("다음 물품을 내려주세요"));
          #endif
        }
      }
    }else{
      #ifdef DEBUG
        Serial.println(F("작업을 잘못 수행했습니다. 다시 해주세요"));
      #endif
      doc["code"] = "bad";
      serializeJson(doc,jsonBuffer,sizeof(jsonBuffer));
      mqttClient.publish(sendMessageTopic,jsonBuffer);
      delay(2000);
    }
  }
  turnOffLED(led);
  #ifdef DEBUG
    Serial.println(F("모든 작업 완료! LED를 끕니다."));
  #endif
}