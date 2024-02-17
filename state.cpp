#include <variant>
#include <tuple>
#include <EEPROM.h> //from ESP8266 Arduino Core (automatically installed when ESP8266 was installed via Boardmanager)
#include "state.h"
#include "tools.h"
#include "timedef.h"
#include <ArduinoJson.h>

namespace eeprom
{
    constexpr uint8_t MODE_COUNT = 8;
    constexpr uint8_t INIT_MARKER = 0x42;

    enum class Type : uint8_t
    {
        EMPTY = 0,
        WORDCLOCK = 1,
        DIGICLOCK = 2
    };

    struct ModeConfig
    {
        // 4 bits for type + config data
        // 0001 = wordclock
        // 0010 = digiclock
        uint8_t type = 0b0;
        uint64_t config[2] = {0, 0};
        // 8bit red, 8bit green, 8bit blue, 8 bit brightness
        uint32_t color = 0;

        char name[11] = {0};
    };

    struct EEPROMConfig
    {

        uint8_t initMarker;

        // 0000 = fix mode / 4 bits for startstate
        // 0001 = auto mode
        uint8_t startMode = 0;
        ModeConfig modes[MODE_COUNT];
    };
}

namespace config
{
    constexpr const char *TYPE_NAME_EMPTY = "EMPTY";
    constexpr const char *TYPE_NAME_OFF = "OFF";
    constexpr const char *TYPE_NAME_WORDCLOCK = "WORDCLOCK";
    constexpr const char *TYPE_NAME_DIGICLOCK = "DIGICLOCK";

    struct Error
    {
    };

    struct Initialized
    {
        ESP8266WebServer &server;
        UDPLogger &logger;

        UpdateHandler updateHandler;

        eeprom::EEPROMConfig eepromConfig;
        ModeConfig modes[eeprom::MODE_COUNT];
        ModeConfig currentMode;

        Initialized(ESP8266WebServer &server, UDPLogger &logger, UpdateHandler updateHandler)
            : server(server), logger(logger), updateHandler(updateHandler) {}
    };

    std::variant<std::monostate, Initialized, Error> moduleState;

    void loadFromEEProm(Initialized &init);
    void initServerEndpoints(ESP8266WebServer &server, UDPLogger &logger, UpdateHandler updateHandler);
    void unpackModeConfig(const eeprom::ModeConfig modeConfigs[], uint8_t index, ModeConfig &config);

    ModeConfig init(ESP8266WebServer &server, UDPLogger &logger, UpdateHandler updateHandler)
    {
        Serial.println(F("enter init"));
        initServerEndpoints(server, logger, updateHandler);
        return std::visit(
            Overload{
                [&](std::monostate &empty)
                {
                    Serial.println(F("init first time"));
                    // Init EEPROM
                    EEPROM.begin(sizeof(eeprom::EEPROMConfig));

                    auto &init = moduleState.emplace<Initialized>(server, logger, updateHandler);
                    loadFromEEProm(init);
                    Serial.println("init currentMode " + init.currentMode.index());
                    return init.currentMode;
                },
                [](Initialized &init)
                {
                    return init.currentMode;
                },
                [](auto &init)
                {
                    return ModeConfig(OffConfig());
                }},
            moduleState);
    }

    void loadFromEEProm(Initialized &init)
    {
        EEPROM.get(0, init.eepromConfig);
        if (init.eepromConfig.initMarker != eeprom::INIT_MARKER)
        {
            auto baseConfig = [](BaseConfig &config, int index)
            {
                config.index = index;
                config.brightness = 50;
            };

            auto &first = init.modes[0].emplace<WordClockConfig>();
            first.name = "WordClock";
            baseConfig(first, 0);
            auto &second = init.modes[1].emplace<DigiClockConfig>();
            second.name = "DigiClock";
            baseConfig(second, 1);

            for (int i = 2; i < eeprom::MODE_COUNT; i++)
            {
                init.modes[i].emplace<Empty>().index = i;
            }

            init.currentMode = init.modes[0];
        }
        else
        {
            for (int i = 0; i < eeprom::MODE_COUNT; i++)
            {
                unpackModeConfig(init.eepromConfig.modes, i, init.modes[i]);
            }

            init.currentMode = init.modes[init.eepromConfig.startMode];
            if (std::holds_alternative<Empty>(init.currentMode))
            {
                init.currentMode = init.modes[0];
            }
        }
    }

