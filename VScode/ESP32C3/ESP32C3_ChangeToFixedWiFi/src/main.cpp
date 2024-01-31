#include <Arduino.h>
#include <WiFi.h>

const char* ssid = "UARobotics_212_2.4G";
const char* pass = "uarobotics";
const char* apName = "M5STAMP_C3";

void checkWiFiConnection();
int32_t getWiFiChannel(char *ssid);
void scanWiFi();

void setup() {
  Serial.begin(115200);

  WiFi.mode(WIFI_AP_STA);


  WiFi.begin(ssid, pass);

  int32_t channel = getWiFiChannel((char*)ssid);

  bool result = WiFi.softAP(apName, NULL, channel, 0);
  if (!result) 
  {
    Serial.println("AP Config failed.");
  } 
  else 
  {
    Serial.println("AP Config Success. Broadcasting with AP: " + String(apName));
  }

  checkWiFiConnection();

  Serial.println("Dynamic IP");
  Serial.print("\nIP      : ");
  Serial.print(WiFi.localIP());
  Serial.print("\nGateway : ");
  Serial.print(WiFi.gatewayIP());
  Serial.print("\nSubnet  : ");
  Serial.print(WiFi.subnetMask());
  Serial.println();

  Serial.printf("Channel : %d\r\n", channel);


  // for (int i = 0; i < 4; i++)
  // {
  //   Serial.printf("IP [%d] : %d\r\n", i, WiFi.gatewayIP()[i]);
  // }
  
  IPAddress fixedIp(WiFi.localIP()[0], 
                    WiFi.localIP()[1], 
                    WiFi.localIP()[2], 
                    WiFi.localIP()[3] + 1);

  IPAddress gateway = WiFi.gatewayIP();
  IPAddress subnet = WiFi.subnetMask();

  WiFi.disconnect();
  delay(1000);

  

  WiFi.config(fixedIp, gateway, subnet);

  WiFi.begin(ssid, pass);
  
  checkWiFiConnection();

  Serial.println("Fixed IP");
  Serial.print("\nIP      : ");
  Serial.print(WiFi.localIP());
  Serial.print("\nGateway : ");
  Serial.print(WiFi.gatewayIP());
  Serial.print("\nSubnet  : ");
  Serial.print(WiFi.subnetMask());
  Serial.println();

  // 주변 스캔 
  // 해당 AP 비교
  // 각 보드마다 IP 고정

}

void loop() {
  scanWiFi();
  delay(1000);
}


void checkWiFiConnection()
{
    while (WiFi.status() != WL_CONNECTED) 
  {
      delay(500);
      Serial.print(".");
  }


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

void scanWiFi()
{
  int n = WiFi.scanNetworks();
  Serial.println("scan done");
  if (n == 0) {
      Serial.println("no networks found");
  } else {
    Serial.print(n);
    Serial.println(" networks found");
    for (int i = 0; i < n; ++i) {
      // Print SSID and RSSI for each network found
      Serial.print(i + 1);
      Serial.print(": ");
      Serial.print(WiFi.SSID(i));
      Serial.print(" (");
      Serial.print(WiFi.RSSI(i));
      Serial.print(")");
      Serial.println((WiFi.encryptionType(i) == WIFI_AUTH_OPEN)?" ":"*");
      delay(10);
    }
  }
  Serial.println("");

  
}