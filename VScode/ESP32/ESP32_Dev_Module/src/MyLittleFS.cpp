#include "MyLittleFS.h"

void MyLittleFS::InitLitteFS()
{
  Serial.print("Mounting LittleFS filesystem...");
  if(!LittleFS.begin(FORMAT_LITTLEFS_IF_FAILED))
  {
    Serial.println("ERROR: LittleFS Mount Failed!");
  }
  else
  {
    Serial.println("Mounted!");
  }
}

void MyLittleFS::listDir(fs::FS &fs, const char * dirname, uint8_t levels)\
{

    LINE;
    Serial.printf("Listing directory: %s\r\n", dirname);

    File root = fs.open(dirname);
    if(!root){
        Serial.println("- failed to open directory");
        return;
    }
    if(!root.isDirectory()){
        Serial.println(" - not a directory");
        return;
    }
    
    File file = root.openNextFile();
    while(file){
        if(file.isDirectory()){
            Serial.print("  DIR : ");

#ifdef CONFIG_LITTLEFS_FOR_IDF_3_2
            Serial.println(file.name());
#else
            Serial.print(file.name());
            time_t t= file.getLastWrite();
            struct tm * tmstruct = localtime(&t);
            Serial.printf("  LAST WRITE: %d-%02d-%02d %02d:%02d:%02d\n",(tmstruct->tm_year)+1900,( tmstruct->tm_mon)+1, tmstruct->tm_mday,tmstruct->tm_hour , tmstruct->tm_min, tmstruct->tm_sec);
#endif

            if(levels){
                listDir(fs, file.name(), levels -1);
            }
        } else {
            Serial.print("  FILE: ");
            Serial.print(file.name());
            Serial.print("  SIZE: ");

#ifdef CONFIG_LITTLEFS_FOR_IDF_3_2
            Serial.println(file.size());
#else
            Serial.print(file.size());
            time_t t= file.getLastWrite();
            struct tm * tmstruct = localtime(&t);
            Serial.printf("  LAST WRITE: %d-%02d-%02d %02d:%02d:%02d\n",(tmstruct->tm_year)+1900,( tmstruct->tm_mon)+1, tmstruct->tm_mday,tmstruct->tm_hour , tmstruct->tm_min, tmstruct->tm_sec);
#endif
        }
        file = root.openNextFile();
    }
    LINE;
}

void MyLittleFS::createDir(fs::FS &fs, const char * path){
    LINE;
    Serial.printf("Creating Dir: %s\n", path);
    if(fs.mkdir(path)){
        Serial.println("Dir created");
    } else {
        Serial.println("mkdir failed");
    }
    LINE;
}

void MyLittleFS::removeDir(fs::FS &fs, const char * path){
    LINE;
    Serial.printf("Removing Dir: %s\n", path);
    if(fs.rmdir(path)){
        Serial.println("Dir removed");
    } else {
        Serial.println("rmdir failed");
    }
    LINE;
}

void MyLittleFS::writeFile(fs::FS &fs, const char * path, const char * message){
    LINE;
    Serial.printf("Writing file: %s\r\n", path);

    File file = fs.open(path, FILE_WRITE);
    if(!file){
        Serial.println("- failed to open file for writing");
        return;
    }
    
    if(file.print(message)){
        Serial.println("- file written");
    } else {
        Serial.println("- write failed");
    }
    file.close();
    LINE;
}

void MyLittleFS::readFile(fs::FS &fs, const char * path){
    LINE;
    Serial.printf("Reading file: %s\r\n", path);

    File file = fs.open(path);
    if(!file || file.isDirectory()){
        Serial.println("- failed to open file for reading");
        return;
    }
    
    Serial.println("- read from file:");
    while(file.available()){
        Serial.write(file.read());
    }
    file.close();
    LINE;
}


void MyLittleFS::appendFile(fs::FS &fs, const char * path, const char * message){
    LINE;
    Serial.printf("Appending to file: %s\r\n", path);

    File file = fs.open(path, FILE_APPEND);
    if(!file){
        Serial.println("- failed to open file for appending");
        return;
    }
    if(file.print(message)){
        Serial.println("- message appended");
    } else {
        Serial.println("- append failed");
    }
    file.close();
    LINE;
}

void MyLittleFS::renameFile(fs::FS &fs, const char * path1, const char * path2){
    LINE;
    Serial.printf("Renaming file %s to %s\r\n", path1, path2);
    if (fs.rename(path1, path2)) {
        Serial.println("- file renamed");
    } else {
        Serial.println("- rename failed");
    }
    LINE;
}

