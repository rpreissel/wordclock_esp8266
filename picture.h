#ifndef PICTURE_H
#define PICTURE_H

#include "modes.h"

namespace picture
{
    using namespace modes;

    struct PictureConfig;

    struct PictureHandler : ModeTypeHandler<PictureConfig>
    {
        constexpr static const char *const TYPE = "PICTURE";

        virtual void init(PictureConfig &modeConfig,Env& env) override;
        virtual uint8_t toConfig(const PictureConfig &modeConfig, Env& env, uint64_t config[], const uint8_t emptyConfigs);
        virtual void fromConfig(PictureConfig &modeConfig, Env& env, const uint64_t config[], const uint8_t usedConfigs);

        virtual void modeToJson(const PictureConfig &modeConfig,Env& env, JsonObject data) override;
        virtual void modeFromJson(PictureConfig &modeConfig, Env& env,JsonObjectConst doc) override;

        virtual void onActivate(PictureConfig &modeConfig, Env& env) override;
        virtual uint32_t onLoop(PictureConfig &modeConfig, Env& env, unsigned long millis) override;
    };

    struct PictureConfig : BaseConfig
    {
        using handler_type = PictureHandler;
        uint32_t pixels[11];
        uint8_t additionalColorIndexes[3];
    };
}

#endif