    void unpackModeConfig(const eeprom::ModeConfig modeConfigs[], uint8_t index, ModeConfig &config)
    {
        eeprom::ModeConfig modeConfig = modeConfigs[index];
        eeprom::Type mode = static_cast<eeprom::Type>(modeConfig.type && 0b1111);

        switch (mode)
        {
        case eeprom::Type::WORDCLOCK:
        {
            auto &wordClockConfig = config.emplace<WordClockConfig>();
            uint32_t bitConfig = modeConfig.config[0] && 0b11'11'11'11'11'11'11'11'11'11'11'11;
            for (int c = 0; c < 12; c++)
            {
                wordClockConfig.config[c] = (bitConfig >> (2 * c)) & 0x03;
            }
        }
        break;
        case eeprom::Type::DIGICLOCK:
            config.emplace<DigiClockConfig>();
            break;
        default:
            config.emplace<Empty>();
            break;
        }

        std::visit(
            Overload{
                [&modeConfig, index](BaseConfig &config)
                {
                    config.index = index;
                    config.name = modeConfig.name;
                    config.brightness = modeConfig.color >> 24;
                    config.color = modeConfig.color & 0x0fff;
                },
                [](auto &config) {

                }},
            config);
    }

    void on(ESP8266WebServer &server, const String &uri, HTTPMethod method, UpdateHandler updateHandler, std::function<bool(Initialized &)> handler)
    {
        server.on(uri, method, [handler, &server, &uri, updateHandler]()
                  { std::visit(
                        Overload{
                            [handler, &uri, updateHandler](Initialized &init)
                            {
                                Serial.printf("handle %s (%d)\r", uri.c_str(), moduleState.index());
                                if (handler(init))
                                {
                                    updateHandler(init.currentMode);
                                }
                            },
                            [&server, &uri](auto &init)
                            {
                                Serial.printf("error %s (%d)\r", uri.c_str(), moduleState.index());
                                server.send(400, "application/json", "{}");
                            }},
                        moduleState); });
    }

    void modeToJson(JsonObject current, ModeConfig mode)
    {
        auto baseConfigJson = [](JsonObject doc, const BaseConfig &config)
        {
            doc[F("index")] = config.index;
            doc[F("name")] = config.name;
            doc[F("color")] = config.color;
            doc[F("brightness")] = config.brightness;
        };
        std::visit(Overload{
                       [current](const Empty &config)
                       {
                           current[F("index")] = config.index;
                           current[F("type")] = TYPE_NAME_EMPTY;
                       },
                       [current](const OffConfig &config)
                       {
                           current[F("index")] = config.index;
                           current[F("type")] = TYPE_NAME_OFF;
                       },
                       [current, &baseConfigJson](const WordClockConfig &config)
                       {
                           current[F("type")] = TYPE_NAME_WORDCLOCK;
                           baseConfigJson(current, config);
                           auto configArray = current[F("config")].to<JsonArray>();
                           for (int c = 0; c < 12; c++)
                           {
                               configArray.add(config.config[c]);
                           }
                       },
                       [current, &baseConfigJson](const DigiClockConfig &config)
                       {
                           current[F("type")] = TYPE_NAME_DIGICLOCK;
                           baseConfigJson(current, config);
                       },

                   },
                   mode);
    }

