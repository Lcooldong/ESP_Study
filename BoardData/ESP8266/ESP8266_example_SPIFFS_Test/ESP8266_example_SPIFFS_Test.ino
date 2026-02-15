#include "FS.h"


void setup(){
  Serial.begin(115200);
  Serial.println();
  if (!SPIFFS.begin()) {
    Serial.println("Failed to mount file system");
    return;
  }
  SPIFFS.format();
  FSInfo fsInfo;
  SPIFFS.info(fsInfo);
  Serial.print("totalBytes: "); Serial.println(fsInfo.totalBytes);
  Serial.print("usedBytes: "); Serial.println(fsInfo.usedBytes);
  listDir("/");
  writeFile("/hello.txt", "Hello ");
  readFile("/hello.txt");
  appendFile("/hello.txt", "World!\r\n");
  readFile("/hello.txt");
  renameFile("/hello.txt", "/foo.txt");
  readFile("/foo.txt");
  deleteFile("/foo.txt");
  testFileIO("/test.txt");
  deleteFile("/test.txt");
  Serial.println( "Test complete" );
}

void loop(){

}

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

void readFile(const char * path){
  Serial.printf("Reading file: %s\r\n", path);
  File file = SPIFFS.open(path, "r");
  if(!file || file.isDirectory()){
    Serial.println("- failed to open file for reading");
    return;
  }
  Serial.println("read from file:");
  while(file.available()){
    Serial.write(file.read());
  }
}

void writeFile(const char * path, const char * message){
  Serial.printf("Writing file: %s\r\n", path);
  File file = SPIFFS.open(path, "w");
  if(!file){
    Serial.println("failed to open file for writing");
    return;
  }
  if(file.print(message)){
    Serial.println("file written");
  } else {
    Serial.println("frite failed");
  }
}

void appendFile(const char * path, const char * message){
  Serial.printf("Appending to file: %s\r\n", path);
  File file = SPIFFS.open(path, "a");
  if(!file){
    Serial.println("failed to open file for appending");
      return;
  }
  if(file.print(message)){
    Serial.println("message appended");
  } else {
    Serial.println("append failed");
  }
}

void renameFile(const char * path1, const char * path2){
  Serial.printf("Renaming file %s to %s\r\n", path1, path2);
  if (SPIFFS.rename(path1, path2)) {
    Serial.println("file renamed");
  } else {
    Serial.println("rename failed");
  }
}

void deleteFile(const char * path){
  Serial.printf("Deleting file: %s\r\n", path);
  if(SPIFFS.remove(path)){
    Serial.println("file deleted");
  } else {
    Serial.println("delete failed");
  }
}

void testFileIO(const char * path){
  Serial.printf("Testing file I/O with %s\r\n", path);
  static uint8_t buf[512];
  size_t len = 0;
  File file = SPIFFS.open(path, "w");
  if(!file){
    Serial.println("failed to open file for writing");
    return;
  }
  size_t i;
  Serial.print("writing" );
  uint32_t start = millis();
  for(i=0; i<1024; i++){      // ESP8266 - 1024
    if ((i & 0x001F) == 0x001F){
      Serial.print(".");
    }
    file.write(buf, 512);
  }
  Serial.println("");
  uint32_t end = millis() - start;
  Serial.printf("%u bytes written in %u ms\r\n", 1024 * 512, end);
  file.close();
  file = SPIFFS.open(path, "r");
  start = millis();
  end = start;
  i = 0;
  if(file && !file.isDirectory()){
    len = file.size();
    size_t flen = len;
    start = millis();
    Serial.print("reading" );
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
    Serial.printf("%u bytes read in %u ms\r\n", flen, end);
    file.close();
  } else {
    Serial.println("failed to open file for reading");
  }
}
