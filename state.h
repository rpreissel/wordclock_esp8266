#ifndef STATE_H
#define STATE_H

#include <ESP8266WebServer.h>
#include <variant>
#include "udplogger.h"

namespace config
{
    struct BaseConfig
    {
        uint8_t index;
        uint8_t brightness;
        uint32_t color;
        String name;
    };

    struct Empty
    {
        uint8_t index;
    };
    const int MODE_OFF_INDEX = -1;
    struct OffConfig
    {
        OffConfig(): index(MODE_OFF_INDEX) {}
        uint8_t index;
    };

    struct WordClockConfig : BaseConfig
    {
        uint8_t config[12] = {0,0,0,0,0,0,0,0,0,0,0,0};
        bool fixed;
        uint8_t hours;
        uint8_t minutes;
    };

    struct DigiClockConfig : BaseConfig
    {
    };


    typedef std::variant<Empty,OffConfig, WordClockConfig, DigiClockConfig> ModeConfig;

    typedef std::function<void(const ModeConfig&)> UpdateHandler;
    ModeConfig init(ESP8266WebServer &server, UDPLogger& logger ,UpdateHandler updateHandler);

    String modeName(const ModeConfig& config);
    int modeIndex(const ModeConfig& config);
}
#endif