#ifndef MY_LITTLEFS_H
#define MY_LITTLEFS_H

#include <FS.h>         // ESP8266
#include <LittleFS.h>   // ESP32
#include <SPIFFS.h>


#define FORMAT_LITTLEFS_IF_FAILED true
#define LINE Serial.println("\n=====================================\n")

#ifndef CONFIG_LITTLEFS_FOR_IDF_3_2
 #include <time.h>
#endif

class MyLittleFS
{
public:
    void InitLitteFS();
    void InitSPIFFS();
    void listDir(fs::FS &fs, const char * dirname, uint8_t levels);
    void createDir(fs::FS &fs, const char * path);
    void removeDir(fs::FS &fs, const char * path);
    void writeFile(fs::FS &fs, const char * path, const char * message);
    void readFile(fs::FS &fs, const char * path);
    void appendFile(fs::FS &fs, const char * path, const char * message);
    void renameFile(fs::FS &fs, const char * path1, const char * path2);
    void deleteFile(fs::FS &fs, const char * path);
    void writeFile2(fs::FS &fs, const char * path, const char * message);
    void deleteFile2(fs::FS &fs, const char * path);

    bool saveConfig(fs::FS &fs, String SSID, String PASS);
    bool saveMotorValue(fs::FS &fs, int stateValue);
    bool saveCurrentState(fs::FS &fs, bool state);
    bool loadConfig(fs::FS &fs);
    bool loadMotorValue(fs::FS &fs);
    bool loadLightState(fs::FS &fs);

    void writeServoLog();

    char ssid[32];
    char pass[32];
    int motorValue;
    char mylight[4];
    bool mylightState;
private:
    

    String configFilePath = "/config.txt";
    String motorStatePath = "/value.txt";
    String lightStatePath = "/light.txt";

    String json_parser(String target, String key);
};


#endif