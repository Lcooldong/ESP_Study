#ifndef __MYLITTLEFS_H__
#define __MYLITTLEFS_H__

#include <LittleFS.h>

void littleFS_init();
void listDir(fs::FS &fs, const char * dirname, uint8_t levels);

#endif
