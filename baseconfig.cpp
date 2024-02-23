#include "baseconfig.h"

namespace config
{
    void baseConfigInit(BaseConfig &config, Env& env, const BaseConfig *old, const char *defaultName)
    {
        if (old)
        {
            config.name = old->name;
            config.color = old->color;
            config.brightness = old->brightness;
        }
        else
        {
            config.name = defaultName;
            config.color = 0xfff;
            config.brightness = 50;
        }
    }

    void baseConfigToJson(const BaseConfig &config, Env& env, JsonObject current)
    {
        current[F("name")] = config.name;
        current[F("color")] = config.color;
        current[F("brightness")] = config.brightness;
    }

    void baseConfigFromJson(BaseConfig &config, Env& env, JsonObjectConst doc)
    {
        JsonVariantConst brightness = doc[F("brightness")];
        if (!brightness.isNull())
        {
            config.brightness = brightness.as<int>();
        }
        JsonVariantConst color = doc[F("color")];
        if (!color.isNull())
        {
            config.color = color.as<int>();
        }
        const char *name = doc[F("name")];
        if (name)
        {
            config.name = name;
        }
    }
}