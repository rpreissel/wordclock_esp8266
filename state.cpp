#include <variant>
#include <tuple>
#include <EEPROM.h> //from ESP8266 Arduino Core (automatically installed when ESP8266 was installed via Boardmanager)
#include "state.h"
#include "tools.h"
#include "timedef.h"
#include "baseconfig.h"

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
        int current_mode_index;

        Initialized(ESP8266WebServer &server, UDPLogger &logger, UpdateHandler updateHandler)
            : server(server), logger(logger), updateHandler(updateHandler) {}

        ModeConfig &currentMode()
        {
            static ModeConfig offConfig = OffConfig();
            if (current_mode_index >= 0 && current_mode_index < eeprom::MODE_COUNT)
            {
                return modes[current_mode_index];
            }
            return offConfig;
        }
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
                    Serial.println("init currentMode " + init.currentMode().index());
                    return init.currentMode();
                },
                [](Initialized &init)
                {
                    return init.currentMode();
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
            create(WordClockHandler::NAME, init.modes[0], 0);
            create(DigiClockHandler::NAME, init.modes[1], 1);
            for (int i = 2; i < eeprom::MODE_COUNT; i++)
            {
                init.modes[i].emplace<Empty>().index = i;
            }
            init.current_mode_index = 0;
        }
        else
        {
            for (int i = 0; i < eeprom::MODE_COUNT; i++)
            {
                unpackModeConfig(init.eepromConfig.modes, i, init.modes[i]);
            }

            init.current_mode_index = init.eepromConfig.startMode;
            if (std::holds_alternative<Empty>(init.currentMode()))
            {
                init.current_mode_index = 0;
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
                                Serial.printf("handle %s (%d)\n", uri.c_str(), moduleState.index());
                                if (handler(init))
                                {
                                    Serial.printf("3. switched to mode %s\n", config::modeName(init.currentMode()).c_str());
                                    updateHandler(init.currentMode());
                                }
                            },
                            [&server, &uri](auto &init)
                            {
                                Serial.printf("error %s (%d)\r", uri.c_str(), moduleState.index());
                                server.send(400, "application/json", "{}");
                            }},
                        moduleState); });
    }

    void EmptyTypeHandler::toJson(const Empty &config, JsonObject current)
    {
        current[F("type")] = NAME;
        current[F("index")] = config.index;
    }

    void EmptyTypeHandler::fromJson(Empty &config, JsonObjectConst current)
    {
        config.index = current[F("index")];
    }

    void EmptyTypeHandler::init(Empty &config, int index, const BaseConfig *old)
    {
        config.index = index;
    }

    void OffTypeHandler::toJson(const OffConfig &config, JsonObject current)
    {
        current[F("type")] = NAME;
        current[F("index")] = OffConfig::MODE_OFF_INDEX;
    }

    void OffTypeHandler::fromJson(OffConfig &config, JsonObjectConst current)
    {
    }

    void OffTypeHandler::init(OffConfig &config, int index, const BaseConfig *old)
    {
    }

    void baseConfigInit(BaseConfig &config, int index, const BaseConfig *old, const char *defaultName)
    {
        config.index = index;
        if (old)
        {
            config.name = old->name;
            config.color = old->color;
            config.brightness = old->brightness;
        }
        else
        {
            config.name = defaultName;
            config.color = 0xfff;
            config.brightness = 50;
        }
    }

    void baseConfigToJson(const BaseConfig &config, JsonObject current)
    {
        current[F("index")] = config.index;
        current[F("name")] = config.name;
        current[F("color")] = config.color;
        current[F("brightness")] = config.brightness;
    }

    void baseConfigFromJson(BaseConfig &config, JsonObjectConst doc)
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
    }

    void WordClockHandler::toJson(const WordClockConfig &config, JsonObject current)
    {
        current[F("type")] = NAME;
        baseConfigToJson(config, current);

        auto configArray = current[F("config")].to<JsonArray>();
        for (int c = 0; c < 12; c++)
        {
            configArray.add(config.config[c]);
        }
        current[F("fixed")] = config.fixed;
        current[F("hours")] = config.hours;
        current[F("minutes")] = config.minutes;
    }

    void WordClockHandler::fromJson(WordClockConfig &config, JsonObjectConst doc)
    {
        baseConfigFromJson(config, doc);

        JsonVariantConst clockConfig = doc[F("config")];
        if (!clockConfig.isNull())
        {
            JsonArrayConst ar = clockConfig.as<JsonArrayConst>();
            for (int i = 0; i < 12; i++)
            {
                config.config[i] = ar[i];
            }
        }
        JsonVariantConst fixed = doc[F("fixed")];
        if (!fixed.isNull())
        {
            config.fixed = fixed.as<bool>();
        }
        JsonVariantConst hours = doc[F("hours")];
        if (!hours.isNull())
        {
            config.hours = hours.as<uint8_t>();
        }
        JsonVariantConst minutes = doc[F("minutes")];
        if (!minutes.isNull())
        {
            config.minutes = minutes.as<uint8_t>();
        }
    }

    void WordClockHandler::init(WordClockConfig &config, int index, const BaseConfig *old)
    {
        baseConfigInit(config, index, old, NAME);
    }

    void DigiClockHandler::toJson(const DigiClockConfig &config, JsonObject current)
    {
        current[F("type")] = NAME;
        baseConfigToJson(config, current);
    }

    void DigiClockHandler::fromJson(DigiClockConfig &config, JsonObjectConst current)
    {
        baseConfigFromJson(config, current);
    }

    void DigiClockHandler::init(DigiClockConfig &config, int index, const BaseConfig *old)
    {
        baseConfigInit(config, index, old, NAME);
    }

    bool onGetCurrent(ESP8266WebServer &server, UDPLogger &logger, Initialized &init)
    {

        JsonDocument json;
        JsonObject current = json["current"].to<JsonObject>();
        toJson(init.currentMode(), current);

        auto modes = json["modes"].to<JsonArray>();
        for (int i = 0; i < eeprom::MODE_COUNT; i++)
        {
            auto mode = modes.add<JsonObject>();
            mode[F("index")] = i;
            mode[F("type")] = modeName(init.modes[i]);
            if(auto baseConfig = toBaseConfig(init.modes[i]))
            {
                 mode[F("name")] = baseConfig ->name;
            }
        }

        String message;
        serializeJsonPretty(json, message);

        server.send(200, "application/json", message);
        return false;
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
        int currentModeIndex = init.current_mode_index;
        if (currentModeIndex >= 0)
        {
            const char *typeCstr = doc[F("type")];
            if (typeCstr)
            {
                create(typeCstr, init.currentMode(), currentModeIndex);                                
            }

            fromJson(init.currentMode(), doc.to<JsonObject>());

            notifyUpdateHandler = true;
        }
        doc.clear();
        toJson(init.currentMode(), doc.to<JsonObject>());
        String message;
        serializeJsonPretty(doc, message);
        server.send(200, "application/json", message);
        if (notifyUpdateHandler)
        {
            Serial.printf("1. switched to mode %s\n", config::modeName(init.currentMode()).c_str());
        }
        else
        {
            Serial.printf("1. stayed at mode %s\n", config::modeName(init.currentMode()).c_str());
        }
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
        if (mode >= 0 && mode < eeprom::MODE_COUNT)
        {
            init.current_mode_index = mode;
            notifyUpdateHandler = true;
        }
        else if (mode == OffConfig::MODE_OFF_INDEX)
        {
            init.current_mode_index = mode;
            notifyUpdateHandler = true;
        }

        doc.clear();
        toJson(init.currentMode(), doc.to<JsonObject>());
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
