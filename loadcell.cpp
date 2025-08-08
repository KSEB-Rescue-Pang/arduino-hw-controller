#include "HX711.h"
#include "loadCell.h"
//#include <UnorderedMap.h>
#include "ledControl.h"
#include <ArduinoJson.h>

const int LOADCELL_DOUT_PIN = 4;
const int LOADCELL_SCK_PIN = 5;
int calibration_factor = 430000;
extern std::vector<string> myVector;
extern struct ProductInfo;
extern std::map<std::string, ProductInfo> productMap;
extern PubSubClient mqttClient(secureClient);
HX711 scale;
int index = 0;
void setLoadCell(){
  Serial.println("HX711 Demo");

  Serial.println("Initializing the scale");

  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);

  Serial.println("Before setting up the scale:");
  Serial.print("read: \t\t");
  Serial.println(scale.read());      // print a raw reading from the ADC

  Serial.print("read average: \t\t");
  Serial.println(scale.read_average(20));   // print the average of 20 readings from the ADC

  Serial.print("get value: \t\t");
  Serial.println(scale.get_value(5));   // print the average of 5 readings from the ADC minus the tare weight (not set yet)

  Serial.print("get units: \t\t");
  Serial.println(scale.get_units(5), 1);  // print the average of 5 readings from the ADC minus tare weight (not set) divided
            // by the SCALE parameter (not set yet)
            
  scale.set_scale(calibration_factor);
  scale.tare();               // reset the scale to 0

  Serial.println("After setting up the scale:");

  Serial.print("read: \t\t");
  Serial.println(scale.read());                 // print a raw reading from the ADC

  Serial.print("read average: \t\t");
  Serial.println(scale.read_average(20));       // print the average of 20 readings from the ADC

  Serial.print("get value: \t\t");
  Serial.println(scale.get_value(5));   // print the average of 5 readings from the ADC minus the tare weight, set with tare()

  Serial.print("get units: \t\t");
  Serial.println(scale.get_units(5), 1);        // print the average of 5 readings from the ADC minus tare weight, divided
            // by the SCALE parameter set with set_scale

  Serial.println("Readings:");
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

void calibrateWeight(String led){
  const String sendMessageTopic = "esp/ack";
  bool isInbound = (led == "IB") ? true : false;
  float cumulativeExpectedWeight = 0.0; // 기대 누적 변수
  float previousMeasuredWeight = 0.0;// 이전 단계의 실제 측정 무게 
  int totalQuantity = 0;
  if(!isInbound){
    Serial.println("출고 과정을 시작합니다. 로드셀에 올바르게 물건이 올라가져 있는 상태인지 검사가 이루어집니다.");
    for (const auto& productId_std : myVector) {
      auto it = productMap.find(productId_std);
      if (it != productMap.end()) {
        ProductInfo productInfo = it->second;
        cumulativeExpectedWeight += productInfo.weight * productInfo.quantity;
        totalQuantity += productInfo.quantity;
      }
    }
    previousMeasuredWeight = abs(roundf(scale.get_units(10) * 100) / 100.0);
    if(abs(previousMeasuredWeight - cumulativeExpectedWeight) > 1.0){
      Serial.println("입고를 제대로 하지 않아서 출고 불가입니다.");
      return;
    }
    Serial.println("출고 하기전 초기 상태 점검 완료!! 첫번째 품목의 물건들을 빼주세요");
  }else{
    Serial.println("입고 작업 준비 완료!! 첫번째 품목의 물건들을 올려주세요");
  }

  while(index < myVector.size()){
    std::string productId_std = myVector[index];
    auto it = productMap.find(productId_std);

    if(it == productMap.end()){return;}
    ProductInfo productInfo = it->second;
    float weight = productInfo.weight;
    int currentQuantity = productInfo.quantity;
    float currentProductExpectedWeight = weight * currentQuantity;
    float currentMeasuredWeight = abs(roundf(scale.get_units(10) * 100) / 100.0);
    if(previousMeasuredWeight == 0){ //입고를 처음하는 경우 말고는 이전 측정치를 가지고 있어야 비교 가능
      previousMeasuredWeight = currentMeasuredWeight;
    }

    StaticJsonDocument<256> doc;
    doc["product_id"] = productId_std;
    doc["worker_id"] = workerId;
    doc["location_id"] = "z01-s01";

    bool isMatched = false;
    float weightChange = 0.0; // 무게 변화량 (입고 시 증가량, 출고 시 감소량)
    float expectedTotalWeight = 0.0;
    if(isInbound){
      expectedTotalWeight = cumulativeExpectedWeight + currentProductExpectedWeight;
      weightChange = currentMeasuredWeight - previousMeasuredWeight;
      bool isTotalWeightMatched = (abs(currentMeasuredWeight - expectedTotalWeight) < 0.5);//총 누적합 == 총 기대치 누적합
      bool isDeltaWeightMatched = (abs(weightChange - currentProductExpectedWeight) < 0.5)//(총 누적합 - 이전까지의 누적합(실제변화값)) == (총 기대치 누적합 - 이전까지의 기대치 누적합)
      bool isWeightPerItem = (abs((weightChange / currentQuantity) - (currentProductExpectedWeight / currentQuantity)) < 0.5);//증가량 / 증가한 갯수 == 예상 증가량 / 기대 증가 갯수
      if(isTotalWeightMatched && isDeltaWeightMatched && isWeightPerItem){
        isMatched = true;
        cumulativeExpectedWeight += currentProductExpectedWeight;
      }
    }else{
      expectedTotalWeight = cumulativeExpectedWeight - currentProductExpectedWeight;
      weightChange = previousMeasuredWeight - currentMeasuredWeight;
      int decreasedQuantity = totalQuantity - currentQuantity;

      bool isTotalWeighMatched =  (abs(currentMeasuredWeight - expectedTotalWeight) < 0.5);//출고 1번
      bool isDeltaWeightMatched = (abs(weightChange - currentProductExpectedWeight) < 0.5);//출고 2번
      bool isWeightPerItem = (abs((weightChange / decreasedQuantity) - (currentProductExpectedWeight / decreasedQuantity)) < 0.5); //출고 3번
      if (isTotalWeightMatched && isDeltaWeightMatched && isWeightPerItem) {
        isMatched = true;
        cumulativeExpectedWeight -= currentProductExpectedWeight; // 기대치 감소
      }
    }
    if(isMatched){
      Serial.println("성공적으로 수행하였습니다!");
      doc["code"] = "good";
      String jsonBuffer;
      serializeJson(doc,jsonBuffer);
      mqttClient.publish(sendMessageTopic.c_str(),jsonBuffer.c_str());
      previousMeasuredWeight = currentMeasuredWeight;
      index++;
      if(index < myVector.size()){
        if(isInbound){
          Serial.println("다음 물품을 올려주세요");
        }else{
          Serial.println("다음 물품을 내려주세요");
        }
      }
    }else{
      Serial.println("작업을 잘못 수행했습니다. 다시 해주세요");
      doc["code"] = "bad";
      String jsonBuffer;
      serializeJson(doc,jsonBuffer);
      mqttClient.publish(sendMessageTopic.c_str(),jsonBuffer.c_str());
      delay(2000);
    }
  }
  turnOffLED(led);
  Serial.println("모든 작업 완료! LED를 끕니다.");
}