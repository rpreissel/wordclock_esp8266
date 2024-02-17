#ifndef WORDCLOCK_H
#define WORDCLOCK_H

#include <tuple>
#include "ledmatrix.h"
#include "udplogger.h"


class WordClock
{
public:
    WordClock(LEDMatrix& ledmatrix, UDPLogger& logger);

    void show(const uint8_t config[12], uint8_t hours,uint8_t minutes, uint32_t color);  

private:
    int showStringOnClock(const uint8_t config[12], uint8_t hours,uint8_t minutes, uint32_t color);
    void drawMinuteIndicator(uint8_t minutes, uint32_t color);

    LEDMatrix& ledmatrix;
    UDPLogger& logger;
    String message;
};

#endif