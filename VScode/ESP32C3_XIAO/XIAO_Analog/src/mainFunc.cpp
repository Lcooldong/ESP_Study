#include "mainFunc.h"





PACKET serialData = {0, };
PACKET incomingReadings;
esp_now_peer_info_t slave;
uint8_t data = 0;
String compareApName = "AP_";
String slaveApName = "";
int sendCompleteFlag = 0; 
int32_t channel = 1;


uint64_t hallLastTime = 0;
unsigned long ota_progress_millis = 0;


AsyncWebServer server(80);
DNSServer dns;
MyLittleFS* mySPIFFS = new MyLittleFS();
MyNeopixel* myNeopixel = new MyNeopixel();

U8X8_SSD1306_128X64_NONAME_SW_I2C u8x8(/* clock=*/ SCL_PIN, /* data=*/ SDA_PIN, /* reset=*/ U8X8_PIN_NONE);
//U8X8_SSD1306_128X64_NONAME_HW_I2C u8x8(/* reset=*/ U8X8_PIN_NONE); 
//extern U8X8_SSD1306_128X64_NONAME_HW_I2C u8x8;

extern IPAddress ip;
extern IPAddress gateway;
extern IPAddress subnet;
extern const char* apName;

void initOLED(const uint8_t* _font)
{
  u8x8.begin();
  u8x8.setPowerSave(0);  
  u8x8.setFont(_font);
  u8x8.setInverseFont(0);
  u8x8.clearDisplay();
}

int32_t getWiFiChannel(char *ssid)
{
  if (int32_t n = WiFi.scanNetworks()) {
    for (uint8_t i=0; i<n; i++) {
      if (!strcmp(ssid, WiFi.SSID(i).c_str())) {
        return WiFi.channel(i);
      }
    }
  }
  return 0;
}

void resetBoardValue()
{
  if(hallLastTime > RESET_DEADLINE)
  {
    ESP.restart();
  }

}

void setUpWiFi()
{
  AsyncWiFiManager wifiManager(&server,&dns);

  mySPIFFS->InitLitteFS();

  if(mySPIFFS->loadConfig(LittleFS))
  {
//    Serial.println(mySPIFFS->ssid);
//    Serial.println(mySPIFFS->pass);
#ifdef FIXED_IP
    // IPAddress ip (192, 168, 1, 48);
    // IPAddress gateway (192, 168, 1, 1);
    // IPAddress subnet(255, 255, 255, 0);
    WiFi.config(ip, gateway, subnet);
#endif

    WiFi.mode(WIFI_AP_STA);

    channel = getWiFiChannel(mySPIFFS->ssid);

    bool result = WiFi.softAP(apName, NULL, channel, 0);
    if (!result) 
    {
      Serial.println("AP Config failed.");
    } 
    else 
    {
      Serial.println("AP Config Success. Broadcasting with AP: " + String(apName));
    }

    
    esp_wifi_set_promiscuous(true);
    esp_wifi_set_channel(channel, WIFI_SECOND_CHAN_NONE);
    esp_wifi_set_promiscuous(false);
    WiFi.begin(mySPIFFS->ssid, mySPIFFS->pass);
    Serial.println("Connect to Flash Memory!");

    
  }
  else
  {
    Serial.println("Saved file doesn't exist => Move to WiFiManager");
  }

  unsigned long connectionLastTime = millis();
  while (WiFi.status() != WL_CONNECTED) 
  {
      delay(500);
      Serial.print(".");

      if (millis() - connectionLastTime > WIFI_CONNECTION_INTERVAL)
      {
        digitalWrite(BUILTIN_LED, HIGH);
        String WiFiManagerName = "WiFiManager_" + (String)apName;
        myNeopixel->pickOneLED(0, myNeopixel->strip->Color(0, 0, 255), 50, 1);
        u8x8.setCursor(0, 1);
        u8x8.printf("WiFiManager\r\n");
        u8x8.setCursor(0, 2);
        u8x8.printf("%s\r\n", apName);
        Serial.println("Start WiFiManager => 192.168.4.1");
        wifiManager.resetSettings();
        bool wmRes = wifiManager.autoConnect(WiFiManagerName.c_str());
        if(!wmRes)
        {
          Serial.println("Failed to connect");
        }
        else
        {
          Serial.printf("\nSuccess\n");

          mySPIFFS->saveConfig(LittleFS, wifiManager.getConfiguredSTASSID(), wifiManager.getConfiguredSTAPassword());
          delay(100);
          ESP.restart();

          break;  // Temp
        }
      }

  }
  Serial.print("\nIP : ");
  Serial.print(WiFi.localIP());
  Serial.print("\nGateway : ");
  Serial.print(WiFi.gatewayIP());
  Serial.print("\nSubnet  : ");
  Serial.print(WiFi.subnetMask());
  Serial.println();
  Serial.print(" Channel : ");
  Serial.println(WiFi.channel());

// OLED
  u8x8.clearDisplay();

  u8x8.setCursor(0, 0);
  u8x8.print(WiFi.localIP());
  u8x8.setCursor(0, 1);
  u8x8.print(WiFi.gatewayIP());
  u8x8.setCursor(0, 2);
  u8x8.print(WiFi.subnetMask());
  u8x8.setCursor(0, 3);
  u8x8.printf("Channel : %d\r\n", channel);
  digitalWrite(BUILTIN_LED, LOW);
}

