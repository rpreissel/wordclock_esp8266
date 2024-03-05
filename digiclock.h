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

        virtual void onActivate(DigiClockConfig &modeConfig, Env& env) override;
        virtual uint32_t onLoop(DigiClockConfig &modeConfig, Env& env, unsigned long millis) override;
    };

    struct DigiClockConfig : BaseConfig
    {
        using handler_type = DigiClockHandler;
    };
}

#endif