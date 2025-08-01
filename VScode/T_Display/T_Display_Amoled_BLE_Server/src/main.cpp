#include "main.h"

uint32_t currMillis = 0;
uint32_t lastMillis = 0;
int counter = 0;

OneButton button1(PIN_BUTTON_1, true, true);
OneButton button2(PIN_BUTTON_2, true, true);

static NimBLEServer* pServer;

void setup() {
 Serial.begin(115200);
  while (!Serial) {
    // Wait for Serial to be ready
  }
  delay(1000); // Give some time for Serial to initialize
  Serial.println("T-Display Amoled Sample");
  Serial.print("ESP32 SDK          : "); Serial.println(ESP.getSdkVersion());
  Serial.print("ESP32 CPU FREQ     : "); Serial.print(getCpuFrequencyMhz()); Serial.println("MHz");
  Serial.print("ESP32 APB FREQ     : "); Serial.print(getApbFrequency() / 1000000.0, 1); Serial.println("MHz");
  Serial.print("ESP32 FLASH SIZE   : "); Serial.print(ESP.getFlashChipSize() / (1024.0 * 1024), 2); Serial.println("MB");
  Serial.print("ESP32 PSRAM SIZE   : "); Serial.print(ESP.getPsramSize() / 1024.0, 2); Serial.println("KB");
  Serial.print("ESP32 RAM SIZE     : "); Serial.print(ESP.getHeapSize() / 1024.0, 2); Serial.println("KB");
  Serial.print("ESP32 FREE RAM     : "); Serial.print(ESP.getFreeHeap() / 1024.0, 2); Serial.println("KB");
  Serial.print("ESP32 MAX RAM ALLOC: "); Serial.print(ESP.getMaxAllocHeap() / 1024.0, 2); Serial.println("KB");
  Serial.print("ESP32 FREE PSRAM   : "); Serial.print(ESP.getFreePsram() / 1024.0, 2); Serial.println("KB");


  pinMode(PIN_LED, OUTPUT);
  digitalWrite(PIN_LED, HIGH);

  buttonInit();

  /** Initialize NimBLE and set the device name */
  
  NimBLEDevice::init("T-Display Amoled BLE Server");

  /**
   * Set the IO capabilities of the device, each option will trigger a different pairing method.
   *  BLE_HS_IO_DISPLAY_ONLY    - Passkey pairing
   *  BLE_HS_IO_DISPLAY_YESNO   - Numeric comparison pairing
   *  BLE_HS_IO_NO_INPUT_OUTPUT - DEFAULT setting - just works pairing
   */
  // NimBLEDevice::setSecurityIOCap(BLE_HS_IO_DISPLAY_ONLY); // use passkey
  // NimBLEDevice::setSecurityIOCap(BLE_HS_IO_DISPLAY_YESNO); //use numeric comparison

  /**
   *  2 different ways to set security - both calls achieve the same result.
   *  no bonding, no man in the middle protection, BLE secure connections.
   *
   *  These are the default values, only shown here for demonstration.
   */
  // NimBLEDevice::setSecurityAuth(false, false, true);
  // NimBLEDevice::setSecurityAuth(BLE_SM_PAIR_AUTHREQ_BOND | BLE_SM_PAIR_AUTHREQ_MITM | BLE_SM_PAIR_AUTHREQ_SC);

  pServer = NimBLEDevice::createServer();
  pServer->setCallbacks(&serverCallbacks);

  NimBLEService*        pDeadService = pServer->createService("DEAD");
  NimBLECharacteristic* pBeefCharacteristic =
      pDeadService->createCharacteristic("BEEF",
                                          NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE |
                                              /** Require a secure connection for read and write access */
                                              NIMBLE_PROPERTY::READ_ENC | // only allow reading if paired / encrypted
                                              NIMBLE_PROPERTY::WRITE_ENC  // only allow writing if paired / encrypted
      );

  pBeefCharacteristic->setValue("Burger");
  pBeefCharacteristic->setCallbacks(&chrCallbacks);

  /**
   *  2902 and 2904 descriptors are a special case, when createDescriptor is called with
   *  either of those uuid's it will create the associated class with the correct properties
   *  and sizes. However we must cast the returned reference to the correct type as the method
   *  only returns a pointer to the base NimBLEDescriptor class.
   */
  NimBLE2904* pBeef2904 = pBeefCharacteristic->create2904();
  pBeef2904->setFormat(NimBLE2904::FORMAT_UTF8);
  pBeef2904->setCallbacks(&dscCallbacks);

  NimBLEService*        pBaadService = pServer->createService("BAAD");
  NimBLECharacteristic* pFoodCharacteristic =
      pBaadService->createCharacteristic("F00D", NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::NOTIFY);

  pFoodCharacteristic->setValue("Fries");
  pFoodCharacteristic->setCallbacks(&chrCallbacks);

  /** Custom descriptor: Arguments are UUID, Properties, max length of the value in bytes */
  NimBLEDescriptor* pC01Ddsc =
      pFoodCharacteristic->createDescriptor("C01D",
                                            NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::WRITE_ENC,
                                            20);
  pC01Ddsc->setValue("Send it back!");
  pC01Ddsc->setCallbacks(&dscCallbacks);

  /** Start the services when finished creating all Characteristics and Descriptors */
  pDeadService->start();
  pBaadService->start();

  /** Create an advertising instance and add the services to the advertised data */
  NimBLEAdvertising* pAdvertising = NimBLEDevice::getAdvertising();
  pAdvertising->setName("NimBLE-Server");
  pAdvertising->addServiceUUID(pDeadService->getUUID());
  pAdvertising->addServiceUUID(pBaadService->getUUID());
  /**
   *  If your device is battery powered you may consider setting scan response
   *  to false as it will extend battery life at the expense of less data sent.
   */
  pAdvertising->enableScanResponse(true);
  pAdvertising->start();

  Serial.printf("Advertising Started\n");


}

void loop() {
  currMillis = millis();
    if (currMillis - lastMillis >= 1000) {
      lastMillis = currMillis;
      counter++;
      Serial.printf("[%d]Free heap: %d\n", counter, ESP.getFreeHeap());

      if (pServer->getConnectedCount()) {
      NimBLEService* pSvc = pServer->getServiceByUUID("BAAD");
      if (pSvc) {
          NimBLECharacteristic* pChr = pSvc->getCharacteristic("F00D");
          if (pChr) {
              pChr->notify();
          }
        }
      }

    }
    button1.tick();
    button2.tick();
    delay(2);
}




void buttonInit()
{
  button1.attachClick(click1);
  button1.attachDoubleClick(doubleclick1);
  button1.attachLongPressStart(longPressStart1);
  button1.attachLongPressStop(longPressStop1);
  button1.attachDuringLongPress(longPress1);

  button2.attachClick(click2);
  button2.attachDoubleClick(doubleclick2);
  button2.attachLongPressStart(longPressStart2);
  button2.attachLongPressStop(longPressStop2);
  button2.attachDuringLongPress(longPress2);
}

