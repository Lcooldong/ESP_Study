#ifndef FileSystem_h
#define FileSystem_h

#include "SPIFFS.h"

void init_SPIFFS(bool format);
void listDir(const char * dirname);
void readFile(const char * path);
void writeFile(const char * path, const char * message);
void appendFile(const char * path, const char * message);
void renameFile(const char * path1, const char * path2);
void deleteFile(const char * path);

bool saveConfig();
bool loadConfig();
String json_parser(String s, String a);
String Name(String a);
String strVal(String a);
String intNum(int a);
String floatNum(float a) ;
void stringTo(String ssidTemp, String passTemp);

void testFileIO(const char * path);



#endif
