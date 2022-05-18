#include "FS.h"     // ESP8266

unsigned long int atime;        // 시작 시간, 밀리 초

void listDir(const char * dirname){
  Serial.printf("Listing directory: %s\r\n", dirname);
  Dir dir = SPIFFS.openDir(dirname);
  while (dir.next()) {
    Serial.print("File Name: "); Serial.print(dir.fileName());
    if(dir.fileSize()) {
      File f = dir.openFile("r");
      Serial.print(", Size: "); Serial.println(f.size());
    }
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println();
  if (!SPIFFS.begin()) {
    Serial.println("Failed to mount file system");
    return;
  }
  FSInfo fsInfo;
  SPIFFS.info(fsInfo);
  Serial.print("totalBytes: "); Serial.println(fsInfo.totalBytes);
  Serial.print("usedBytes: "); Serial.println(fsInfo.usedBytes);
  listDir("/");
  String temp = "test, 53.3, teH_Sst\"2, 5.2, 85"; // 저장될 문자열 "test, 53.3, teH_Sst"2, 5.2, 85"
  atime = micros();
  File f =  SPIFFS.open("/Stream_test.txt", "w");  
  f.println(temp);  // println사용 - '\n'문자 포함 쓰기
  f.close();
  atime = micros() - atime;
  Serial.print("Wirting time: "); Serial.print(atime); Serial.println(" micro sec");
  f = SPIFFS.open("/Stream_test.txt", "r");
  Serial.println(f.readStringUntil('\n')); // '\n'문자 까지 읽기
  f.close();
  f = SPIFFS.open("/Stream_test.txt", "r");
  Serial.println(f.parseFloat());   // 스트림 함수 테스트
  Serial.println(f.parseInt());
  Serial.println(f.parseFloat());
  Serial.println(f.parseInt());
  f.close(); 
  f = SPIFFS.open("/Stream_test.txt", "r");
  uint16_t position_t;
  f.find("Sst\"");  // 문자열 Sst" 검색 
  position_t = f.position();
  Serial.print("Sst\" position: "); Serial.println(position_t); // Sst" position
  Serial.println(char(f.read())); // f.read()는 byte로 읽는다 -> DEC 50: - CHAR 2
  f.close(); 
  f = SPIFFS.open("/Stream_test.txt", "r");
  f.find("H");  // "" 문자 또는 문자열("H_S") 검색 
  position_t = f.position();
  Serial.print("H position: "); Serial.println(position_t); // " position
  char tempA[4] = { '\0', }; // 문자열로 출력하기 위해 '\0'으로 초기화, 문자열 크기: 배열크기 -1 
  f.readBytes(tempA, sizeof(tempA)-1);  // 배열 크기 -1 보다 작게
  Serial.println(tempA);
  f.find('"');  // 따옴표를 검색할 경우에는 ''사용 문자 검색 사용 나머지는 "" 사용
  position_t = f.position();
  Serial.print("\" position: "); Serial.println(position_t); // '"' position
  char tempB[7] = { '\0', }; // 문자열로 출력하기 위해 '\0'으로 초기화, 문자열 크기: 배열크기 -1 
  f.readBytes(tempB, sizeof(tempB)-1);  // 배열 크기 -1
  Serial.println(tempB);
  f.close();
  f = SPIFFS.open("/Stream_test.txt", "r"); // "test, 53.3, teH_Sst\"2, 5.2, 85"
  byte len = f.size();
  Serial.println(len);
  while (position_t < len) {
    f.findUntil("e", ",");  // 문자 'e'를  ','를 기준으로 검색한다. 
    position_t = f.position();
    Serial.print("Cursor position: "); Serial.println(position_t); // 검색중 커서의 위치를 출력한다.
  }
  f.close();
  Serial.println("test finished"); 
}

void loop() {
  if(Serial.available() > 0){
    String temp = Serial.readStringUntil('\n');
    if (temp == "1") {      // 시리얼 모니터에 1을 입력하면 포멧
      Serial.println("File System Format....");
      if(SPIFFS.format())  Serial.println("File System Formated");  //Format File System
      else   Serial.println("File System Formatting Error");
    }
    else if (temp == "2") { // 2를 입력하면 파일 리스트 출력
      listDir("/");
    }
  }
}