void setupESPNOW()
{
  if (esp_now_init() != ESP_OK)
  {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  else
  {
    Serial.println("Init ESP-NOW");
  }
  esp_now_register_send_cb(OnDataSent);
  esp_now_register_recv_cb(OnDataRecv);



}

void onOTAStart() {
  // Log when OTA has started
  Serial.println("OTA update started!");
  // <Add your own code here>
}

void onOTAProgress(size_t current, size_t final) {
  // Log every 1 second
  if (millis() - ota_progress_millis > 1000) {
    ota_progress_millis = millis();
    Serial.printf("OTA Progress Current: %u bytes, Final: %u bytes\n", current, final);
  }
}

void onOTAEnd(bool success) {
  // Log when OTA has finished
  if (success) {
    Serial.println("OTA update finished successfully!");
  } else {
    Serial.println("There was an error during OTA update!");
  }
  // <Add your own code here>
}




void recvMsg(uint8_t *data, size_t len){
  WebSerial.println("Received Data...");
  String d = "";
  for(int i=0; i < len; i++){
    d += char(data[i]);
  }
  WebSerial.println(d); // 받은 데이터 출력
  // if (d == "ON"){
  // digitalWrite(LED, HIGH);
  // }
  // if (d=="OFF"){
  //   digitalWrite(LED, LOW);
  // }
}


void arduinoOTAProgress()
{
  ArduinoOTA
    .onStart([]() {
      String type;
      if (ArduinoOTA.getCommand() == U_FLASH)
        type = "sketch";
      else // U_SPIFFS
        type = "filesystem";

      // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
      Serial.println("Start updating " + type);
    })
    .onEnd([]() {
      Serial.println("\nEnd");
    })
    .onProgress([](unsigned int progress, unsigned int total) {
      Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    })
    .onError([](ota_error_t error) {
      Serial.printf("Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
      else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
      else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
      else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
      else if (error == OTA_END_ERROR) Serial.println("End Failed");
    });

  ArduinoOTA.begin();
}

void setupOTA()
{
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/plain", "For Update -> /update \r\nFor Debug  -> /webserial");
  });

  ElegantOTA.begin(&server);    // Start ElegantOTA
  // ElegantOTA callbacks
  ElegantOTA.onStart(onOTAStart);
  ElegantOTA.onProgress(onOTAProgress);
  ElegantOTA.onEnd(onOTAEnd);

  WebSerial.begin(&server);
  WebSerial.msgCallback(recvMsg);

  arduinoOTAProgress();

  server.begin();
  Serial.println("HTTP server started");

}

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  char macStr[18];
  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
           mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  Serial.print("Last Packet Sent to: "); Serial.println(macStr);
  Serial.print("Last Packet Send Status: "); Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

