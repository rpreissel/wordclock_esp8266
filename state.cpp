#include <variant>
#include <tuple>
#include <EEPROM.h> //from ESP8266 Arduino Core (automatically installed when ESP8266 was installed via Boardmanager)
#include "state.h"
#include "tools.h"
#include "timedef.h"
#include "baseconfig.h"
#include "wordclock.h"
#include "digiclock.h"

namespace eeprom
{
    constexpr uint8_t MODE_COUNT = 8;
    constexpr uint8_t INIT_MARKER = 0x42;

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
    using EEPROMModeConfig = std::variant<config::Empty, wordclock::WordClockConfig, digiclock::DigiClockConfig>;
    using ModeConfig = concatenator<EEPROMModeConfig, OffConfig>::type;
    
    struct Error
    {
    };

    struct Initialized
    {
        ESP8266WebServer &server;
        Env env;

        eeprom::EEPROMConfig eepromConfig;
        ModeConfig modes[eeprom::MODE_COUNT];
        int current_mode_index;
        unsigned long lastAnimationStep;
        uint32_t animationTime;

        Initialized(ESP8266WebServer &server, LEDMatrix &ledmatrix, UDPLogger &logger, NTPClientPlus &ntp)
            : server(server), env{ledmatrix, logger, ntp}{}

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

    String currentModeDescription() 
    {
        return std::visit(
            Overload{
                [&](Initialized& init)
                {
                    return modeName(init.currentMode());
                },
                [&](auto &init)
                {
                    return String(F("Not intialized"));
                }},
            moduleState);
    }

    void loadFromEEProm(Initialized &init);
    void initServerEndpoints(ESP8266WebServer &server);
    void unpackModeConfig(Env& env,const eeprom::ModeConfig modeConfigs[], uint8_t index, ModeConfig &config);
    void activateCurrent(Initialized &init);

    template <std::size_t I>
    void reInit(ModeConfig &current,Env& env)
    {
        const BaseConfig *old = toBaseConfig(current);
        auto newMode = std::variant_alternative_t<I, EEPROMModeConfig>();
        _handler_instance<std::variant_alternative_t<I, EEPROMModeConfig>>::handler.init(newMode,env, old);
        current = newMode;
    }

    template <std::size_t I = 0>
    void reInit(String typeName,Env& env, ModeConfig &current)
    {
        if constexpr (I < std::variant_size_v<EEPROMModeConfig>)
        {
            if (typeName.compareTo(std::variant_alternative_t<I, EEPROMModeConfig>::handler_type::TYPE) == 0)
            {
                reInit<I>(current, env);
                return;
            }
            reInit<I + 1>(typeName, env, current);
            return;
        }
        reInit<0>(current, env);
    }

    template <std::size_t I = 0>
    void reInit(uint8_t typeIndex, Env& env, ModeConfig &current)
    {
        if constexpr (I < std::variant_size_v<EEPROMModeConfig>)
        {
            if (typeIndex == I)
            {
                reInit<I>(current, env);
                return;
            }
            reInit<I + 1>(typeIndex, env, current);
            return;
        }
        reInit<0>(current, env);
    }


    void init(ESP8266WebServer &server, LEDMatrix &ledmatrix, UDPLogger &logger, NTPClientPlus &ntp)
    {
        initServerEndpoints(server);
        std::visit(
            Overload{
                [&](std::monostate &empty)
                {
                    logger.logFormatted(F("init first time"));
                    // Init EEPROM
                    EEPROM.begin(sizeof(eeprom::EEPROMConfig));

                    auto &init = moduleState.emplace<Initialized>(server, ledmatrix, logger, ntp);
                    loadFromEEProm(init);
                    logger.logFormatted(F("start with mode: %d"), init.currentMode().index());
                    activateCurrent(init);
                },
                [&](auto &init)
                {
                    logger.logFormatted(F("ignore init"));
                }},
            moduleState);
    }

    void loadFromEEProm(Initialized &init)
    {
        EEPROM.get(0, init.eepromConfig);
        if (init.eepromConfig.initMarker != eeprom::INIT_MARKER)
        {
            for (int i = 0; i < eeprom::MODE_COUNT; i++)
            {
                init.modes[i].emplace<Empty>();
            }
            reInit(wordclock::WordClockHandler::TYPE, init.env, init.modes[0]);
            reInit(digiclock::DigiClockHandler::TYPE, init.env, init.modes[1]);
            init.current_mode_index = 0;
        }
        else
        {
            for (int i = 0; i < eeprom::MODE_COUNT; i++)
            {
                unpackModeConfig(init.env, init.eepromConfig.modes, i, init.modes[i]);
            }

            init.current_mode_index = init.eepromConfig.startMode;
            if (std::holds_alternative<Empty>(init.currentMode()))
            {
                init.current_mode_index = 0;
            }
        }
    }

