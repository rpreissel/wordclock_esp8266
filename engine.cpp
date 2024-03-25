#include <variant>
#include <tuple>
#include <EEPROM.h> //from ESP8266 Arduino Core (automatically installed when ESP8266 was installed via Boardmanager)
#include "engine.h"
#include "tools.h"
#include "wordclock.h"
#include "digiclock.h"
#include "automode.h"
#include "picture.h"

namespace eeprom
{
    constexpr uint8_t MODE_COUNT = 16;
    constexpr uint8_t NAMES_BUFFER = MODE_COUNT * 9;
    constexpr uint8_t CONFIG_COUNT = 64;
    constexpr uint8_t INIT_MARKER = 0x42;

    struct ModeConfig
    {
        // 4 bits for type + config data count
        // 0001 = wordclock
        // 0010 = digiclock
        uint8_t type = 0b0; // 4bits type, 4 bits config
        uint8_t brightness = 0;
        uint8_t colorIndex = 0;
    };

    struct EEPROMConfig
    {

        uint8_t initMarker;
        uint8_t startMode = 0;
        ModeConfig modes[MODE_COUNT];
        char names[NAMES_BUFFER + 1] = {0};
        uint64_t configs[CONFIG_COUNT];
    };
}

namespace modes
{
    using EEPROMModeConfig = std::variant<Empty, wordclock::WordClockConfig, digiclock::DigiClockConfig, automode::TimerModeConfig, automode::IntervalModeConfig, picture::PictureConfig>;
    using ModeConfig = concatenator<EEPROMModeConfig, OffConfig>::type;
    constexpr int EMPTY_MODE_INDEX = -16;

    template <typename C, typename... Args>
    const C *castToConfig(const std::variant<Args...> &para)
    {
        return std::visit(
            Overload{[](const Args &mt) -> const C *
                     {
                         if constexpr (std::is_base_of<C, Args>())
                         {
                             return &mt;
                         }

                         return nullptr;
                     }...},
            para);
    }

    template <typename C, typename... Args>
    C *castToConfig(std::variant<Args...> &para)
    {
        return std::visit(
            Overload{[](Args &mt) -> C *
                     {
                         if constexpr (std::is_base_of<C, Args>())
                         {
                             return &mt;
                         }

                         return nullptr;
                     }...},
            para);
    }

    template <typename... Args>
    String modeType(const std::variant<Args...> &para)
    {
        return std::visit(Overload{[](const Args &mt)
                                   { return String(Args::handler_type::TYPE); }...},
                          para);
    }

    template <typename... Args>
    String modeName(const std::variant<Args...> &para)
    {
        String type = modeType(para);
        const NameConfig *nameConfig = castToConfig<NameConfig>(para);
        if (nameConfig)
        {
            return nameConfig->name + F(" (") + type + F(")");
        }
        return type;
    }

    template <typename... Args>
    void onActivate(std::variant<Args...> &para, Env &env)
    {
        env.logger.logFormatted(F("OnActivate %d"), para.index());
        return std::visit(Overload{[&](Args &mt)
                                   { _handler_instance<Args>::handler.onActivate(mt, env); }...},
                          para);
    }

    template <typename... Args>
    uint16_t onLoop(std::variant<Args...> &para, Env &env, unsigned long millis)
    {
        return std::visit(Overload{[&](Args &mt)
                                   { return _handler_instance<Args>::handler.onLoop(mt, env, millis); }...},
                          para);
    }

    struct Error
    {
    };

    struct Initialized
    {
        ESP8266WebServer &server;
        Env env;
        ResetCallback resetCallback;

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
            if (activationLevel >= eeprom::MODE_COUNT)
            {
                env.logger.logFormatted(F("Too many recursions (%d)"), index);
                return;
            }
            activatedModeIndexes[activationLevel] = index;
            env.logger.logFormatted(F("Activate Mode (%d/%d)"), activationLevel, index);
            activationLevel++;
            for (uint8_t i = activationLevel; i < eeprom::MODE_COUNT; i++)
            {
                activatedModeIndexes[i] = EMPTY_MODE_INDEX;
            }