void OnDataRecv(const uint8_t *mac_addr, const uint8_t *data, int data_len) {
  char macStr[18];
  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
           mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  Serial.print("Last Packet Recv from: "); Serial.println(macStr);
  Serial.print("Last Packet Recv Data: "); Serial.println(*data);
  Serial.println("");

  memcpy(&incomingReadings, data, sizeof(incomingReadings));
  
  uint8_t target_board_id = incomingReadings.device_id;
  uint8_t R = incomingReadings.RED;
  uint8_t G = incomingReadings.GREEN;
  uint8_t B = incomingReadings.BLUE;
  uint8_t _brightness = incomingReadings.brightness;
  uint8_t waitORtimes = incomingReadings.wait;
  myNeopixel->strip->setBrightness(_brightness);
  
  
  
  // // target_board_led >> 4
  // if(device_id == (target_board_led / 16))
  // {
  //   switch(incomingReadings.style)
  //   {
  //     case oneColor:
  //       pickOneLED(target_board_led%16, strip.Color(R, G, B), _brightness, waitORtimes);
  //       break;
        
  //     case CHASE:
  //       theaterChase(strip.Color(R, G, B), waitORtimes);
  //       resetNeopixel();
  //       break;
        
  //     case RAINBOW:
  //       rainbow(waitORtimes);
  //       resetNeopixel();
  //       break;
        
  //     case COLORWIPE:
  //       colorWipe(strip.Color(R, G, B), waitORtimes * 10);
  //       break;
  //     case CHASE_RAINBOW:
  //       theaterChaseRainbow(waitORtimes);
  //       resetNeopixel();
  //       break;

  //     default:
  //       resetNeopixel();
  //       break;
  //   }
  // } 

  
}

void setupESPNOWPair()
{
  while(true)
  {
      // In the loop we scan for slave
      ScanForSlave();
      // If Slave is found, it would be populate in `slave` variable
      // We will check if `slave` is defined and then we proceed further
      if (slave.channel == channel) { // check if slave channel is defined
        // `slave` is defined
        // Add slave as peer if it has not been added already
        if(slaveApName.startsWith(compareApName) == 1) // check one more ScanData
        {
          bool isPaired = manageSlave();
          if (isPaired) {
            // pair success or already paired
            // Send data to device
            sendData();
          } else {
            // slave pair failed
            Serial.println("Slave pair failed!");
          }
 
          if(sendCompleteFlag == 1)
          {
            Serial.println("------Remote LED connected-------");
            break;
          }
        }        
      }
      else {
        // No slave found to process
      }
    
      // wait for 3seconds to run the logic again
      delay(1000);
  }
}


// Scan for slaves in AP mode
void ScanForSlave() {
  int8_t scanResults = WiFi.scanNetworks();
  // reset on each scan
  bool slaveFound = 0;
  memset(&slave, 0, sizeof(slave));

  Serial.println("");
  if (scanResults == 0) {
    Serial.println("No WiFi devices in AP Mode found");
  } else {
    Serial.print("Found "); Serial.print(scanResults); Serial.println(" devices ");
    for (int i = 0; i < scanResults; ++i) {
      // Print SSID and RSSI for each device found
      slaveApName = WiFi.SSID(i);
      int32_t RSSI = WiFi.RSSI(i);
      String BSSIDstr = WiFi.BSSIDstr(i);
      
      if (PRINTSCANRESULTS) {
        Serial.print(i + 1);
        Serial.print(": ");
        Serial.print(slaveApName);
        Serial.print(" (");
        Serial.print(RSSI);
        Serial.print(")");
        Serial.println("");
      }
      delay(10);
      // Check if the current device starts with `Remote`
      if (slaveApName.indexOf(compareApName) == 0) {
        // SSID of interest
        Serial.println("Found a Slave.");
        Serial.print(i + 1); Serial.print(": "); Serial.print(slaveApName); Serial.print(" ["); Serial.print(BSSIDstr); Serial.print("]"); Serial.print(" ("); Serial.print(RSSI); Serial.print(")"); Serial.println("");
        // Get BSSID => Mac Address of the Slave
        int mac[6];
        if ( 6 == sscanf(BSSIDstr.c_str(), "%x:%x:%x:%x:%x:%x",  &mac[0], &mac[1], &mac[2], &mac[3], &mac[4], &mac[5] ) ) {
          for (int ii = 0; ii < 6; ++ii ) {
            slave.peer_addr[ii] = (uint8_t) mac[ii];
          }
        }

        slave.channel = channel; // pick a channel
        slave.encrypt = 0; // no encryption

        slaveFound = 1;
        // we are planning to have only one slave in this example;
        // Hence, break after we find one, to be a bit efficient
        
      
        break;
        
      }
    }
  }

  if (slaveFound) {
    Serial.println("Slave Found, processing..");    
  } else {
    Serial.println("Slave Not Found, trying again.");
  }
  
  

  // clean up ram
  WiFi.scanDelete();
}

