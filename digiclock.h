#ifndef MODES_DIGICLOCK_H
#define MODES_DIGICLOCK_H

#include "modes.h"

namespace digiclock
{
    using namespace modes;

    struct DigiClockConfig;

    struct DigiClockHandler : ModeTypeHandler<DigiClockConfig>
    {
        constexpr static const char *const TYPE = "DIGICLOCK";
        virtual void init(DigiClockConfig &modeConfig,Env& env, const BaseConfig *old) override;
        virtual void toJson(const DigiClockConfig &modeConfig, Env& env, JsonObject data, JsonObject config) override;
        virtual void fromJson(DigiClockConfig &modeConfig, Env& env,JsonObjectConst doc) override;

        virtual uint32_t onActivate(DigiClockConfig &modeConfig, Env& env) override;
        virtual uint32_t onLoop(DigiClockConfig &modeConfig, Env& env, unsigned long millis) override;
    };

    struct DigiClockConfig : BaseConfig
    {
        using handler_type = DigiClockHandler;
    };
}

#endif