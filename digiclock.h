#ifndef DIGICLOCK_H
#define DIGICLOCK_H

#include "baseconfig.h"

namespace digiclock
{
    using namespace config;

    struct DigiClockConfig;

    struct DigiClockHandler : ModeTypeHandler<DigiClockConfig>
    {
        constexpr static const char *const TYPE = "DIGICLOCK";
        virtual void init(DigiClockConfig &modeConfig,Env& env, const BaseConfig *old) override;
        virtual void toJson(const DigiClockConfig &modeConfig, Env& env,JsonObject doc) override;
        virtual void fromJson(DigiClockConfig &modeConfig, Env& env,JsonObjectConst doc) override;
    };

    struct DigiClockConfig : BaseConfig
    {
        using handler_type = DigiClockHandler;
    };
}

#endif