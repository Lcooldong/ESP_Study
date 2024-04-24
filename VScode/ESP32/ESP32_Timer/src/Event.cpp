#include "Event.h"


#if defined(ARDUINO) && ARDUINO >= 100
#include <Arduino.h>
#else
#include "WProgram.h"
#endif

Event::Event(void)
{
    eventType = EVENT_NONE;
}

void Event::update(void)
{
    unsigned long now = millis();
    update(now);
}

void Event::update(unsigned long now)
{
    if(now - lastEventTime >= period)
    {
        switch (eventType)
        {
        case EVENT_EVERY:
            (*callback)();
            break;
        case EVENT_OSCILLATE:
            pinState = !pinState;
            digitalWrite(pin, pinState);
            break;
        }
        lastEventTime = now;
        count++;
    }
    
    if(repeatCount > -1 && count >= repeatCount)
    {
        eventType = EVENT_NONE;

    }
}