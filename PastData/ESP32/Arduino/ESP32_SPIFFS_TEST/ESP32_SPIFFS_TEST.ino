#include "SPIFFS.h"

#define FORMAT_SPIFFS_IF_FAILED true    // 마운트 실패시 SPIFFS 파일 시스템 포맷

// 내부 Flash Memory 에 접근

void setup() {
  Serial.begin(115200);
  if(!SPIFFS.begin(FORMAT_SPIFFS_IF_FAILED)){
    Serial.println("SPIFFS Mount Failed");
    return;
  }


  File file = SPIFFS.open("/hello.txt", "w");
  if(!file){
    Serial.println("fail to open");
    return;  
  }
  if(file.print("Hello")){
    Serial.println("- file written");
  } else {
    Serial.println("- frite failed");
  }

  file.close(); 

  file = SPIFFS.open("/hello.txt", "r");
  if(!file || file.isDirectory()){
    Serial.println("- failed to open file for reading");
    return;
  }
  Serial.println("- read from file:");
  while(file.available()){
    Serial.write(file.read());
  }

  file.close();

}

void loop() {
  // put your main code here, to run repeatedly:

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
