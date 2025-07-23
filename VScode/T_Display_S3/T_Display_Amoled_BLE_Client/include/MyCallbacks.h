#ifndef MY_CALLBACKS_H
#define MY_CALLBACKS_H

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

#endif