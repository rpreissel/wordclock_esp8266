#ifndef CONFIG_H
#define CONFIG_H

#include <variant>
#include <ArduinoJson.h>

namespace config
{

    struct BaseConfig
    {
        uint8_t index;
        uint8_t brightness;
        uint32_t color;
        String name;
    };

    struct OffConfig : BaseConfig
    {
    };

    struct WordClockConfig : BaseConfig
    {
        uint8_t config[12] = {0,0,0,0,0,0,0,0,0,0,0,0};
    };

    struct DigiClockConfig : BaseConfig
    {
    };


    typedef std::variant<std::monostate,OffConfig, WordClockConfig, DigiClockConfig> ModeConfig;

}

#endif