    void unpackModeConfig(Env& env,const eeprom::ModeConfig modeConfigs[], uint8_t index, ModeConfig &config)
    {
        eeprom::ModeConfig modeConfig = modeConfigs[index];
        auto type = modeConfig.type && 0b1111;
        reInit(type, env, config);
        /*
                switch (mode)
                {
                case eeprom::Type::WORDCLOCK:
                {
                    auto &wordClockConfig = config.emplace<wordclock::WordClockConfig>();
                    uint32_t bitConfig = modeConfig.config[0] && 0b11'11'11'11'11'11'11'11'11'11'11'11;
                    for (int c = 0; c < 12; c++)
                    {
                        wordClockConfig.config[c] = (bitConfig >> (2 * c)) & 0x03;
                    }
                }
                break;
                case eeprom::Type::DIGICLOCK:
                    config.emplace<digiclock::DigiClockConfig>();
                    break;
                default:
                    config.emplace<Empty>();
                    break;
                }

                std::visit(
                    Overload{
                        [&modeConfig, index](BaseConfig &config)
                        {
                            config.name = modeConfig.name;
                            config.brightness = modeConfig.color >> 24;
                            config.color = modeConfig.color & 0x0fff;
                        },
                        [](auto &config) {

                        }},
                    config);
                    */
    }

    void activateCurrent(Initialized &init)
    {
        init.env.ledmatrix.gridFlush();
        init.animationTime = onActivate(init.currentMode(), init.env);
    }

    void loop(unsigned long millis)
    {
        std::visit(
            Overload{
                [=](Initialized &init)
                {
                    if (init.animationTime > 0 && millis - init.lastAnimationStep > init.animationTime)
                    {
                        init.animationTime = onLoop(init.currentMode(), init.env, millis);
                        init.lastAnimationStep = millis;
                    }
                },
                [](auto &init) {

                }},
            moduleState);
    }

    void on(ESP8266WebServer &server, const String &uri, HTTPMethod method, std::function<void(Initialized &)> handler)
    {
        server.on(uri, method, [handler, &server, &uri]()
                  { std::visit(
                        Overload{
                            [handler, &uri](Initialized &init)
                            {
                                init.env.logger.logFormatted(F("handle %s (%d)"), uri.c_str(), moduleState.index());
                                handler(init);
                            },
                            [&server, &uri](auto &init)
                            {
                                server.send(400, "application/json", "{}");
                            }},
                        moduleState); });
    }

    void onGetCurrent(Initialized &init)
    {

        JsonDocument json;
        JsonObject current = json["current"].to<JsonObject>();
        toJson(init.currentMode(), init.env, init.current_mode_index, current);

        auto modes = json["modes"].to<JsonArray>();
        for (int i = 0; i < eeprom::MODE_COUNT; i++)
        {
            auto mode = modes.add<JsonObject>();
            mode[F("index")] = i;
            mode[F("type")] = modeType(init.modes[i]);
            if (auto baseConfig = toBaseConfig(init.modes[i]))
            {
                mode[F("name")] = baseConfig->name;
            }
        }

        String message;
        serializeJsonPretty(json, message);

        init.server.send(200, "application/json", message);
    }

    void onChangeCurrent(Initialized &init)
    {
        auto text = init.server.arg("plain");
        JsonDocument doc;
        DeserializationError error = deserializeJson(doc, text);
        if (error)
        {
            Serial.println(F("deserializeJson() failed: "));
            init.server.send(400, "text/plain", error.c_str());
            return;
        }

        int currentModeIndex = init.current_mode_index;
        if (currentModeIndex >= 0)
        {
            const char *typeCstr = doc[F("type")];
            if (typeCstr)
            {
                reInit(typeCstr, init.env, init.currentMode());
            }

            fromJson(init.currentMode(), init.env, doc.as<JsonObjectConst>());
            activateCurrent(init);
        }
        doc.clear();
        toJson(init.currentMode(), init.env, init.current_mode_index, doc.to<JsonObject>());
        String message;
        serializeJsonPretty(doc, message);
        init.server.send(200, "application/json", message);
    }

    void onChangeMode(Initialized &init)
    {
        auto text = init.server.arg("plain");
        JsonDocument doc;
        DeserializationError error = deserializeJson(doc, text);
        if (error)
        {
            Serial.println(F("deserializeJson() failed: "));
            init.server.send(400, "text/plain", error.c_str());
            return;
        }

        int mode = doc["mode"];
        if (mode >= 0 && mode < eeprom::MODE_COUNT)
        {
            init.current_mode_index = mode;
        }
        else if (mode == OffConfig::MODE_OFF_INDEX)
        {
            init.current_mode_index = mode;
        }
        activateCurrent(init);

        doc.clear();
        toJson(init.currentMode(), init.env, init.current_mode_index, doc.to<JsonObject>());
        String message;
        serializeJsonPretty(doc, message);
        init.server.send(200, "application/json", message);
    }

    void onTimedefsGet(Initialized &init)
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

        init.server.send(200, "application/json", message);
    }

    void initServerEndpoints(ESP8266WebServer &server)
    {
        on(server, "/current", HTTP_GET, onGetCurrent);
        on(server, "/current", HTTP_PATCH, onChangeCurrent);
        on(server, "/mode", HTTP_PUT,  onChangeMode);
        on(server, "/timedefs", HTTP_GET, onTimedefsGet);
    }
}
