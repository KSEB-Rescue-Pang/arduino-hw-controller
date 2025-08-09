#include "utils.h"

bool parseJson(const char* jsonBuffer, StaticJsonDocument<200>& doc) {
  DeserializationError error = deserializeJson(doc, jsonBuffer);
  if (error) {
    return false;
  }
  return true;
}