// Check if the slave is already paired with the master.
// If not, pair the slave with master
bool manageSlave() {
  if (slave.channel == channel) {
    if (DELETEBEFOREPAIR) {
      deletePeer();
    }

    Serial.print("Slave Status: ");
    // check if the peer exists
    bool exists = esp_now_is_peer_exist(slave.peer_addr);
    if ( exists) {
      // Slave already paired.
      Serial.println("Already Paired");
      return true;
    } else {
      // Slave not paired, attempt pair
      esp_err_t addStatus = esp_now_add_peer(&slave);
      if (addStatus == ESP_OK) {
        // Pair success
        Serial.println("Pair success");
        return true;
      } else if (addStatus == ESP_ERR_ESPNOW_NOT_INIT) {
        // How did we get so far!!
        Serial.println("ESPNOW Not Init");
        return false;
      } else if (addStatus == ESP_ERR_ESPNOW_ARG) {
        Serial.println("Invalid Argument");
        return false;
      } else if (addStatus == ESP_ERR_ESPNOW_FULL) {
        Serial.println("Peer list full");
        return false;
      } else if (addStatus == ESP_ERR_ESPNOW_NO_MEM) {
        Serial.println("Out of memory");
        return false;
      } else if (addStatus == ESP_ERR_ESPNOW_EXIST) {
        Serial.println("Peer Exists");
        return true;
      } else {
        Serial.println("Not sure what happened");
        return false;
      }
    }
  } else {
    // No slave found to process
    Serial.println("No Slave found to process");
    return false;
  }
}

void deletePeer() {
  esp_err_t delStatus = esp_now_del_peer(slave.peer_addr);
  Serial.print("Slave Delete Status: ");
  if (delStatus == ESP_OK) {
    // Delete success
    Serial.println("Success");
  } else if (delStatus == ESP_ERR_ESPNOW_NOT_INIT) {
    // How did we get so far!!
    Serial.println("ESPNOW Not Init");
  } else if (delStatus == ESP_ERR_ESPNOW_ARG) {
    Serial.println("Invalid Argument");
  } else if (delStatus == ESP_ERR_ESPNOW_NOT_FOUND) {
    Serial.println("Peer not found.");
  } else {
    Serial.println("Not sure what happened");
  }
}


// send data
void sendData() {
  data++;
  const uint8_t *peer_addr = slave.peer_addr;
  Serial.print("Sending: "); Serial.println(data);
  esp_err_t result = esp_now_send(peer_addr, (uint8_t *)&serialData, sizeof(serialData));    // send data
  Serial.print("Send Status: ");
  if (result == ESP_OK) {
    sendCompleteFlag = 1;
    Serial.println("Success");
  } else if (result == ESP_ERR_ESPNOW_NOT_INIT) {
    // How did we get so far!!
    Serial.println("ESPNOW not Init.");
  } else if (result == ESP_ERR_ESPNOW_ARG) {
    Serial.println("Invalid Argument");
  } else if (result == ESP_ERR_ESPNOW_INTERNAL) {
    Serial.println("Internal Error");
  } else if (result == ESP_ERR_ESPNOW_NO_MEM) {
    Serial.println("ESP_ERR_ESPNOW_NO_MEM");
  } else if (result == ESP_ERR_ESPNOW_NOT_FOUND) {
    Serial.println("Peer not found.");
  } else {
    Serial.println("Not sure what happened");
  }
}