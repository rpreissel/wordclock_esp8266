#ifndef STATE_H
#define STATE_H

#include <ESP8266WebServer.h>

#include <variant>
#include "baseconfig.h"
#include "wordclock.h"
#include "digiclock.h"
#include "udplogger.h"

namespace config
{
  
    using EEPROMModeConfig = std::variant<Empty, wordclock::WordClockConfig, digiclock::DigiClockConfig>;
    using ModeConfig = concatenator<EEPROMModeConfig, OffConfig>::type;

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

    typedef std::function<void(const ModeConfig &)> UpdateHandler;
    ModeConfig init(ESP8266WebServer &server, LEDMatrix& ledmatrix, UDPLogger &logger, NTPClientPlus &ntp, UpdateHandler updateHandler);
    void loop(unsigned long millis);
}
#endif