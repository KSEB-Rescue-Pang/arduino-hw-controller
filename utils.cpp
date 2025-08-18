#include "utils.h"

bool parseJson(const char* jsonBuffer, StaticJsonDocument<512>& doc) {
  DeserializationError error = deserializeJson(doc, jsonBuffer);
  if (error) {
    Serial.print("JSON 파싱 실패: ");
    Serial.println(error.c_str());
    return false;
  }
  return true;
}