            onActivate(mode(index), env);
            activationLevel--;
        }

        uint32_t loopNextMode(unsigned long millis)
        {
            if (activationLevel >= eeprom::MODE_COUNT)
            {
                env.logger.logFormatted(F("Too many recursions (%d)"), index);
                return 0;
            }
            auto result = onLoop(mode(activatedModeIndexes[activationLevel++]), env, millis);
            activationLevel--;
            return result;
        }

        Initialized(ESP8266WebServer &server,
                    LEDMatrix &ledmatrix,
                    UDPLogger &logger,
                    NTPClientPlus &ntp,
                    ResetCallback resetCallback)
            : server(server),
              resetCallback(resetCallback),
              activatedModeIndexes{EMPTY_MODE_INDEX},
              env{
                  ledmatrix,
                  logger,
                  ntp,
                  [this](int index)
                  { activateMode(index); },
                  [this](unsigned long millis)
                  {
                      return loopNextMode(millis);
                  }} {}
    };

    std::variant<std::monostate, Initialized, Error> moduleState;

    void currentModeDescription(String &desc)
    {
        desc.clear();
        return std::visit(
            Overload{
                [&](Initialized &init)
                {
                    for (size_t i = 0; i < eeprom::MODE_COUNT; i++)
                    {
                        if (init.activatedModeIndexes[i] != EMPTY_MODE_INDEX)
                        {
                            if (i)
                            {
                                desc.concat(F(" / "));
                            }

                            desc.concat(modeName(init.mode(init.activatedModeIndexes[i])));
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

    template <typename... Args>
    void fromConfig(std::variant<Args...> &para, Env &env, const uint64_t config[], const uint8_t usedConfigs)
    {
        return std::visit(Overload{[config, usedConfigs, &env](Args &mt)
                                   {
                                       _handler_instance<Args>::handler.fromConfig(mt, env, config, usedConfigs);
                                   }...},
                          para);
    }

    template <typename... Args>
    uint8_t toConfig(std::variant<Args...> &para, Env &env, uint64_t config[], const uint8_t emptyConfigs)
    {
        return std::visit(Overload{[config, emptyConfigs, &env](Args &mt)
                                   {
                                       return _handler_instance<Args>::handler.toConfig(mt, env, config, emptyConfigs);
                                   }...},
                          para);
    }
    template <std::size_t I>
    void reInit(ModeConfig &current, Env &env)
    {
        auto newMode = std::variant_alternative_t<I, EEPROMModeConfig>();
        _handler_instance<std::variant_alternative_t<I, EEPROMModeConfig>>::handler.init(newMode, env);
        current = newMode;
        NameConfig *nameConfig = castToConfig<NameConfig>(current);
        if (nameConfig)
        {
            nameConfig->name = modeType(current);
        }
        ColorConfig *colorConfig = castToConfig<ColorConfig>(current);
        if (colorConfig)
        {
            colorConfig->colorIndex = 1;
            colorConfig->brightness = 50;
        }
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

    void init(ESP8266WebServer &server,
              LEDMatrix &ledmatrix,
              UDPLogger &logger,
              NTPClientPlus &ntp,
              ResetCallback resetCallback)
    {
        initServerEndpoints(server);
        std::visit(
            Overload{
                [&](std::monostate &empty)
                {
                    logger.logFormatted(F("init first time"));
                    // Init EEPROM
                    EEPROM.begin(sizeof(eeprom::EEPROMConfig));

                    auto &init = moduleState.emplace<Initialized>(server, ledmatrix, logger, ntp, resetCallback);
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

    void initEEProm(Initialized &init)
    {
        for (int i = 0; i < eeprom::MODE_COUNT; i++)
        {
            init.modes[i].emplace<Empty>();
        }
        reInit(wordclock::WordClockHandler::TYPE, init.env, init.modes[0]);
        reInit(digiclock::DigiClockHandler::TYPE, init.env, init.modes[1]);
        reInit(automode::TimerModeHandler::TYPE, init.env, init.modes[2]);
        reInit(automode::IntervalModeHandler::TYPE, init.env, init.modes[3]);

        init.eepromConfig.initMarker = eeprom::INIT_MARKER;
        for (int i = 0; i < eeprom::MODE_COUNT; i++)
        {
            auto &mode = init.eepromConfig.modes[i];
            mode.type = 0;
            mode.colorIndex = 0;
            init.eepromConfig.names[i] = 0;
        }
        for (int i = 0; i < eeprom::CONFIG_COUNT; i++)
        {
            init.eepromConfig.configs[i] = 0;
        }

        init.eepromConfig.startMode = 0;
    }

    int loadFromEEProm(Initialized &init)
    {
        EEPROM.get(0, init.eepromConfig);
        if (init.eepromConfig.initMarker != eeprom::INIT_MARKER)
        {
            initEEProm(init);
        }
        else
        {
            uint8_t nextConfig = 0;
            uint8_t nameStartIndex = 0;
            for (int i = 0; i < eeprom::MODE_COUNT; i++)
            {
                const auto &eeprommode = init.eepromConfig.modes[i];
                auto &mode = init.modes[i];
                auto type = eeprommode.type & 0xF;
                const char *name = &init.eepromConfig.names[nameStartIndex];
                auto usedConfigs = (eeprommode.type >> 4);

                init.env.logger.logFormatted(F("Start load: index: %d type: %d configs: %d name: %s"), i, type, usedConfigs, name);
                reInit(type, init.env, mode);
                init.env.logger.logFormatted(F("After reinit: index: %d modeindex: %d"), i, mode.index());
                NameConfig *nameConfig = castToConfig<NameConfig>(mode);
                ColorConfig *colorConfig = castToConfig<ColorConfig>(mode);
                if (nameConfig)
                {
                    if (strlen(name))
                    {
                        nameConfig->name = name;
                    }
                    else
                    {
                        nameConfig->name = modeType(mode);
                    }
                }
                if (colorConfig)
                {
                    colorConfig->brightness = eeprommode.brightness;
                    colorConfig->colorIndex = eeprommode.colorIndex;
                }
                fromConfig(mode, init.env, &init.eepromConfig.configs[nextConfig], usedConfigs);
                nextConfig += usedConfigs;
                nameStartIndex += (strlen(name) + 1);
            }

            if (init.modes[init.eepromConfig.startMode].index())
            {
                return init.eepromConfig.startMode;
            }
        }
        return 0;
    }

    void saveToEEProm(Initialized &init)
    {
        uint8_t nextConfig = 0;
        uint8_t nextNameIndex = 0;
        for (int i = 0; i < eeprom::MODE_COUNT; i++)
        {
            auto &eeprommode = init.eepromConfig.modes[i];
            auto &mode = init.modes[i];
            init.env.logger.logFormatted(F("Start save: %d-%d"), i, mode.index());

            const NameConfig *nameConfig = castToConfig<NameConfig>(mode);
            const ColorConfig *colorConfig = castToConfig<ColorConfig>(mode);
            if (nameConfig)
            {
                auto nameLength = nameConfig->name.length();
                auto modesPending = eeprom::MODE_COUNT - i;
                auto availableChars = eeprom::NAMES_BUFFER - nextNameIndex - modesPending;
                if (nameConfig->name == modeType(mode) || nameLength > availableChars)
                {
                    init.env.logger.logFormatted(F("No Name: %d-%s"), i, nameConfig->name.c_str());
                    init.eepromConfig.names[nextNameIndex] = 0;
                    nextNameIndex++;
                }
                else
                {
                    init.env.logger.logFormatted(F("Name: %d-%s"), i, nameConfig->name.c_str());
                    strcpy(&init.eepromConfig.names[nextNameIndex], nameConfig->name.c_str());
                    nextNameIndex += (nameLength + 1);
                }
            }
            if (colorConfig)
            {
                eeprommode.brightness = colorConfig->brightness;
                eeprommode.colorIndex = colorConfig->colorIndex;
            }
            uint8_t usedConfigs = toConfig(mode, init.env, &init.eepromConfig.configs[nextConfig], eeprom::CONFIG_COUNT - nextConfig);
            nextConfig += usedConfigs;
            eeprommode.type = (mode.index() & 0xF) | (usedConfigs << 4);
        }

        init.eepromConfig.startMode = init.activatedModeIndexes[0] < 0 ? 0 : init.activatedModeIndexes[0];
        EEPROM.put(0, init.eepromConfig);
        bool success = EEPROM.commit();
        init.env.logger.logFormatted(F("Flash EEPROM: Used configs: %d Success: %d Size: %d"), nextConfig, success, sizeof(init.eepromConfig));
        loadFromEEProm(init);
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

    template <std::size_t I = 0>
    void modeTypes(const char *typeNames[])
    {
        if constexpr (I < std::variant_size_v<ModeConfig>)
        {
            typeNames[I] = std::variant_alternative_t<I, ModeConfig>::handler_type::TYPE;
            modeTypes<I + 1>(typeNames);
        }
    }

    template <std::size_t I = 0>
    void modeConfigs(Env &env, JsonObject current)
    {
        if constexpr (I < std::variant_size_v<ModeConfig>)
        {
            _handler_instance<std::variant_alternative_t<I, ModeConfig>>::handler.configToJson(env, current);
            modeConfigs<I + 1>(env, current);
        }
    }

    template <typename... Args>
    void modeToJson(std::variant<Args...> &para, Env &env, JsonObject current)
    {
        return std::visit(Overload{[&para, current, &env](Args &mt)
                                   {
                                       current[F("type")] = Args::handler_type::TYPE;
                                       NameConfig *nameConfig = castToConfig<NameConfig>(para);
                                       ColorConfig *colorConfig = castToConfig<ColorConfig>(para);
                                       if (nameConfig)
                                       {
                                           current[F("name")] = nameConfig->name;
                                       }
                                       if (colorConfig)
                                       {
                                           current[F("color")] = colorName(colorConfig->colorIndex);
                                           current[F("brightness")] = colorConfig->brightness;
                                       }

                                       _handler_instance<Args>::handler.modeToJson(mt, env, current);
                                   }...},
                          para);
    }

    void toJson(Initialized &init, JsonObject doc)
    {
        JsonArray modes = doc[F("modes")].to<JsonArray>();
        for (int i = 0; i < eeprom::MODE_COUNT; i++)
        {
            JsonObject mode = modes.add<JsonObject>();
            mode[F("index")] = i;
            modeToJson(init.modes[i], init.env, mode);
        }
        doc[F("current")] = init.activatedModeIndexes[0];
        JsonObject time = doc["fixedTime"].to<JsonObject>();
        time[F("enabled")] = init.env.fixedTime();
        time[F("hours")] = init.env.fixedHours();
        time[F("minutes")] = init.env.fixedMinutes();
    }

    void onGetConfig(Initialized &init)
    {
        JsonDocument json;
        JsonObject config = json.to<JsonObject>();
        const char *typenames[std::variant_size_v<ModeConfig>];
        modeTypes(typenames);
        JsonArray types = config[F("types")].to<JsonArray>();
        for (int i = 0; i < std::variant_size_v<ModeConfig>; i++)
        {
            types.add(typenames[i]);
        }
        JsonObject colors = config[F("colors")].to<JsonObject>();
        for (int i = 0; i < NUM_COLORS; i++)
        {
            auto color = colors[String(i, HEX)].to<JsonObject>();
            color[F("name")] = std::get<0>(COLORS[i]);
            color[F("r")] = (std::get<1>(COLORS[i]) >> 16) & 0xFF;
            color[F("g")] = (std::get<1>(COLORS[i]) >> 8) & 0xFF;
            color[F("b")] = std::get<1>(COLORS[i]) & 0xFF;
        }

        JsonObject leds = config[F("leds")].to<JsonObject>();
        String textRow;
        textRow.reserve(LEDMatrix::width);

        for (int r = 0; r < LEDMatrix::height; r++)
        {
            textRow.clear();
            for (int c = 0; c < LEDMatrix::width; c++)
            {
                textRow.concat(LEDMatrix::clockStringUmlaut[r * 11 + c]);
            }

            leds[String(r, HEX)] = textRow;
        }

        leds[F("M")] = "   ----   ";

        modeConfigs(init.env, config);
        String message;
        serializeJsonPretty(json, message);

        init.server.send(200, "application/json", message);
    }

    void onGetLive(Initialized &init)
    {
        JsonDocument json;
        JsonArray modesJson = json[F("activemodes")].to<JsonArray>();
        for (uint8_t i; i < eeprom::MODE_COUNT; i++)
        {
            if (init.activatedModeIndexes[i] == EMPTY_MODE_INDEX)
            {
                break;
            }

            modesJson.add(init.activatedModeIndexes[i]);
        }

        JsonObject timeJson = json[F("time")].to<JsonObject>();
        timeJson[F("hours")] = init.env.hours();
        timeJson[F("minutes")] = init.env.minutes();

        JsonObject colorJson = json[F("colors")].to<JsonObject>();
        String colorRow;
        colorRow.reserve(LEDMatrix::width);

        for (int r = 0; r < LEDMatrix::height; r++)
        {
            colorRow.clear();
            for (int c = 0; c < LEDMatrix::width; c++)
            {
                uint8_t color = init.env.ledmatrix.colorIndexGrid(c, r);
                if (color)
                {
                    colorRow.concat(String(color, HEX));
                }
                else
                {
                    colorRow.concat(" ");
                }
            }

            colorJson[String(r, HEX)] = colorRow;
        }

        colorRow.clear();
        for (int m = 3; m >= 0; m--)
        {
            uint8_t color = init.env.ledmatrix.colorIndexMinIndicator(m);
            if (color)
            {
                colorRow.concat(String(color, HEX));
            }
            else
            {
                colorRow.concat(" ");
            }
        }
        colorJson[F("M")] = colorRow;
        String message;
        serializeJsonPretty(json, message);

        init.server.send(200, "application/json", message);
    }

    void onGet(Initialized &init)
    {
        init.env.logger.logFormatted(F("On Get (%d)"), init.activatedModeIndexes);
        JsonDocument json;
        JsonObject current = json.to<JsonObject>();
        toJson(init, current);

        String message;
        serializeJsonPretty(json, message);

        init.server.send(200, "application/json", message);
    }

    template <typename... Args>
    void modeFromJson(std::variant<Args...> &para, Env &env, JsonObjectConst data)
    {
        return std::visit(Overload{[&para, data, &env](Args &mt)
                                   {
                                       ColorConfig *baseConfig = castToConfig<ColorConfig>(para);
                                       if (baseConfig)
                                       {
                                           JsonVariantConst brightness = data[F("brightness")];
                                           if (!brightness.isNull())
                                           {
                                               baseConfig->brightness = brightness.as<int>();
                                           }
                                           JsonVariantConst colorJson = data[F("color")];
                                           if (!colorJson.isNull())
                                           {
                                               baseConfig->colorIndex = colorIndex(colorJson.as<const char *>());
                                           }
                                       }
                                       NameConfig *nameConfig = castToConfig<NameConfig>(para);
                                       if (nameConfig)
                                       {
                                           const char *name = data[F("name")];
                                           if (name)
                                           {
                                               nameConfig->name = name;
                                           }
                                       }

                                       _handler_instance<Args>::handler.modeFromJson(mt, env, data);
                                   }...},
                          para);
    }

    void onChange(Initialized &init)
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

        JsonVariantConst fixedTimeVariant = doc[F("fixedTime")];
        if (!fixedTimeVariant.isNull())
        {
            JsonObjectConst fixedTimeJson = fixedTimeVariant.as<JsonObjectConst>();
            init.env.fixedTime(fixedTimeJson[F("enabled")]);
            init.env.fixedHours(fixedTimeJson[F("hours")]);
            init.env.fixedMinutes(fixedTimeJson[F("minutes")]);
        }
        JsonVariantConst modesVariant = doc[F("modes")];
        if (!modesVariant.isNull())
        {
            JsonArrayConst modesArray = modesVariant.as<JsonArrayConst>();
            for (JsonVariantConst modeVariant : modesArray)
            {
                JsonObjectConst modeJson = modeVariant.as<JsonObjectConst>();
                JsonVariantConst indexVariant = modeJson[F("index")];
                if (indexVariant.isNull())
                {
                    continue;
                }
                int index = indexVariant.as<int>();
                if (index < 0 || index >= eeprom::MODE_COUNT)
                {
                    continue;
                }
                const char *typeCstr = modeJson[F("type")];
                if (typeCstr && modeType(init.modes[index]).compareTo(typeCstr) != 0)
                {
                    init.env.logger.logFormatted(F("before reinit %d / %d"), index, init.modes[index].index());
                    reInit(typeCstr, init.env, init.modes[index]);
                    init.env.logger.logFormatted(F("after reinit %d / %d"), index, init.modes[index].index());
                }
                init.env.logger.logFormatted(F("modeFromJson %d / %d"), index, init.modes[index].index());
                modeFromJson(init.modes[index], init.env, modeJson);
            }
        }

        JsonVariantConst currentVariant = doc[F("current")];
        if (currentVariant.isNull())
        {
            activateRootMode(init, init.activatedModeIndexes[0]);
        }
        else
        {
            int current = currentVariant.as<int>();
            if (current >= 0 && current < eeprom::MODE_COUNT || current == OffConfig::MODE_OFF_INDEX)
            {
                activateRootMode(init, current);
            }
            else
            {
                activateRootMode(init, init.activatedModeIndexes[0]);
            }
        }

        JsonVariantConst flash = doc[F("flash")];
        if (!flash.isNull() && flash.as<bool>())
        {
            saveToEEProm(init);
        }
        doc.clear();
        toJson(init, doc.to<JsonObject>());
        String message;
        serializeJsonPretty(doc, message);
        init.server.send(200, "application/json", message);
    }

    void onReset(Initialized &init)
    {
        init.env.logger.logFormatted(F("On Reset (%d)"), init.activatedModeIndexes[0]);
        auto text = init.server.arg("plain");
        JsonDocument doc;
        DeserializationError error = deserializeJson(doc, text);
        if (error)
        {
            Serial.println(F("deserializeJson() failed: "));
            init.server.send(400, "text/plain", error.c_str());
            return;
        }

        uint8_t resetFlags = NOTHING;
        JsonVariantConst wifiVariant = doc[F("wifi")];
        if (!wifiVariant.isNull() && wifiVariant.as<bool>())
        {
            resetFlags |= WIFI | ESP;
        }
        JsonVariantConst eepromVariant = doc[F("eeprom")];
        if (!eepromVariant.isNull() && eepromVariant.as<bool>())
        {
            initEEProm(init);
            activateRootMode(init,0);
            saveToEEProm(init);
            resetFlags |= ESP;
        }
        init.resetCallback(resetFlags);
        doc.clear();
        toJson(init, doc.to<JsonObject>());
        String message;
        serializeJsonPretty(doc, message);
        init.server.send(200, "application/json", message);
    }

    void initServerEndpoints(ESP8266WebServer &server)
    {
        on(server, "/api/modes", HTTP_GET, onGet);
        on(server, "/api/modes", HTTP_PATCH, onChange);
        on(server, "/api/reset", HTTP_PATCH, onReset);
        on(server, "/api/configs", HTTP_GET, onGetConfig);
        on(server, "/api/live", HTTP_GET, onGetLive);
    }
}
