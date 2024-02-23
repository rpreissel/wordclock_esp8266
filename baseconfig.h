#ifndef BASECONFIG_H
#define BASECONFIG_H
#include <ArduinoJson.h>
#include <type_traits>
#include <concepts>
#include <variant>
#include "tools.h"
#include "ledmatrix.h"
#include "udplogger.h"
#include "ntp_client_plus.h"

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
    struct Env
    {
        LEDMatrix &ledmatrix;
        UDPLogger &logger;
        NTPClientPlus &ntp;
    };
    

    template <typename TModeType>
    struct ModeTypeHandler
    {
        virtual void init(TModeType &modeConfig, Env& env, const BaseConfig *old)
        {
        }
        virtual void toJson(const TModeType &modeConfig, Env& env, JsonObject doc)
        {
        }
        virtual void fromJson(TModeType &modeConfig, Env& env,JsonObjectConst doc)
        {
        }
        virtual uint16_t onActivate(TModeType &modeConfig, Env& env)
        {
            return 0;
        }
        virtual uint16_t onLoop(TModeType &modeConfig, Env& env, unsigned long millis)
        {
            return 0;
        }
    };

    struct BaseConfig
    {
        uint8_t brightness;
        uint32_t color;
        String name;
    };

    void baseConfigInit(BaseConfig &config, Env& env,const BaseConfig *old, const char *defaultName);
    void baseConfigToJson(const BaseConfig &config, Env& env,JsonObject current);
    void baseConfigFromJson(BaseConfig &config, Env& env,JsonObjectConst doc);

    struct Empty;

    struct EmptyTypeHandler : ModeTypeHandler<Empty>
    {
        constexpr static const char *TYPE = "EMPTY";
    };

    struct Empty
    {
        using handler_type = EmptyTypeHandler;
    };

    struct OffConfig;

    struct OffTypeHandler : ModeTypeHandler<OffConfig>
    {
        constexpr static const char *const TYPE = "OFF";
    };

    struct OffConfig
    {
        using handler_type = OffTypeHandler;
        constexpr static int MODE_OFF_INDEX = -1;
    };
   
    template <typename... Args>
    void toJson(std::variant<Args...> &para, Env& env, int index, JsonObject doc)
    {
        doc[F("index")] = index;
        return std::visit(Overload{[doc,&env](Args &mt)
                                   {
                                       doc[F("type")] = Args::handler_type::TYPE;
                                       _handler_instance<Args>::handler.toJson(mt, env, doc);
                                   }...},
                          para);
    }

    template <typename... Args>
    void fromJson(std::variant<Args...> &para, Env& env, JsonObjectConst doc)
    {
        return std::visit(Overload{[doc,&env](Args &mt)
                                   { _handler_instance<Args>::handler.fromJson(mt, env, doc); }...},
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
        const BaseConfig *baseConfig = toBaseConfig(para);
        if (baseConfig)
        {
            return baseConfig->name + F(" (") + type + F(")");
        }
        return type;
    }

    template <typename... Args>
    uint16_t onActivate(std::variant<Args...> &para, Env& env)
    {
        return std::visit(Overload{[&](Args &mt)
                                   { return _handler_instance<Args>::handler.onActivate(mt, env); }...},
                          para);
    }

    template <typename... Args>
    uint16_t onLoop(std::variant<Args...> &para, Env& env, unsigned long millis)
    {
        return std::visit(Overload{[&](Args &mt)
                                   { return _handler_instance<Args>::handler.onLoop(mt, env , millis); }...},
                          para);
    }
}

#endif