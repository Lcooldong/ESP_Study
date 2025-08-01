#include "MyBLECallback.h"
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




ServerCallbacks serverCallbacks;
CharacteristicCallbacks chrCallbacks;
DescriptorCallbacks dscCallbacks;

void ServerCallbacks::onConnect(NimBLEServer* pServer, NimBLEConnInfo& connInfo) {
    Serial.printf("Client address: %s\n", connInfo.getAddress().toString().c_str());

    // 연결 매개변수 요청
    pServer->updateConnParams(connInfo.getConnHandle(), 24, 48, 0, 180);
}

void ServerCallbacks::onDisconnect(NimBLEServer* pServer, NimBLEConnInfo& connInfo, int reason) {
    Serial.printf("Client disconnected - start advertising\n");
    NimBLEDevice::startAdvertising();
}

void ServerCallbacks::onMTUChange(uint16_t MTU, NimBLEConnInfo& connInfo) {
    Serial.printf("MTU updated: %u for connection ID: %u\n", MTU, connInfo.getConnHandle());
}

uint32_t ServerCallbacks::onPassKeyDisplay() {
    Serial.printf("Server Passkey Display\n");
    return 123456;
}

void ServerCallbacks::onConfirmPassKey(NimBLEConnInfo& connInfo, uint32_t pass_key) {
    Serial.printf("The passkey YES/NO number: %" PRIu32 "\n", pass_key);
    NimBLEDevice::injectConfirmPasskey(connInfo, true);
}

void ServerCallbacks::onAuthenticationComplete(NimBLEConnInfo& connInfo) {
    if (!connInfo.isEncrypted()) {
        NimBLEDevice::getServer()->disconnect(connInfo.getConnHandle());
        Serial.printf("Encrypt connection failed - disconnecting client\n");
        return;
    }

    Serial.printf("Secured connection to: %s\n", connInfo.getAddress().toString().c_str());
}

void CharacteristicCallbacks::onRead(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo) {
    Serial.printf("%s : onRead(), value: %s\n",
                  pCharacteristic->getUUID().toString().c_str(),
                  pCharacteristic->getValue().c_str());
}

void CharacteristicCallbacks::onWrite(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo) {
    Serial.printf("%s : onWrite(), value: %s\n",
                  pCharacteristic->getUUID().toString().c_str(),
                  pCharacteristic->getValue().c_str());
}

void CharacteristicCallbacks::onStatus(NimBLECharacteristic* pCharacteristic, int code) {
    Serial.printf("Notification/Indication return code: %d, %s\n", code, NimBLEUtils::returnCodeToString(code));
}

void CharacteristicCallbacks::onSubscribe(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo, uint16_t subValue) {
    std::string str = "Client ID: ";
    str += connInfo.getConnHandle();
    str += " Address: ";
    str += connInfo.getAddress().toString();
    
    switch (subValue) {
        case 0: str += " Unsubscribed to "; break;
        case 1: str += " Subscribed to notifications for "; break;
        case 2: str += " Subscribed to indications for "; break;
        case 3: str += " Subscribed to notifications and indications for "; break;
    }

    str += std::string(pCharacteristic->getUUID());
    Serial.printf("%s\n", str.c_str());
}

void DescriptorCallbacks::onWrite(NimBLEDescriptor* pDescriptor, NimBLEConnInfo& connInfo) {
    std::string dscVal = pDescriptor->getValue();
    Serial.printf("Descriptor written value: %s\n", dscVal.c_str());
}

void DescriptorCallbacks::onRead(NimBLEDescriptor* pDescriptor, NimBLEConnInfo& connInfo) {
    Serial.printf("%s Descriptor read\n", pDescriptor->getUUID().toString().c_str());
}