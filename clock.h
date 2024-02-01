#ifndef clock_h
#define clock_h

#include "ledmatrix.h"
#include "udplogger.h"

class Clock
{
public:
    Clock(const String& clockString, LEDMatrix& ledmatrix, UDPLogger& logger)
        :clockString(clockString),ledmatrix(ledmatrix), logger(logger)  {}

    void show(uint8_t hours,uint8_t minutes, uint32_t color);    
private:
    int showStringOnClock(String message, uint32_t color);
    void drawMinuteIndicator(uint8_t minutes, uint32_t color);
    String timeToString(uint8_t hours,uint8_t minutes);

    const String& clockString;
    LEDMatrix& ledmatrix;
    UDPLogger& logger;
};

#endif