#include "main.h"

uint32_t currentMillis = 0;
uint32_t previousMillis[4] = {0,};
int counter = 0;


uint8_t ledValue = 0;
bool can_switch = false;
bool as5600_dir_switch = false;
int as5600_dir     = 0; 
int as5600_counter = 0;
float angle        = 0;
float lastAngle    = 0;
int32_t as5600_cumulativePosition = 0;
long prevCumulativePos            = 0;

int speed = 0;
int lastSpeed = 0;  
float rpm = 0;
float filteredRPM = 0;
const float alpha = 0.3; // 필터 민감도 (추천: 0.1 ~ 0.3) 1에 가까울 수록 원본과 비슷

bool ledDirection = true;

AS5600 as5600;
HardwareSerial Serail1(1);  // UART1
static RotaryEncoder rotaryEncoder(ROTARY_CLK_PIN, ROTARY_DT_PIN, RotaryEncoder::LatchMode::TWO03);; // Rotary encoder object
static NimBLEServer* pServer;
NimBLEHIDDevice *hid;

void serial_command();
void i2c_scan();

void setup() {
  Serial.begin(115200);
  Serial1.begin(115200, SERIAL_8N1, UART1_RX_PIN, UART1_TX_PIN); // UART1 with RX on GPIO 16 and TX on GPIO 17
  
  
  
  // ledcAttachPin(LED_PIN, LED_CHANNEL); // Attach LED pin to channel (not used in this example)
  // ledcSetup(LED_CHANNEL, LED_FREQUENCY, LED_RESOLUTION); // Setup LED channel (not used in this example)
  // ledcWrite(LED_CHANNEL, 0); // Set LED brightness to 50% (not used in this example)
  analogWrite(LED_PIN, 0); // Set initial LED brightness


  button_init();

  Wire.setPins(AS5600_SDA_PIN, AS5600_SCL_PIN);
  Wire.begin();
  // i2c_scan();
  Serial.println(__FILE__);
  Serial.print("AS5600_LIB_VERSION: ");
  Serial.println(AS5600_LIB_VERSION);

  if(as5600.begin(AS5600_DIR_PIN))
  {
    Serial.println("AS5600 not connected!");
  }
  else
  {
    Serial.println("AS5600 connected!");
    as5600.setDirection(AS5600_CLOCK_WISE);
  }

  ;  //  set direction pin.
  
  // as5600.setDirection(AS5600_COUNTERCLOCK_WISE);
  // int b = as5600.isConnected();
  // Serial.print("Connect: ");
  // Serial.println(b);

  can_init(CAN_RX_PIN, CAN_TX_PIN);
  Serial.printf("FLASH: %d MB\r\n", ESP.getFlashChipSize()/(1024*1024)); // Should print 16777216 for 16MB
  delay(100); 

  littleFS_init();
  
  tft_init();
  tft_printf(0, 25, "%s  ", menu_items[item_selected]);
  tft_printf(0, 60, "Click  ");

  NimBLEDevice::init("ESP32 Controller");
  NimBLEDevice::setPower(0);

  
  pServer = NimBLEDevice::createServer();
  pServer->setCallbacks(&serverCallbacks);

  hid = new NimBLEHIDDevice(pServer);
  hid->setManufacturer("DONG");
  hid->startServices();
  hid->setBatteryLevel(1);



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
  pAdvertising->setName("ESP32 Controller");
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
  currentMillis = millis();
  can_receive(); // Check for received CAN messages

  if (currentMillis - previousMillis[0] >= 1000) { // Update every second
    previousMillis[0] = currentMillis;
    counter++;
    Serial.printf("Counter: %d %d|\r\n", counter, digitalRead(ROTARY_SW_PIN)); // Print counter and button state to serial
    
    Serial1.printf("V%d\n", speed); // Send counter value over UART1
    // if(speed >= 20)
    // {
    //   speed = 0;
    // }
    // else
    // {
    //   speed += 2;
    // }
    
    // tft.fillScreen(TFT_BLACK); // Clear screen
    
    tft_printf(0, 5, "[%d]\r\n", counter);
    

    if(can_switch)
    {
      uint8_t test[8] = {0,};
      for (int i = 0; i < 8; i++)
      {
        test[i] = i + counter; // Fill test array with some data
      }
      can_send(0x123, test, sizeof(test)/sizeof(test[0]));
    }

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
  else if(currentMillis - previousMillis[1] >= 100 && as5600.isConnected())  // Update every 100ms
  {
    previousMillis[1] = currentMillis;
    long deltaPos = as5600_cumulativePosition - prevCumulativePos;

    float deltaRev = deltaPos / 4096.0;
    // rpm = deltaRev * 600.0;
    float newRpm = deltaRev * 600.0;
    filteredRPM = (1.0 - alpha) * filteredRPM + alpha * newRpm;

    if(deltaPos > 0)
    {
      as5600_dir = true;
    }
    else if(deltaPos < 0)
    {
      as5600_dir = -1;
    }
    else 
    {
      as5600_dir = 0;
    }
    prevCumulativePos = as5600_cumulativePosition;

    tft_printf(0, 125, "DIR:%3d",  as5600_dir);
    tft_printf(0, 150, "CP:%6d", as5600_cumulativePosition); // 누적 위치
    tft_printf(0, 180, "RPM:%5.1f", filteredRPM);
  }
  else if (currentMillis - previousMillis[2] >= 10) 
  { // Update every 10ms
    previousMillis[2] = currentMillis;
    button_tick();
  }
  else if (currentMillis - previousMillis[3] >= 5 && as5600.isConnected()) 
  { // Update every 5ms
    previousMillis[3] = currentMillis;
    
    angle = as5600.rawAngle() * AS5600_RAW_TO_DEGREES;
    as5600_cumulativePosition = as5600.getCumulativePosition();
    tft_printf(0, 100, "Angle: %3.0lf\r\n", angle);
  }
  else
  {
    serial_command(); // Handle serial commands 
  } 
  indicate_led();
  encoder_update();
  
}


void encoder_update()
{
  static int pos = 0;

  rotaryEncoder.tick();
  
  int newPos = (int)(rotaryEncoder.getPosition()/2); // Two steps per click
  if (pos != newPos) {          // changed in pos
    Serial.print("pos:");
    Serial.print(newPos);
    int dir = (int)(rotaryEncoder.getDirection());
    Serial.print(" dir:");
    Serial.println(dir);

    item_sel_previous = item_selected; // Store previous item index
    if(dir == 1){
      item_selected++;
      speed += 1;
      if(item_selected >= sizeof(menu_items)/sizeof(menu_items[0])) {
        item_selected = 0; // Wrap around to the first menu
      }
    }
    else if(dir == -1){
      item_selected--;
      speed -= 1;
      if(item_selected < 0) {
        item_selected = sizeof(menu_items)/sizeof(menu_items[0]) - 1; // Wrap around to the last menu
      }
    }
    pos = newPos;
    // lastSpeed = speed;
    Serial.printf("Item selected: %s\r\n", menu_items[item_selected]); // Print selected item index to serial
    
    tft_printf(0, 25, "%s  ", menu_items[item_selected]);
    // tft_printf(0, 210, "SPEED:%3d", speed);
    // Serial1.printf("V%d\n", speed); // Send counter value over UART1
  }

  if(speed != lastSpeed)
  {
    lastSpeed = speed;
    Serial.printf("Speed changed: %d\r\n", speed);
    tft_printf(0, 210, "SPEED:%3d", speed);
    Serial1.printf("V%d\n", speed); // Send counter value over UART1
  }
}




void serial_command()
{
  if(Serial.available()) {
    char c = Serial.read();
    switch (c)
    {
    case '1': 
      ledValue++;
      break;
    case '2': 
      ledValue--;
      break;
    case '3':
      can_switch = !can_switch;
      break;
    case '4':
      Serial.printf("-------------------------Restart-----------------------\r\n");
      ESP.restart();
      break;
    case '5':
      if(!as5600_dir_switch)
      {
        as5600.setDirection(AS5600_CLOCK_WISE);
        Serial.printf("AS5600 CW\r\n");
      }
      else
      { 
        as5600.setDirection(AS5600_COUNTERCLOCK_WISE);
        Serial.printf("AS5600 CCW\r\n");
      }
      as5600_dir_switch = !as5600_dir_switch;

      break;
    case '6':

      break;
    case '7':

      break;
    case '8':
      drawJpg(0, 0, "/panda240.jpg");
      break;
    case '9':
      drawJpg(0, 0, "/panda135_180.jpg");
      break;
    case '0':
      listDir(LittleFS, "/", 0);
      break;
    }
    // ledcWrite(LED_CHANNEL, ledValue); // Set LED brightness to 50% (not used in this example)
    // Serial.printf("LED Value: %d\r\n", ledValue); // Print LED value to serial
  }
}


void i2c_scan()
{
  static uint8_t i2c_counter = 0;

  for (byte i = 1; i < 120; i++)
  {
    Wire.beginTransmission (i);
    if (Wire.endTransmission () == 0)
      {
      Serial.print ("Found address: ");
      Serial.print (i, DEC);
      Serial.print (" (0x");
      Serial.print (i, HEX);
      Serial.println (")");
      i2c_counter++;
      delay (10);
      } // end of good response
  } // end of for loop
  Serial.println ("Done.");
  Serial.print ("Found ");
  Serial.print (i2c_counter, DEC);
  Serial.println (" device(s).");
}

void indicate_led()
{
  static uint32_t previousMillis_led = 0;
  
  const uint32_t interval = 50; // LED toggle interval in milliseconds

  uint32_t currentMillis_led = millis();

  if (currentMillis_led - previousMillis_led >= interval) {
    previousMillis_led = currentMillis_led;
    
    // ledValue += 1;
    if(ledValue >= 255)
    {
      // ledValue = 0;
      ledDirection = false;
    }
    else if(ledValue <= 0)
    {
      ledDirection = true;
    }
    ledValue = ledDirection ? ledValue + 5 : ledValue - 5;
    analogWrite(LED_PIN, ledValue);
    // Serial.printf("LED Value: %d %d\r\n", ledDirection ,ledValue); // Print LED value to serial
  }
}
