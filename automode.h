#ifndef MODES_AUTO_H
#define MODES_AUTO_H

#include "modes.h"

namespace automode
{
    using namespace modes;

    struct TimerModeConfig;

    struct TimerModeHandler : ModeTypeHandler<TimerModeConfig>
    {
        constexpr static const char *const TYPE = "TIMER";
        int currentMode = -1;
        int modeInterval = -1;
        unsigned long lastModeTime = 0;
        virtual void init(TimerModeConfig &modeConfig,Env& env, const BaseConfig *old) override;
        virtual uint8_t toConfig(const TimerModeConfig &modeConfig, Env& env, uint64_t config[], const uint8_t emptyConfigs);
        virtual void fromConfig(TimerModeConfig &modeConfig, Env& env, const uint64_t config[], const uint8_t usedConfigs);

        virtual void toJson(const TimerModeConfig &modeConfig,Env& env, JsonObject data, JsonObject config) override;
        virtual void fromJson(TimerModeConfig &modeConfig, Env& env,JsonObjectConst doc) override;
        virtual void onActivate(TimerModeConfig &modeConfig, Env& env) override;
        virtual uint32_t onLoop(TimerModeConfig &modeConfig, Env& env, unsigned long millis) override;

    };

    struct Timer
    {
        int mode;
        uint16_t startMinute;
        uint16_t endMinute;
    };

    constexpr uint8_t MAX_TIMER_COUNT = 10;
    struct TimerModeConfig : BaseConfig
    {
        using handler_type = TimerModeHandler;
        int mainMode = 0;
        uint8_t timerCount = 0;
        Timer timers[MAX_TIMER_COUNT];
    };

}
#endif