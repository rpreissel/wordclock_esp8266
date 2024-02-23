#include "digiclock.h"

namespace digiclock
{
    using namespace config;
       
    void DigiClockHandler::toJson(const DigiClockConfig &config,Env& env, JsonObject current)
    {
        baseConfigToJson(config, env, current);
    }

    void DigiClockHandler::fromJson(DigiClockConfig &config, Env& env,JsonObjectConst current)
    {
        baseConfigFromJson(config, env, current);
    }

    void DigiClockHandler::init(DigiClockConfig &config, Env& env,const BaseConfig *old)
    {
        baseConfigInit(config, env, old, TYPE);
    }
}