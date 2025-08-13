#include "bleScan.h"
#include "ledControl.h"

const int scanTime = 1; // seconds

void processTasks() {
    static NimBLEScan* pBLEScan = nullptr;

    if (pBLEScan == nullptr) {
        NimBLEDevice::init("");
        pBLEScan = NimBLEDevice::getScan();
        pBLEScan->setActiveScan(true);
    }

    int lastRSSI = -100;

    // NimBLE에서는 start()가 bool 반환 → 결과는 getResults()로 가져옴
    pBLEScan->start(scanTime, false);
    NimBLEScanResults foundDevices = pBLEScan->getResults();

    int maxRSSI = -100;
    for (int i = 0; i < foundDevices.getCount(); i++) {
        const NimBLEAdvertisedDevice* device = foundDevices.getDevice(i); // 포인터 반환
        int8_t rssi = device->getRSSI();

        if (device->haveName() && device->getName().find("Quantum3") != std::string::npos) {
            if (rssi > maxRSSI) maxRSSI = rssi;
        }
    }

    lastRSSI = maxRSSI;
    manageHardWare(lastRSSI, ledType);
    pBLEScan->clearResults();
}
