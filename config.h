#ifndef CONFIG_H
#define CONFIG_H

#include <variant>
#include <ArduinoJson.h>

namespace config
{
    enum class Mode : uint8_t
    {
        OFF = 0,
        WORDCLOCK = 1,
        DIGICLOCK = 2
    };

    struct ModeConfig
    {
        // 4 bits for mode + config data
        // 0000 = off
        // 0001 = wordclock + 24 bit configData
        // 0010 = digiclock
        //
        uint8_t mode = 0b0;
        uint64_t config[2] = {0,0};
        // 8bit red, 8bit green, 8bit blue, 8 bit brightness
        uint32_t color = 0;

        char name[11] = {0};
    };

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

    struct TimerConfig
    {
        // 4 bits for timer mode
        // 0000 = always
        // 0001 = interval / interval in 15s 6 bits
        // 0010 = block /start and end in minutes (1440) / 11 bits / interval in 15s 6 bits
        uint32_t timer = 0;
    };


    struct EEPROMConfig
    {
        constexpr static uint8_t MODE_COUNT = 8;
        constexpr static uint8_t INIT_MARKER = 0x42;

        uint8_t initMarker;

        // 0000 = fix mode / 4 bits for startstate
        // 0001 = auto mode
        uint8_t startMode = 0;
        ModeConfig modes[MODE_COUNT];
        TimerConfig timers[MODE_COUNT];
    };

    typedef std::variant<OffConfig, WordClockConfig, DigiClockConfig> UnpackedModeConfig;

    struct UnpackedEEPROMConfig
    {
        constexpr static uint8_t MODE_COUNT = EEPROMConfig::MODE_COUNT;

        uint8_t startMode = 0;
                
        UnpackedModeConfig modes[MODE_COUNT];
    };


    void unpackConfig(const EEPROMConfig& eepromConfig, UnpackedEEPROMConfig& config);
    void packConfig(EEPROMConfig& eepromConfig, UnpackedEEPROMConfig &config);


    void serializeModeConfigToJson(JsonObject doc, const UnpackedModeConfig &config);
    void serializeConfigToJson(JsonObject doc, const UnpackedEEPROMConfig &config);
}

#endif