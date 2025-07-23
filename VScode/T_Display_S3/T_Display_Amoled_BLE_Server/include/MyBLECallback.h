#ifndef __MYCALLBACK_H__
#define __MYCALLBACK_H__

#include <NimBLEDevice.h>

/** Client 연결 콜백 클래스 */
class ClientCallbacks : public NimBLEClientCallbacks {
public:
    void onConnect(NimBLEClient* pClient) override;
    void onDisconnect(NimBLEClient* pClient, int reason) override;
};

/** 스캔 콜백 클래스 */
class ScanCallbacks : public NimBLEScanCallbacks {
public:
    void onResult(const NimBLEAdvertisedDevice* advertisedDevice) override;
    void onScanEnd(const NimBLEScanResults& results, int reason) override;
};

/** 외부에서 접근할 수 있도록 인스턴스 선언 */
extern ClientCallbacks clientCallbacks;
extern ScanCallbacks scanCallbacks;


/** Server 콜백 클래스 */
class ServerCallbacks : public NimBLEServerCallbacks {
public:
    void onConnect(NimBLEServer* pServer, NimBLEConnInfo& connInfo) override;
    void onDisconnect(NimBLEServer* pServer, NimBLEConnInfo& connInfo, int reason) override;
    void onMTUChange(uint16_t MTU, NimBLEConnInfo& connInfo) override;
    uint32_t onPassKeyDisplay() override;
    void onConfirmPassKey(NimBLEConnInfo& connInfo, uint32_t pass_key) override;
    void onAuthenticationComplete(NimBLEConnInfo& connInfo) override;
};

/** Characteristic 콜백 클래스 */
class CharacteristicCallbacks : public NimBLECharacteristicCallbacks {
public:
    void onRead(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo) override;
    void onWrite(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo) override;
    void onStatus(NimBLECharacteristic* pCharacteristic, int code) override;
    void onSubscribe(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo, uint16_t subValue) override;
};

/** Descriptor 콜백 클래스 */
class DescriptorCallbacks : public NimBLEDescriptorCallbacks {
public:
    void onWrite(NimBLEDescriptor* pDescriptor, NimBLEConnInfo& connInfo) override;
    void onRead(NimBLEDescriptor* pDescriptor, NimBLEConnInfo& connInfo) override;
};

/** 콜백 인스턴스 외부 선언 */
extern ServerCallbacks serverCallbacks;
extern CharacteristicCallbacks chrCallbacks;
extern DescriptorCallbacks dscCallbacks;


#endif  // __MYCALLBACK_H__