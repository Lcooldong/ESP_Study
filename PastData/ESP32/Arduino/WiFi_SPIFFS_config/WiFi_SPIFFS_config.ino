//#include "FS.h" // ESP8266
#include "SPIFFS.h"  // ESP32 SPIFSPIFFS 라이브러리

char ssid[21]          = "YOUR_SSID";
char pass[21]          = "YOUR_PASS";
float timeZone       = 9;
uint8_t summerTime     = 0; // 3600

// convert to String
String Name(String a) {  
  String temp = "\"{v}\":";
  temp.replace("{v}", a);
  return temp;
}

String strVal(String a) {  
  String temp = "\"{v}\",";
  temp.replace("{v}", a);
  return temp;
}

String intNum(int a) {  
  String temp = "{v},";
  temp.replace("{v}", String(a));
  return temp;
}

String floatNum(float a) {  
  String temp = "{v},";
  temp.replace("{v}", String(a));
  return temp;
}

void stringTo(String ssidTemp, String passTemp) { // 스트링 SSID / PASS 배열에 저장
  for (int i = 0; i < ssidTemp.length(); i++) ssid[i] = ssidTemp[i];
  ssid[ssidTemp.length()] = '\0';
  for (int i = 0; i < passTemp.length(); i++) pass[i] = passTemp[i];
  pass[passTemp.length()] = '\0';
}

/* 데이터 저장 형식 - JSON 형식
   첫 번째, "스트링"처럼 큰 따옴표 기호(")로 묶인 스트링은 변수 명이고 고유하다.   
   두 번째, 변수에 대한 값은 연이어 나오는':'문자 다음에 위치한다. 
   세 번째, 값이 스트링이면 큰 따옴표(")로 묶이고, 값이 숫자(소수/정수)이면 큰 따옴표(")가 없다 
   네 번째, 값 다음에 반드시 콤마(,)가 있다. */

bool saveConfig() { // "SSID":"YOUR_SSID","PASS":"YOUR_PASS","ZONE":9.00,"SUMMER":0,
  String value;
  value = Name("SSID") + strVal(ssid);
  value += Name("PASS") + strVal(pass);
  value += Name("ZONE") + floatNum(timeZone);
  value += Name("SUMMER") + intNum(summerTime);
  File configFile = SPIFFS.open("/config.txt", "w");
  if (!configFile) {
    Serial.println("Failed to open config file for writing");
    return false;
  }
  configFile.println(value); // SPIFF config.txt에 데이터 저장, '\n'포함
  configFile.close();
  return true;
}

String json_parser(String s, String a) { 
  String val;
  if (s.indexOf(a) != -1) {
    int st_index = s.indexOf(a);
    int val_index = s.indexOf(':', st_index);
    if (s.charAt(val_index + 1) == '"') {     // 값이 스트링 형식이면
      int ed_index = s.indexOf('"', val_index + 2);
      val = s.substring(val_index + 2, ed_index);
    }
    else {                                   // 값이 스트링 형식이 아니면
      int ed_index = s.indexOf(',', val_index + 1);
      val = s.substring(val_index + 1, ed_index);
    }
  } 
  else {
    Serial.print(a); Serial.println(F(" is not available"));
  }
  return val;
}

bool loadConfig() {
  File configFile = SPIFFS.open("/config.txt", "r");
  if (!configFile) {
    Serial.println("Failed to open config file");
    return false;
  }
  String line = configFile.readStringUntil('\n');
  configFile.close();
  String ssidTemp = json_parser(line, "SSID");
  String passTemp = json_parser(line, "PASS");
  stringTo(ssidTemp, passTemp);                // String을 배열에 저장
  String temp = json_parser(line, "ZONE");
  timeZone = temp.toFloat();                   // 스트링을 float로 변환
  temp = json_parser(line, "SUMMER");
  summerTime = temp.toInt();                   // 스트링을 int로 변환
  return true;
}

void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.println("Mounting FS...");
  if (!SPIFFS.begin()) {
    Serial.println("Failed to mount file system");
    return;
  }
  if (SPIFFS.exists("/config.txt")) loadConfig();
  else saveConfig();
  Serial.println(ssid);
  Serial.println(pass);
  Serial.println(timeZone);
  Serial.println(summerTime);
}

void loop() {
  if(Serial.available() > 0){
    String temp = Serial.readStringUntil('\n');
    if (temp == "1") {
      Serial.println("File System Format....");
      if(SPIFFS.format())  Serial.println("File System Formated");  //Format File System
      else   Serial.println("File System Formatting Error");
    }
    else if (temp.startsWith("id:")) {
      Serial.println("SPIFF test");
      temp.remove(0, 3);
      int index = temp.indexOf(",");
      String ssidTemp = temp.substring(0, index);
      temp.remove(0, index+1);
      String passTemp = temp;
      stringTo(ssidTemp, passTemp);
      timeZone = -1.5;
      summerTime = 30;
      saveConfig();
      loadConfig();
      Serial.println(ssid);
      Serial.println(pass);
      Serial.println(timeZone);
      Serial.println(summerTime);
    }
  }
}