    bool onGetCurrent(ESP8266WebServer &server, UDPLogger &logger, Initialized &init)
    {

        JsonDocument json;
        JsonObject current = json["current"].to<JsonObject>();
        modeToJson(current, init.currentMode);

        auto modes = json["modes"].to<JsonArray>();
        for (int i = 0; i < eeprom::MODE_COUNT; i++)
        {
            auto mode = modes.add<JsonObject>();
            mode[F("index")] = i;
            std::visit(Overload{
                           [mode](const Empty &config)
                           {
                               mode[F("type")] = TYPE_NAME_EMPTY;
                           },
                           [mode](const OffConfig &config)
                           {
                               mode[F("type")] = TYPE_NAME_OFF;
                               mode[F("index")] = MODE_OFF_INDEX;
                           },
                           [mode](const WordClockConfig &config)
                           {
                               mode[F("type")] = TYPE_NAME_WORDCLOCK;
                               mode[F("name")] = config.name;
                           },
                           [mode](const DigiClockConfig &config)
                           {
                               mode[F("type")] = TYPE_NAME_DIGICLOCK;
                               mode[F("name")] = config.name;
                           },
                       },
                       init.modes[i]);
        }

        String message;
        serializeJsonPretty(json, message);

        server.send(200, "application/json", message);
        return false;
    }

    int modeIndex(const ModeConfig &config)
    {
        return std::visit(Overload{
                              [](const Empty &config)
                              {
                                  return config.index;
                              },
                              [](const OffConfig &config)
                              {
                                  return config.index;
                              },
                              [](const WordClockConfig &config)
                              {
                                  return config.index;
                              },
                              [](const DigiClockConfig &config)
                              {
                                  return config.index;
                              },
                          },
                          config);
    }

    
    String modeName(const ModeConfig &config)
    {
        return std::visit(Overload{
                              [](const Empty &config)
                              {
                                  return String(TYPE_NAME_EMPTY);
                              },
                              [](const OffConfig &config)
                              {
                                  return String(TYPE_NAME_OFF);
                              },
                              [](const WordClockConfig &config)
                              {
                                    char s[32];
                                    snprintf(s, sizeof(s), "%s (%s)", config.name, TYPE_NAME_WORDCLOCK);
                                    return String(s);
                              },
                              [](const DigiClockConfig &config)
                              {
                                  char s[32];
                                    snprintf(s, sizeof(s), "%s (%s)", config.name, TYPE_NAME_DIGICLOCK);
                                    return String(s);
                              },
                          },
                          config);
    }
    bool onChangeCurrent(ESP8266WebServer &server, UDPLogger &logger, Initialized &init)
    {
        auto text = server.arg("plain");
        JsonDocument doc;
        DeserializationError error = deserializeJson(doc, text);
        if (error)
        {
            Serial.println(F("deserializeJson() failed: "));
            server.send(400, "text/plain", error.c_str());
            return false;
        }

        bool notifyUpdateHandler = false;
        int currentModeIndex = modeIndex(init.currentMode);
        if (currentModeIndex >= 0)
        {
            const char *typeCstr = doc[F("type")];
            if (typeCstr)
            {
                auto copyFromOldConfig = [currentModeIndex,&init](BaseConfig &newConfig)
                {
                    std::visit(Overload{
                                   [&newConfig, &init](const Empty &config)
                                   {
                                       newConfig.index = config.index;
                                       newConfig.brightness = 50;
                                       newConfig.color = 0xFFF;
                                   },
                                   [&newConfig](const OffConfig &config)
                                   {
                                       newConfig.index = config.index;
                                       newConfig.brightness = 50;
                                       newConfig.color = 0xFFF;
                                   },
                                   [&newConfig](WordClockConfig &config)
                                   {
                                       newConfig.index = config.index;
                                       newConfig.brightness = config.brightness;
                                       newConfig.color = config.color;
                                       newConfig.name = config.name;
                                   },
                                   [&newConfig](DigiClockConfig &config)
                                   {
                                       newConfig.index = config.index;
                                       newConfig.brightness = config.brightness;
                                       newConfig.color = config.color;
                                       newConfig.name = config.name;
                                   },
                               },
                               init.modes[currentModeIndex]);
                };
                String type(typeCstr);
                if (type == TYPE_NAME_EMPTY)
                {
                    init.currentMode.emplace<Empty>().index = currentModeIndex;
                }
                else if (type == TYPE_NAME_WORDCLOCK)
                {
                    auto &newConfig = init.currentMode.emplace<WordClockConfig>();
                    newConfig.name = F("WordClock");
                    copyFromOldConfig(newConfig);
                }
                else if (type == TYPE_NAME_DIGICLOCK)
                {
                    auto &newConfig = init.currentMode.emplace<DigiClockConfig>();
                    newConfig.name = F("DigiClock");
                    copyFromOldConfig(newConfig);
                }
            }
            auto copyBaseConfigAttributes = [doc](BaseConfig &config)
            {
                JsonVariantConst brightness = doc[F("brightness")];
                if (!brightness.isNull())
                {
                    config.brightness = brightness.as<int>();
                }
                JsonVariantConst color = doc[F("color")];
                if (!color.isNull())
                {
                    config.color = color.as<int>();
                }
                const char *name = doc[F("name")];
                if (name)
                {
                    config.name = name;
                }
            };
            std::visit(Overload{
                           [](const Empty &config) {
                           },
                           [](const OffConfig &config) {
                           },
                           [copyBaseConfigAttributes,doc](WordClockConfig &config)
                           {
                               copyBaseConfigAttributes(config);
                               JsonVariantConst clockConfig = doc[F("config")];
                               if(!clockConfig.isNull()) {
                                    JsonArrayConst ar = clockConfig.as<JsonArrayConst>();
                                    for(int i = 0; i < 12; i++)
                                    {
                                        config.config[i] = ar[i];
                                    }
                               }
                           },
                           [copyBaseConfigAttributes](DigiClockConfig &config)
                           {
                               copyBaseConfigAttributes(config);
                           },
                       },
                       init.currentMode);
            init.modes[currentModeIndex] = init.currentMode;
            notifyUpdateHandler = true;
        }
        doc.clear();
        modeToJson(doc.to<JsonObject>(), init.currentMode);
        String message;
        serializeJsonPretty(doc, message);
        server.send(200, "application/json", message);

        return notifyUpdateHandler;
    }

