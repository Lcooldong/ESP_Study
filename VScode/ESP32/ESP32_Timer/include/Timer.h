#ifndef  __TIMER_H__
#define __TIMER_H__

#include <inttypes.h>
#include "Event.h"

#define MAX_NUMBER_OF_EVENTS (10)
#define TIMER_NOT_AN_EVENT (-2)
#define NO_TIMER_AVAILABLE (-1)

class Timer
{

public:
    Timer(void);

    int8_t every(unsigned long period, void (*callback)(void));
    int8_t every(unsigned long period, void (*callback)(void), int repeatCount);
    int8_t after(unsigned long duration, void (*callback)(void));
    int8_t oscillate(uint8_t pin, unsigned long period, uint8_t startingValue);
    int8_t oscillate(uint8_t pin, unsigned long period, uint8_t startingValue, int repeatCount);

    int8_t pulse(uint8_t pin, unsigned long period, uint8_t startingValue);
    int8_t pulseImmediate(uint8_t pin, unsigned long period, uint8_t pulseValue);
    
    void stop(int8_t id);
    void update(void);
    void update(unsigned long now);

protected:
    Event _events[MAX_NUMBER_OF_EVENTS];
    int8_t findFreeEventIndex(void);

private:


};

#endif