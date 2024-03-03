#include <variant>
#include <tuple>
#include <EEPROM.h> //from ESP8266 Arduino Core (automatically installed when ESP8266 was installed via Boardmanager)
#include "engine.h"
#include "tools.h"
#include "wordclock.h"
#include "digiclock.h"
#include "automode.h"

namespace eeprom
{
    constexpr uint8_t MODE_COUNT = 16;
    constexpr uint8_t CONFIG_COUNT = 64;
    constexpr uint8_t INIT_MARKER = 0x42;

    struct ModeConfig
    {
        // 4 bits for type + config data count
        // 0001 = wordclock
        // 0010 = digiclock
        uint8_t type = 0b0;
        // 8bit red, 8bit green, 8bit blue, 8 bit brightness
        uint32_t color = 0;

        char name[11] = {0};
    };

    struct EEPROMConfig
    {

        uint8_t initMarker;
        uint8_t startMode = 0;
        ModeConfig modes[MODE_COUNT];
        uint64_t configs[CONFIG_COUNT];
    };
}

namespace modes
{
    using EEPROMModeConfig = std::variant<Empty, wordclock::WordClockConfig, digiclock::DigiClockConfig, automode::TimerModeConfig>;
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
        uint8_t activationLevel;
        int activatedModeIndexes[eeprom::MODE_COUNT];
        unsigned long lastAnimationStep;
        uint32_t animationTime;


        ModeConfig &mode(int index)
        {
            static ModeConfig offConfig = OffConfig();
            if (index >= 0 && index < eeprom::MODE_COUNT)
            {
                return modes[index];
            }
            return offConfig;
        }

        ModeConfig &currentMode()
        {            
            return mode(activatedModeIndexes[0]);
        }

        void activateMode(int index)
        {
            if(activationLevel >= eeprom::MODE_COUNT)
            {
                env.logger.logFormatted(F("Too many recursions (%d)"), index);
                return;
            }
            activatedModeIndexes[activationLevel]=index;
            env.logger.logFormatted(F("Activate Mode (%d/%d)"),activationLevel, index);
            activationLevel++;
            for (uint8_t i = activationLevel; i < eeprom::MODE_COUNT; i++)
            {
                activatedModeIndexes[i] = -1;
            }
            
            onActivate(mode(index), env);
            activationLevel--;
        }

        uint32_t loopNextMode(unsigned long millis)
        {    
            if(activationLevel >= eeprom::MODE_COUNT)
            {
                env.logger.logFormatted(F("Too many recursions (%d)"), index);
                return 0;
            }        
            auto result = onLoop(mode(activatedModeIndexes[activationLevel++]), env, millis);
            activationLevel--;
            return result;
        }

