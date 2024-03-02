#include <variant>
#include <tuple>
#include <EEPROM.h> //from ESP8266 Arduino Core (automatically installed when ESP8266 was installed via Boardmanager)
#include "engine.h"
#include "tools.h"
#include "wordclock.h"
#include "digiclock.h"

namespace eeprom
{
    constexpr uint8_t MODE_COUNT = 16;
    constexpr uint8_t CONFIG_COUNT = 64;
    constexpr uint8_t INIT_MARKER = 0x43;

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
    using EEPROMModeConfig = std::variant<Empty, wordclock::WordClockConfig, digiclock::DigiClockConfig>;
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
            : server(server), env{ledmatrix, logger, ntp} {}

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
                [&](Initialized &init)
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
    void activateCurrent(Initialized &init);

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
                auto type = eeprommode.type && 0b1111;
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

            init.current_mode_index = init.eepromConfig.startMode;
            if (std::holds_alternative<Empty>(init.currentMode()))
            {
                init.current_mode_index = 0;
            }
        }
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

        init.eepromConfig.startMode = init.current_mode_index < 0 ? 0 : init.current_mode_index;
        EEPROM.put(0, init.eepromConfig);
        EEPROM.commit();
        init.env.logger.logFormatted(F("Flash EEPROM: Used configs: %d"), nextConfig);
    }

    void activateCurrent(Initialized &init)
    {
        init.env.logger.logFormatted(F("Activate Mode (%d)"), init.current_mode_index);
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
        init.env.logger.logFormatted(F("On Get (%d)"), init.current_mode_index);
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
        init.env.logger.logFormatted(F("On Change (%d)"), init.current_mode_index);
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

            JsonVariantConst data = doc[F("data")];
            if (!data.isNull())
            {
                fromJson(init.currentMode(), init.env, data.as<JsonObjectConst>());
            }

            activateCurrent(init);

            JsonVariantConst flash = doc[F("flash")];
            if (!flash.isNull() && flash.as<bool>())
            {
                saveToEEProm(init);
            }
        }
        doc.clear();
        toJson(init.currentMode(), init.env, init.current_mode_index, doc.to<JsonObject>());
        String message;
        serializeJsonPretty(doc, message);
        init.server.send(200, "application/json", message);
    }

    void onChangeMode(Initialized &init)
    {
        init.env.logger.logFormatted(F("On Change Mode (%d)"), init.current_mode_index);
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
        saveToEEProm(init);

        doc.clear();
        toJson(init.currentMode(), init.env, init.current_mode_index, doc.to<JsonObject>());
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