    bool onChangeMode(ESP8266WebServer &server, UDPLogger &logger, Initialized &init)
    {
        auto text = server.arg("plain");
        JsonDocument doc;
        DeserializationError error = deserializeJson(doc, text);
        if (error)
        {
            Serial.println(F("deserializeJson() failed: "));
            server.send(400, "text/plain", error.c_str());
            return false;
        }

        int mode = doc["mode"];
        bool notifyUpdateHandler = false;
        if (mode >= 0 && mode <= eeprom::MODE_COUNT)
        {
            init.currentMode = init.modes[mode];
            notifyUpdateHandler = true;
        }
        else if (mode == MODE_OFF_INDEX)
        {
            init.currentMode.emplace<OffConfig>();
            notifyUpdateHandler = true;
        }

        doc.clear();
        modeToJson(doc.to<JsonObject>(), init.currentMode);
        String message;
        serializeJsonPretty(doc, message);
        server.send(200, "application/json", message);
        return notifyUpdateHandler;
    }

    bool onTimedefsGet(ESP8266WebServer &server, UDPLogger &logger, Initialized &init)
    {
        auto config = timedef::getConfig();
        JsonDocument json;
        auto ja = json.to<JsonArray>();
        for (int i = 0; i < 12; i++)
        {
            auto jai = ja.add<JsonArray>();
            auto ta = config.periods[i];
            if (ta[0])
            {
                jai.add(ta[0]);
                if (ta[1])
                {
                    jai.add(ta[1]);
                }
            }
        }

        String message;
        serializeJsonPretty(json, message);

        server.send(200, "application/json", message);
        return false;
    }

    void initServerEndpoints(ESP8266WebServer &server, UDPLogger &logger, UpdateHandler updateHandler)
    {
        on(server, "/current", HTTP_GET, updateHandler, [&](Initialized &init)
           { return onGetCurrent(server, logger, init); });
        on(server, "/current", HTTP_PATCH, updateHandler, [&](Initialized &init)
           { return onChangeCurrent(server, logger, init); });
        on(server, "/mode", HTTP_PUT, updateHandler, [&](Initialized &init)
           { return onChangeMode(server, logger, init); });
        on(server, "/timedefs", HTTP_GET, updateHandler, [&](Initialized &init)
           { return onTimedefsGet(server, logger, init); });
    }
}
