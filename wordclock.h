#ifndef MODES_WORDCLOCK_H
#define MODES_WORDCLOCK_H

#include "modes.h"

namespace wordclock
{
    using namespace modes;

    struct WordClockConfig;

    struct WordClockHandler : ModeTypeHandler<WordClockConfig>
    {
        constexpr static const char *const TYPE = "WORDCLOCK";
        virtual void init(WordClockConfig &modeConfig,Env& env, const BaseConfig *old) override;
        virtual uint8_t toConfig(const WordClockConfig &modeConfig, Env& env, uint64_t config[], const uint8_t emptyConfigs);
        virtual void fromConfig(WordClockConfig &modeConfig, Env& env, const uint64_t config[], const uint8_t usedConfigs);

        virtual void toJson(const WordClockConfig &modeConfig,Env& env, JsonObject data, JsonObject config) override;
        virtual void fromJson(WordClockConfig &modeConfig, Env& env,JsonObjectConst doc) override;
        virtual uint32_t onActivate(WordClockConfig &modeConfig, Env& env) override;
        virtual uint32_t onLoop(WordClockConfig &modeConfig, Env& env, unsigned long millis) override;

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