#include "MyCallbacks.h"
#include <Arduino.h>

static constexpr uint32_t scanTimeMs = 5 * 1000;

ClientCallbacks clientCallbacks;
ScanCallbacks scanCallbacks;

void ClientCallbacks::onConnect(NimBLEClient* pClient) {
    Serial.printf("Connected to: %s\n", pClient->getPeerAddress().toString().c_str());
}

void ClientCallbacks::onDisconnect(NimBLEClient* pClient, int reason) {
    Serial.printf("%s Disconnected, reason = %d - Starting scan\n",
                  pClient->getPeerAddress().toString().c_str(), reason);
    NimBLEDevice::getScan()->start(scanTimeMs);
}

void ScanCallbacks::onResult(const NimBLEAdvertisedDevice* advertisedDevice) {
    Serial.printf("Advertised Device found: %s\n", advertisedDevice->toString().c_str());

    if (advertisedDevice->haveName() && advertisedDevice->getName() == "NimBLE-Server") {
        Serial.printf("Found Our Device\n");

        NimBLEClient* pClient = NimBLEDevice::getDisconnectedClient();
        if (!pClient) {
            pClient = NimBLEDevice::createClient(advertisedDevice->getAddress());
            if (!pClient) {
                Serial.printf("Failed to create client\n");
                return;
            }
        }

        pClient->setClientCallbacks(&clientCallbacks, false);

        if (!pClient->connect(true, true, false)) { // deleteAttributes, asyncConnect, noMTUExchange
            NimBLEDevice::deleteClient(pClient);
            Serial.printf("Failed to connect\n");
            return;
        }
    }
}

void ScanCallbacks::onScanEnd(const NimBLEScanResults& results, int reason) {
    Serial.printf("Scan Ended\n");
    NimBLEDevice::getScan()->start(scanTimeMs);
}