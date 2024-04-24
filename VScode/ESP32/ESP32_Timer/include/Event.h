#ifndef __EVENT_H__
#define __EVENT_H__

#include <inttypes.h>

#define EVENT_NONE 0
#define EVENT_EVERY 1
#define EVENT_OSCILLATE 2

class Event
{
public:
    Event(void);
    void update(void);
    void update(unsigned long now);
    int8_t eventType;
    unsigned long period;
    int repeatCount;
    uint8_t pin;
    uint8_t pinState;
    void (*callback)(void);
    unsigned long lastEventTime;
    int count;

private:


};


#endif