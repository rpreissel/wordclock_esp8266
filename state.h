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
        virtual void init(TModeType &modeConfig, const BaseConfig *old) = 0;
        virtual void toJson(const TModeType &modeConfig, JsonObject doc) = 0;
        virtual void fromJson(TModeType &modeConfig, JsonObjectConst doc) = 0;
    };

    struct BaseConfig
    {
        uint8_t brightness;
        uint32_t color;
        String name;
    };

    struct Empty;

    struct EmptyTypeHandler : ModeTypeHandler<Empty>
    {
        constexpr static const char *TYPE = "EMPTY";
        virtual void init(Empty &modeConfig, const BaseConfig *old) override;
        virtual void toJson(const Empty &modeConfig, JsonObject doc) override;
        virtual void fromJson(Empty &modeConfig, JsonObjectConst doc) override;
    };

    struct Empty
    {
        using handler_type = EmptyTypeHandler;
    };

    struct OffConfig;

    struct OffTypeHandler : ModeTypeHandler<OffConfig>
    {
        constexpr static const char *const TYPE = "OFF";
        virtual void init(OffConfig &modeConfig, const BaseConfig *old) override;
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
        constexpr static const char *const TYPE = "WORDCLOCK";
        virtual void init(WordClockConfig &modeConfig, const BaseConfig *old) override;
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
        constexpr static const char *const TYPE = "DIGICLOCK";
        virtual void init(DigiClockConfig &modeConfig, const BaseConfig *old) override;
        virtual void toJson(const DigiClockConfig &modeConfig, JsonObject doc) override;
        virtual void fromJson(DigiClockConfig &modeConfig, JsonObjectConst doc) override;
    };

    struct DigiClockConfig : BaseConfig
    {
        using handler_type = DigiClockHandler;
    };

    template <typename... Args>
    void toJson(std::variant<Args...> &para, int index, JsonObject doc)
    {
        doc[F("index")] = index;
        return std::visit(Overload{[doc](Args &mt)
                                   { _handler_instance<Args>::handler.toJson(mt, doc); }...},
                          para);
    }

    template <typename... Args>
    void fromJson(std::variant<Args...> &para, JsonObjectConst doc)
    {
        return std::visit(Overload{[doc](Args &mt)
                                   { _handler_instance<Args>::handler.fromJson(mt, doc); }...},
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
    const BaseConfig *toBaseConfig(const std::variant<Args...> &para)
    {
        return std::visit(Overload{[](const Args &mt) -> const BaseConfig *
                                   {
                                       if constexpr (std::is_base_of<BaseConfig, Args>())
                                       {
                                           return &mt;
                                       }
                                       return nullptr;
                                   }...},
                          para);
    }

    template <typename... Args>
    String modeName(const std::variant<Args...> &para)
    {
        String type = modeType(para);
        const BaseConfig* baseConfig = toBaseConfig(para);
        if(baseConfig) {
            return baseConfig->name + F(" (") + type + F(")");
        }
        return type;
    }

    using EEPROMModeConfig = std::variant<Empty, WordClockConfig, DigiClockConfig>;
    using ModeConfig = concatenator<EEPROMModeConfig, OffConfig>::type;

    template <std::size_t I>
    void reInit(ModeConfig &current)
    {
        const BaseConfig *old = toBaseConfig(current);
        auto newMode = std::variant_alternative_t<I, EEPROMModeConfig>();
        _handler_instance<std::variant_alternative_t<I, EEPROMModeConfig>>::handler.init(newMode, old);
        current = newMode;
    }

    template <std::size_t I = 0>
    void reInit(String typeName, ModeConfig &current)
    {
        if constexpr (I < std::variant_size_v<EEPROMModeConfig>)
        {
            if (typeName.compareTo(std::variant_alternative_t<I, EEPROMModeConfig>::handler_type::TYPE) == 0)
            {
                reInit<I>(current);
                return;
            }
            reInit<I + 1>(typeName, current);
            return;
        }
        reInit<0>(current);
    }

    template <std::size_t I = 0>
    void reInit(uint8_t typeIndex, ModeConfig &current)
    {
        if constexpr (I < std::variant_size_v<EEPROMModeConfig>)
        {
            if (typeIndex == I)
            {
                reInit<I>(current);
                return;
            }
            reInit<I + 1>(typeIndex, current);
            return;
        }
        reInit<0>(current);
    }

    typedef std::function<void(const ModeConfig &)> UpdateHandler;
    ModeConfig init(ESP8266WebServer &server, UDPLogger &logger, UpdateHandler updateHandler);

}
#endif