void MyLittleFS::deleteFile(fs::FS &fs, const char * path){
    LINE;
    Serial.printf("Deleting file: %s\r\n", path);
    if(fs.remove(path)){
        Serial.println("- file deleted");
    } else {
        Serial.println("- delete failed");
    }
    LINE;
}

// SPIFFS-like write and delete file, better use #define CONFIG_LITTLEFS_SPIFFS_COMPAT 1

void MyLittleFS::writeFile2(fs::FS &fs, const char * path, const char * message){
    LINE;
    if(!fs.exists(path)){
		if (strchr(path, '/')) {
            Serial.printf("Create missing folders of: %s\r\n", path);
			char *pathStr = strdup(path);
			if (pathStr) {
				char *ptr = strchr(pathStr, '/');
				while (ptr) {
					*ptr = 0;
					fs.mkdir(pathStr);
					*ptr = '/';
					ptr = strchr(ptr+1, '/');
				}
			}
			free(pathStr);
		}
    }

    Serial.printf("Writing file to: %s\r\n", path);
    File file = fs.open(path, FILE_WRITE);
    if(!file){
        Serial.println("- failed to open file for writing");
        return;
    }
    if(file.print(message)){
        Serial.println("- file written");
    } else {
        Serial.println("- write failed");
    }
    file.close();
    LINE;
}

void MyLittleFS::deleteFile2(fs::FS &fs, const char * path){
    LINE;
    Serial.printf("Deleting file and empty folders on path: %s\r\n", path);

    if(fs.remove(path)){
        Serial.println("- file deleted");
    } else {
        Serial.println("- delete failed");
    }

    char *pathStr = strdup(path);
    if (pathStr) {
        char *ptr = strrchr(pathStr, '/');
        if (ptr) {
            Serial.printf("Removing all empty folders on path: %s\r\n", path);
        }
        while (ptr) {
            *ptr = 0;
            fs.rmdir(pathStr);
            ptr = strrchr(pathStr, '/');
        }
        free(pathStr);
    }
    LINE;
}

bool MyLittleFS::saveConfig(String SSID, String PASS)
{
    String configData;

    //ssid = String(SSID);
    //pass = String(PASS);
    strcpy(ssid, SSID.c_str());
    strcpy(pass, PASS.c_str());
    configData = String("\"SSID\":" + SSID + ",\"PASS\":" + PASS ); // 값 추가

    File configFile = LittleFS.open(configFilePath, "w");
    if (!configFile) 
    {
        Serial.println("Failed to open config file for writing");
        return false;
    }
    configFile.println(configData); // 파일 쓰기

    configFile.close();

    return true;
}

bool MyLittleFS::loadConfig()
{
    File configFile = LittleFS.open(configFilePath, "r");

    if(!configFile)
    {
        Serial.println("Failed to open config file");
        return false;
    }

    // read one line
    String result = configFile.readStringUntil('\n');
    json_parser(result, "SSID").toCharArray(ssid, 32);
    json_parser(result, "PASS").toCharArray(pass, 32);
    //this->ssid = String(json_parser(result, "SSID"));
    //this->pass = String(json_parser(result, "PASS"));
    //json_parser(result, "SSID");
    //json_parser(result, "PASS");
    // String ssidTemp = json_parser(result, "SSID");
    // String passTemp = json_parser(result, "PASS");
    
    // //Serial.printf( "SSID : %s | PASS : %s\n" ,ssidTemp.c_str(), passTemp.c_str() );
    // strcpy(ssid, ssidTemp.c_str());
    // passTemp.toCharArray(pass, 32);
    //strcpy(pass, passTemp.c_str());
    Serial.printf( "SSID : %s | PASS : %s\n" ,ssid, pass);

    return true;
}


String MyLittleFS::json_parser(String target, String key){ 
  String val;
  if (target.indexOf(key) != -1) {
    int st_index = target.indexOf(key);
    int val_index = target.indexOf(':', st_index);
    if (target.charAt(val_index + 1) == '"') {     // 값이 스트링 형식이면
      int ed_index = target.indexOf('"', val_index + 2);
      val = target.substring(val_index + 2, ed_index);
    }
    else {                                   // 값이 스트링 형식이 아니면
      int ed_index = target.indexOf(',', val_index + 1);
      val = target.substring(val_index + 1, ed_index);
    }
  } 
  else {
    Serial.print(key); Serial.println(F(" is not available"));
  }
  //Serial.println(val);
  val.trim();   // \0 값 제거

  //Serial.println(val.length());
  
  return val;
}