        Initialized(ESP8266WebServer &server, LEDMatrix &ledmatrix, UDPLogger &logger, NTPClientPlus &ntp)
            : server(server), 
            activatedModeIndexes{-1},
            env{
                ledmatrix, 
                logger, 
                ntp,
                [this](uint8_t index){activateMode(index);},
                [this](unsigned long millis){
                    return loopNextMode(millis);
                }
                } {}

    };

    std::variant<std::monostate, Initialized, Error> moduleState;

    void currentModeDescription(String& desc)
    {
        desc.clear();
        return std::visit(
            Overload{
                [&](Initialized &init)
                {
                    for (size_t i = 0; i < eeprom::MODE_COUNT; i++)
                    {
                        if(init.activatedModeIndexes[i]>=0) 
                        {
                            if(i)
                            {
                                desc.concat(F(" / "));
                            }

                            desc.concat(modeName(init.modes[init.activatedModeIndexes[i]]));
                        }
                        else 
                        {
                            break;
                        }
                    }
                },
                [&](auto &init)
                {
                    desc.concat(F("Not intialized"));
                }},
            moduleState);
    }

    int loadFromEEProm(Initialized &init);
    void initServerEndpoints(ESP8266WebServer &server);
    void activateRootMode(Initialized &init, int index);

    template <std::size_t I>
    void reInit(ModeConfig &current, Env &env)
    {
        const BaseConfig *old = toBaseConfig(current);
        auto newMode = std::variant_alternative_t<I, EEPROMModeConfig>();
        _handler_instance<std::variant_alternative_t<I, EEPROMModeConfig>>::handler.init(newMode, env, old);
        current = newMode;
    }

    template <std::size_t I = 0>
    void reInit(String typeName, Env &env, ModeConfig &current)
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
    void reInit(uint8_t typeIndex, Env &env, ModeConfig &current)
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
                    auto startMode = loadFromEEProm(init);

                    logger.logFormatted(F("start with mode: %d"), startMode);
                    activateRootMode(init, startMode);
                },
                [&](auto &init)
                {
                    logger.logFormatted(F("ignore init"));
                }},
            moduleState);
    }

    int loadFromEEProm(Initialized &init)
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
            reInit(automode::TimerModeHandler::TYPE, init.env, init.modes[2]);

            init.eepromConfig.initMarker = eeprom::INIT_MARKER;
            for (int i = 0; i < eeprom::MODE_COUNT; i++)
            {
                auto &mode = init.eepromConfig.modes[i];
                mode.type = 0;
                mode.color = 0;
                mode.name[0] = 0;
            }
            for (int i = 0; i < eeprom::CONFIG_COUNT; i++)
            {
                init.eepromConfig.configs[i] = 0;
            }

            init.eepromConfig.startMode = 0;            
        }
        else
        {
            uint8_t nextConfig = 0;
            for (int i = 0; i < eeprom::MODE_COUNT; i++)
            {
                const auto &eeprommode = init.eepromConfig.modes[i];
                auto &mode = init.modes[i];
                auto type = eeprommode.type & 0b1111;
                auto usedConfigs = (eeprommode.type >> 4);
                reInit(type, init.env, mode);
                BaseConfig *baseconfig = toBaseConfig(mode);
                if (baseconfig)
                {
                    baseconfig->name = eeprommode.name;
                    baseconfig->brightness = eeprommode.color >> 24;
                    baseconfig->color = eeprommode.color & 0x0FFF;
                }
                fromConfig(mode, init.env, &init.eepromConfig.configs[nextConfig], usedConfigs);
                nextConfig+=usedConfigs;
            }

            if(init.modes[init.eepromConfig.startMode].index())
            {
                return init.eepromConfig.startMode;
            }
        }
        return 0;
    }

    void saveToEEProm(Initialized &init)
    {
        uint8_t nextConfig = 0;
        for (int i = 0; i < eeprom::MODE_COUNT; i++)
        {
            auto &eeprommode = init.eepromConfig.modes[i];
            auto &mode = init.modes[i];

            const BaseConfig *baseconfig = toBaseConfig(mode);
            if (baseconfig)
            {
                eeprommode.color = (baseconfig->brightness & 0x000F) << 24 | (baseconfig->color & 0x0FFF);
                strncpy(eeprommode.name, baseconfig->name.c_str(), 10);
            }
            uint8_t usedConfigs = toConfig(mode, init.env, &init.eepromConfig.configs[nextConfig], eeprom::CONFIG_COUNT - nextConfig);
            nextConfig += usedConfigs;
            eeprommode.type = (mode.index() & 0xF) | (usedConfigs << 4);
        }

        init.eepromConfig.startMode = init.activatedModeIndexes[0] < 0 ? 0 : init.activatedModeIndexes[0];
        EEPROM.put(0, init.eepromConfig);
        EEPROM.commit();
        init.env.logger.logFormatted(F("Flash EEPROM: Used configs: %d"), nextConfig);
    }

    void activateRootMode(Initialized &init, int index)
    {
        init.activateMode(index);
        init.animationTime = 1;
    }

    void loop(unsigned long millis)
    {
        std::visit(
            Overload{
                [=](Initialized &init)
                {
                    if (init.animationTime > 0 && millis - init.lastAnimationStep > init.animationTime)
                    {
                        init.animationTime = init.loopNextMode(millis);
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
        init.env.logger.logFormatted(F("On Get (%d)"), init.activatedModeIndexes);
        JsonDocument json;
        JsonObject current = json["current"].to<JsonObject>();
        toJson(init.currentMode(), init.env, init.activatedModeIndexes[0], current);

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
        init.env.logger.logFormatted(F("On Change (%d)"), init.activatedModeIndexes);
        auto text = init.server.arg("plain");
        JsonDocument doc;
        DeserializationError error = deserializeJson(doc, text);
        if (error)
        {
            Serial.println(F("deserializeJson() failed: "));
            init.server.send(400, "text/plain", error.c_str());
            return;
        }

        int currentModeIndex = init.activatedModeIndexes[0];
        if (currentModeIndex >= 0)
        {
            const char *typeCstr = doc[F("type")];
            if (typeCstr)
            {
                reInit(typeCstr, init.env, init.currentMode());
            }

            JsonVariantConst data = doc[F("data")];
            if (!data.isNull())
            {
                fromJson(init.currentMode(), init.env, data.as<JsonObjectConst>());
            }

            activateRootMode(init, currentModeIndex);

            JsonVariantConst flash = doc[F("flash")];
            if (!flash.isNull() && flash.as<bool>())
            {
                saveToEEProm(init);
            }
        }
        doc.clear();
        toJson(init.currentMode(), init.env, init.activatedModeIndexes[0], doc.to<JsonObject>());
        String message;
        serializeJsonPretty(doc, message);
        init.server.send(200, "application/json", message);
    }

    void onChangeMode(Initialized &init)
    {
        init.env.logger.logFormatted(F("On Change Mode (%d)"), init.activatedModeIndexes);
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
        if (mode >= 0 && mode < eeprom::MODE_COUNT || mode == OffConfig::MODE_OFF_INDEX)
        {
            activateRootMode(init, mode);
        }
        saveToEEProm(init);

        doc.clear();
        toJson(init.currentMode(), init.env, init.activatedModeIndexes[0], doc.to<JsonObject>());
        String message;
        serializeJsonPretty(doc, message);
        init.server.send(200, "application/json", message);
    }

    void initServerEndpoints(ESP8266WebServer &server)
    {
        on(server, "/current", HTTP_GET, onGetCurrent);
        on(server, "/current", HTTP_PATCH, onChangeCurrent);
        on(server, "/mode", HTTP_PUT, onChangeMode);
    }
}
