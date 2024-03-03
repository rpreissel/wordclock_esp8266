#ifndef MODES_BASE_H
#define MODES_BASE_H
#include <ArduinoJson.h>
#include <type_traits>
#include <concepts>
#include <variant>
#include "tools.h"
#include "ledmatrix.h"
#include "udplogger.h"
#include "ntp_client_plus.h"

namespace modes
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
    using ActivateDelegator = std::function<void(int)>;
    using LoopDelegator = std::function<uint32_t(unsigned long)>;
    struct Env
    {
        LEDMatrix &ledmatrix;
        UDPLogger &logger;
        NTPClientPlus &ntp;
        ActivateDelegator activateNextMode;
        LoopDelegator loopNextMode;
    };
    

    template <typename TModeType>
    struct ModeTypeHandler
    {
        virtual void init(TModeType &modeConfig, Env& env, const BaseConfig *old)
        {
        }
        virtual uint8_t toConfig(const TModeType &modeConfig, Env& env, uint64_t config[], const uint8_t emptyConfigs)
        {
            return 0;
        }
        virtual void fromConfig(TModeType &modeConfig, Env& env, const uint64_t config[], const uint8_t usedConfigs)
        {
        }
        virtual void toJson(const TModeType &modeConfig, Env& env, JsonObject data, JsonObject config)
        {
        }
        virtual void fromJson(TModeType &modeConfig, Env& env,JsonObjectConst doc)
        {
        }
        virtual void onActivate(TModeType &modeConfig, Env& env)
        {        
            env.ledmatrix.gridFlush();
        }
        virtual uint32_t onLoop(TModeType &modeConfig, Env& env, unsigned long millis)
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
    void baseConfigToJson(const BaseConfig &baseConfig, Env& env, JsonObject data, JsonObject config);
    void baseConfigFromJson(BaseConfig &config, Env& env,JsonObjectConst data);

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
    void fromConfig(std::variant<Args...> &para, Env& env, const uint64_t config[], const uint8_t usedConfigs)
    {
        return std::visit(Overload{[config,usedConfigs,&env](Args &mt)
                                   {
                                       _handler_instance<Args>::handler.fromConfig(mt, env, config, usedConfigs);
                                   }...},
                          para);
    }

    template <typename... Args>
    uint8_t toConfig(std::variant<Args...> &para, Env& env, uint64_t config[], const uint8_t emptyConfigs)
    {
        return std::visit(Overload{[config,emptyConfigs,&env](Args &mt)
                                   {
                                       return _handler_instance<Args>::handler.toConfig(mt, env, config, emptyConfigs);
                                   }...},
                          para);
    }

    template <typename... Args>
    void toJson(std::variant<Args...> &para, Env& env, int index, JsonObject doc)
    {
        doc[F("index")] = index;
        return std::visit(Overload{[doc,&env](Args &mt)
                                   {
                                       doc[F("type")] = Args::handler_type::TYPE;
                                        JsonObject data = doc[F("data")].to<JsonObject>();
                                        JsonObject config = doc[F("config")].to<JsonObject>();
                                       _handler_instance<Args>::handler.toJson(mt, env, data, config);
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
    BaseConfig *toBaseConfig(std::variant<Args...> &para)
    {
        return std::visit(Overload{[](Args &mt) -> BaseConfig *
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
    void onActivate(std::variant<Args...> &para, Env& env)
    {
        env.logger.logFormatted(F("OnActivate %d"), para.index());
        return std::visit(Overload{[&](Args &mt)
                                   { _handler_instance<Args>::handler.onActivate(mt, env); }...},
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