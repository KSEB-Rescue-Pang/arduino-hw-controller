#include <BLEDevice.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
#include "ledControl.h"
#include "bleScan.h"

BLEScan* pBLEScan;
const int scanTime = 1;

void processTasks() {
  if (!pBLEScan) {
    BLEDevice::init("");
    pBLEScan = BLEDevice::getScan(); 
    pBLEScan->setActiveScan(true); 
  }
  int lastRSSI;

  BLEScanResults foundDevices = *pBLEScan->start(scanTime, false);
  int maxRSSI = -100;
  for (int i = 0; i < foundDevices.getCount(); i++) {
    BLEAdvertisedDevice device = foundDevices.getDevice(i);
    int8_t rssi = device.getRSSI();
    if (device.haveName() && device.getName().indexOf("Quantum3") != -1) {
      if (rssi > maxRSSI) maxRSSI = rssi;
    }
  }
  lastRSSI = maxRSSI;
  manageHardWare(lastRSSI,ledType);
  pBLEScan->clearResults();
}