#include "FileSystem.h"
#include "Arduino.h"


extern char ssid[32];
extern char pass[32];
float timeZone       = 9;
uint8_t summerTime   = 0; // 3600


void init_SPIFFS(bool format){
   if (!SPIFFS.begin(format)) {
    Serial.println("Failed to mount file system");
    return;
  }
}



void listDir(const char * dirname){
  Serial.printf("Listing directory: %s\r\n", dirname);
  File root = SPIFFS.open(dirname); // ESP8266은 확장자 "Dir"과 "File"로 구분해서 사용, ESP32는 "File"로 통합
  File file = root.openNextFile();
  while(file){ // 다음 파일이 있으면(디렉토리 또는 파일)
    if(file.isDirectory()){ // 다음 파일이 디렉토리 이면
      Serial.print("  DIR : "); Serial.println(file.name()); // 디렉토리 이름 출력
    } else {                // 파일이면
      Serial.print("  FILE: "); Serial.print(file.name());   // 파일이름
      Serial.print("\tSIZE: "); Serial.println(file.size()); // 파일 크기
    }
    file = root.openNextFile();
  }
}

void readFile(const char * path){
  Serial.printf("Reading file: %s\r\n", path);
  File file = SPIFFS.open(path, "r");  // 파을을 열때 읽기 또는 쓰기를 지정하는 옵션이 빠져있다.
  if(!file || file.isDirectory()){
    Serial.println("- failed to open file for reading");
    return;
  }
  Serial.println("- read from file:");
  while(file.available()){
    Serial.write(file.read());
  }
}

void writeFile(const char * path, const char * message){
  Serial.printf("Writing file: %s\r\n", path);
  File file = SPIFFS.open(path, "w");   // "w" or FILE_WRITE
  if(!file){
    Serial.println("- failed to open file for writing");
    return;
  }
  if(file.print(message)){
    Serial.println("- file written");
  } else {
    Serial.println("- frite failed");
  }
}

void appendFile(const char * path, const char * message){
  Serial.printf("Appending to file: %s\r\n", path);
  File file = SPIFFS.open(path, "a");   // "a" or FILE_APPEND
  if(!file){
    Serial.println("- failed to open file for appending");
    return;
  }
  if(file.print(message)){
    Serial.println("- message appended");
  } else {
    Serial.println("- append failed");
  }
}

void renameFile(const char * path1, const char * path2){
  Serial.printf("Renaming file %s to %s\r\n", path1, path2);
  if (SPIFFS.rename(path1, path2)) {
    Serial.println("- file renamed");
  } else {
    Serial.println("- rename failed");
  }
}

void deleteFile(const char * path){
  Serial.printf("Deleting file: %s\r\n", path);
  if(SPIFFS.remove(path)){
    Serial.println("- file deleted");
  } else {
    Serial.println("- delete failed");
  }
}

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

void testFileIO(const char * path){
  Serial.printf("Testing file I/O with %s\r\n", path);
  static uint8_t buf[512];
  size_t len = 0;
  File file = SPIFFS.open(path, "w");
  if(!file){
    Serial.println("- failed to open file for writing");
    return;
  }
  size_t i;
  Serial.print("- writing" );
  uint32_t start = millis();
  for(i=0; i<2048; i++){
    if ((i & 0x001F) == 0x001F){
      Serial.print(".");
    }
    file.write(buf, 512);
  }
  Serial.println("");
  uint32_t end = millis() - start;
  Serial.printf(" - %u bytes written in %u ms\r\n", 2048 * 512, end);
  file.close();
  file = SPIFFS.open(path);
  start = millis();
  end = start;
  i = 0;
  if(file && !file.isDirectory()){
    len = file.size();
    size_t flen = len;
    start = millis();
    Serial.print("- reading" );
    while(len){
      size_t toRead = len;
      if(toRead > 512){
        toRead = 512;
      }
      file.read(buf, toRead);
      if ((i++ & 0x001F) == 0x001F){
        Serial.print(".");
      }
      len -= toRead;
    }
    Serial.println("");
    end = millis() - start;
    Serial.printf("- %u bytes read in %u ms\r\n", flen, end);
    file.close();
  } else {
    Serial.println("- failed to open file for reading");
  }
}
