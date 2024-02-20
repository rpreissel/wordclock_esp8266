#ifndef STATE_H
#define STATE_H

#include <ESP8266WebServer.h>
#include <ArduinoJson.h>

#include <type_traits>
#include <variant>
#include "tools.h"
#include "udplogger.h"

namespace config
{

    template <typename TModeType>
    struct _handler_instance
    {
        using handler_type = typename TModeType::handler_type;
        using type = _handler_instance<TModeType>;
        static typename TModeType::handler_type handler;
    };

    template <typename TModeType>
    typename _handler_instance<TModeType>::handler_type _handler_instance<TModeType>::handler;

    struct BaseConfig;

    template <typename TModeType>
    struct ModeTypeHandler
    {
        virtual void init(TModeType &modeConfig, int index, const BaseConfig *old) = 0;
        virtual void toJson(const TModeType &modeConfig, JsonObject doc) = 0;
        virtual void fromJson(TModeType &modeConfig, JsonObjectConst doc) = 0;
    };

    struct BaseConfig
    {
        uint8_t index;
        uint8_t brightness;
        uint32_t color;
        String name;
    };

    struct Empty;

    struct EmptyTypeHandler : ModeTypeHandler<Empty>
    {
        constexpr static const char *NAME = "EMPTY";
        virtual void init(Empty &modeConfig, int index, const BaseConfig *old) override;
        virtual void toJson(const Empty &modeConfig, JsonObject doc) override;
        virtual void fromJson(Empty &modeConfig, JsonObjectConst doc) override;
    };

    struct Empty
    {
        using handler_type = EmptyTypeHandler;
        uint8_t index;
    };

    struct OffConfig;

    struct OffTypeHandler : ModeTypeHandler<OffConfig>
    {
        constexpr static const char *const NAME = "OFF";
        virtual void init(OffConfig &modeConfig, int index, const BaseConfig *old) override;
        virtual void toJson(const OffConfig &modeConfig, JsonObject doc) override;
        virtual void fromJson(OffConfig &modeConfig, JsonObjectConst doc) override;
    };

    struct OffConfig
    {
        using handler_type = OffTypeHandler;
        constexpr static int MODE_OFF_INDEX = -1;
    };

    struct WordClockConfig;

    struct WordClockHandler : ModeTypeHandler<WordClockConfig>
    {
        constexpr static const char *const NAME = "WORDCLOCK";
        virtual void init(WordClockConfig &modeConfig, int index, const BaseConfig *old) override;
        virtual void toJson(const WordClockConfig &modeConfig, JsonObject doc) override;
        virtual void fromJson(WordClockConfig &modeConfig, JsonObjectConst doc) override;
    };
    struct WordClockConfig : BaseConfig
    {
        using handler_type = WordClockHandler;
        uint8_t config[12] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
        bool fixed;
        uint8_t hours;
        uint8_t minutes;
    };

    struct DigiClockConfig;

    struct DigiClockHandler : ModeTypeHandler<DigiClockConfig>
    {
        constexpr static const char *const NAME = "DIGICLOCK";
        virtual void init(DigiClockConfig &modeConfig, int index, const BaseConfig *old) override;
        virtual void toJson(const DigiClockConfig &modeConfig, JsonObject doc) override;
        virtual void fromJson(DigiClockConfig &modeConfig, JsonObjectConst doc) override;
    };

    struct DigiClockConfig : BaseConfig
    {
        using handler_type = DigiClockHandler;
    };

    template <typename... Args>
    void toJson(std::variant<Args...> &para, JsonObject doc)
    {
        return std::visit(Overload{[doc](Args &mt)
                                   { _handler_instance<Args>::handler.toJson(mt, doc); }...},
                          para);
    }

    template <typename... Args>
    void fromJson(std::variant<Args...> &para, JsonObject doc)
    {
        return std::visit(Overload{[doc](Args &mt)
                                   { _handler_instance<Args>::handler.fromJson(mt, doc); }...},
                          para);
    }

    template <typename... Args>
    String modeName(const std::variant<Args...> &para)
    {
        return std::visit(Overload{[](const Args &mt)
                                   { return String(Args::handler_type::NAME); }...},
                          para);
    }

    template <typename... Args>
    const BaseConfig* toBaseConfig(const std::variant<Args...> &para)
    {
        return std::visit(Overload{[](const Args &mt) -> const BaseConfig*
                                   {
                                    if constexpr (std::is_base_of<BaseConfig, Args>()) {
                                        return &mt; 
                                    }
                                    return nullptr;
                                    }...},
                          para);
    }

    typedef std::variant<Empty, OffConfig, WordClockConfig, DigiClockConfig> ModeConfig;

    template <std::size_t I = 0>
    void create(String typeName, ModeConfig &current, int index)
    {
        if constexpr (I < std::variant_size_v<ModeConfig>)
        {
            if (typeName.compareTo(std::variant_alternative_t<I, ModeConfig>::handler_type::NAME) == 0)
            {
                const BaseConfig* old = toBaseConfig(current);
                auto newMode = std::variant_alternative_t<I, ModeConfig>();                 
                _handler_instance<std::variant_alternative_t<I, ModeConfig>>::handler.init(newMode, index, old);
                current = newMode;
                return;
            }
            create<I + 1>(typeName, current, index);
            return;
        }
        create<0>(EmptyTypeHandler::NAME,current, index);
    }

    typedef std::function<void(const ModeConfig &)> UpdateHandler;
    ModeConfig init(ESP8266WebServer &server, UDPLogger &logger, UpdateHandler updateHandler);

}
#endif