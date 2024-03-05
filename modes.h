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
    class Env
    {
    private:
        NTPClientPlus &ntp;
        bool _fixedTime;
        uint8_t _fixedHours;
        uint8_t _fixedMinutes;

    public:
        LEDMatrix &ledmatrix;
        UDPLogger &logger;
        ActivateDelegator activateNextMode;
        LoopDelegator loopNextMode;

        Env(LEDMatrix &ledmatrix,
            UDPLogger &logger,
            NTPClientPlus &ntp,
            ActivateDelegator activateNextMode,
            LoopDelegator loopNextMode)
            : ledmatrix(ledmatrix),
              logger(logger),
              ntp(ntp),
              activateNextMode(activateNextMode),
              loopNextMode(loopNextMode) {}

        bool fixedTime()
        {
            return _fixedTime;
        }
        uint8_t fixedHours()
        {
            return _fixedHours;
        }
        uint8_t fixedMinutes()
        {
            return _fixedMinutes;
        }
        uint8_t minutes()
        {
            return _fixedTime ? _fixedMinutes : ntp.getMinutes();
        }
        uint8_t hours()
        {
            return _fixedTime ? _fixedHours : ntp.getHours24();
        }
        uint16_t dayMinutes()
        {
            return hours() * 60 + minutes();
        }
        void fixedTime(bool fixed)
        {
            _fixedTime = fixed;
        }
        void fixedHours(uint8_t hours)
        {
            _fixedHours = hours;
        }
        void fixedMinutes(uint8_t minutes)
        {
            _fixedMinutes = minutes;
        }
    };

    template <typename TModeType>
    struct ModeTypeHandler
    {
        virtual void init(TModeType &modeConfig, Env &env)
        {
        }
        virtual uint8_t toConfig(const TModeType &modeConfig, Env &env, uint64_t config[], const uint8_t emptyConfigs)
        {
            return 0;
        }
        virtual void fromConfig(TModeType &modeConfig, Env &env, const uint64_t config[], const uint8_t usedConfigs)
        {
        }
        virtual void toJson(const TModeType &modeConfig, Env &env, JsonObject data, JsonObject config)
        {
        }
        virtual void fromJson(TModeType &modeConfig, Env &env, JsonObjectConst doc)
        {
        }
        virtual void onActivate(TModeType &modeConfig, Env &env)
        {
            env.ledmatrix.gridFlush();
        }
        virtual uint32_t onLoop(TModeType &modeConfig, Env &env, unsigned long millis)
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

   
}

#endif