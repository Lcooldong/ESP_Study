#ifndef __MYOTA_H__
#define __MYOTA_H__

#include <ElegantOTA.h>

class MyOTA
{

private:
    
    static void onOTAStart();
    static void onOTAProgress(unsigned int current, unsigned int final);
    static void onOTAEnd(bool success);

public:
    MyOTA();
    ~MyOTA();

    void initOTA(AsyncWebServer* _server, bool _serverStart);
    void loopOTA();

};



#endif