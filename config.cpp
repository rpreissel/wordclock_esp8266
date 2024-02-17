#include "config.h"
#include "tools.h"
#include "ledmatrix.h"

namespace config
{

    /*

    void packModeConfig(ModeConfig modeConfigs[], UnpackedModeConfig &config)
    {
        std::visit(Overload{
                       // (2)
                       [&](OffConfig &config)
                       {
                           ModeConfig &modeConfig = modeConfigs[config.index];
                           modeConfig.mode = static_cast<uint8_t>(Mode::OFF);
                       },
                       [&](WordClockConfig &config)
                       {
                           ModeConfig &modeConfig = modeConfigs[config.index];
                           modeConfig.mode = static_cast<uint8_t>(Mode::WORDCLOCK);
                           uint32_t bitConfig = 0;
                           for (int c = 0; c < 12; c++)
                           {
                               bitConfig = bitConfig | ((config.config[c] & 0x03) << (2 * c));
                           }
                           modeConfig.config[0] = bitConfig;
                       },
                       [&](DigiClockConfig &config)
                       {
                           ModeConfig &modeConfig = modeConfigs[config.index];
                           modeConfig.mode = static_cast<uint8_t>(Mode::DIGICLOCK);
                       },
                   },
                   config);

        std::visit([&](BaseConfig &config)
                   {
            ModeConfig& modeConfig = modeConfigs[config.index];            
            modeConfig.color = config.brightness << 24 | config.color;
            strcpy(modeConfig.name, config.name.c_str()); },
                   config);
    }

    void unpackConfig(const EEPROMConfig &eepromConfig, UnpackedEEPROMConfig &unpacked)
    {
        if (eepromConfig.initMarker != EEPROMConfig::INIT_MARKER)
        {
            unpacked.startMode = 0;
            unpacked.modes[0].emplace<WordClockConfig>().name = "WordClock";
            unpacked.modes[1].emplace<DigiClockConfig>().name = "DigiClock";
            unpacked.modes[2].emplace<OffConfig>().name = "Off";
            for (int i = 0; i < UnpackedEEPROMConfig::MODE_COUNT; i++)
            {
                std::visit([i](BaseConfig &config)
                           {
                    config.index = i;
                    config.brightness = 50; },
                           unpacked.modes[i]);
            }
        }
        else
        {
            unpacked.startMode = eepromConfig.startMode >> 4 && 0x0F;
            for (int i = 0; i < EEPROMConfig::MODE_COUNT; i++)
            {
                unpackModeConfig(eepromConfig.modes, i, unpacked.modes[i]);
            }
        }
    }

    void packConfig(EEPROMConfig &eepromConfig, UnpackedEEPROMConfig &config)
    {
        eepromConfig.initMarker = EEPROMConfig::INIT_MARKER;
        eepromConfig.startMode = config.startMode;
        for (int i = 0; i < EEPROMConfig::MODE_COUNT; i++)
        {
            packModeConfig(eepromConfig.modes, config.modes[i]);
        }
    }

    void serializeModeConfigToJson(JsonObject doc, const UnpackedModeConfig &unpacked)
    {
        doc.clear();
        std::visit(Overload{
                       [doc](const OffConfig &config)
                       {
                           doc[F("mode")] = F("OFF");
                       },
                       [doc](const WordClockConfig &config)
                       {
                           doc[F("mode")] = F("WORDCLOCK");
                           auto configArray = doc[F("config")].to<JsonArray>();
                           for (int c = 0; c < 12; c++)
                           {
                               configArray.add(config.config[c]);
                           }
                       },
                       [doc](const DigiClockConfig &config)
                       {
                           doc[F("mode")] = F("DIGICLOCK");
                       },
                   },
                   unpacked);

        std::visit([doc](const BaseConfig &config)
                   {
                           doc[F("index")] = config.index;
                            doc[F("name")] = config.name;
                            doc[F("color")] = config.color;
                           doc[F("brightness")] = config.brightness; },
                   unpacked);
    }

    void serializeConfigToJson(JsonObject doc, const UnpackedEEPROMConfig &config)
    {
        doc.clear();
        doc[F("startMode")] = config.startMode;
        auto modes = doc[F("modes")].to<JsonArray>();
        for (int i = 0; i < UnpackedEEPROMConfig::MODE_COUNT; i++)
        {
            serializeModeConfigToJson(modes.add<JsonObject>(), config.modes[i]);
        }
    }
    */
}