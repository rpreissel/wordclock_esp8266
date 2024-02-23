#ifndef WORDCLOCK_H
#define WORDCLOCK_H

#include <tuple>
#include "ledmatrix.h"
#include "udplogger.h"
#include "baseconfig.h"

namespace wordclock
{
    using namespace config;

    struct WordClockConfig;

    struct WordClockHandler : ModeTypeHandler<WordClockConfig>
    {
        constexpr static const char *const TYPE = "WORDCLOCK";
        virtual void init(WordClockConfig &modeConfig,Env& env, const BaseConfig *old) override;
        virtual void toJson(const WordClockConfig &modeConfig,Env& env, JsonObject doc) override;
        virtual void fromJson(WordClockConfig &modeConfig, Env& env,JsonObjectConst doc) override;
        virtual uint16_t onActivate(WordClockConfig &modeConfig, Env& env) override;
        virtual uint16_t onLoop(WordClockConfig &modeConfig, Env& env, unsigned long millis) override;

    };
    struct WordClockConfig : BaseConfig
    {
        using handler_type = WordClockHandler;
        uint8_t config[12] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
        bool fixed;
        uint8_t hours;
        uint8_t minutes;
    };

}
#endif