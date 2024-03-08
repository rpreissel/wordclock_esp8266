#include "picture.h"

namespace picture
{
    void PictureHandler::init(PictureConfig &modeConfig,Env& env)
    {
        pixels[0] = 1;
        pixels[1] = 1;
    }
    uint8_t PictureHandler::toConfig(const PictureConfig &modeConfig, Env& env, uint64_t config[], const uint8_t emptyConfigs)
    {
        if(emptyConfigs < 2)
        {
            return 0;
        }
        config[0] = modeConfig.pixels[0];
        config[1] = modeConfig.pixels[1];
    }

    void PictureHandler::fromConfig(PictureConfig &modeConfig, Env& env, const uint64_t config[], const uint8_t usedConfigs)
    {
        if(usedConfigs == 2)
        {
            modeConfig.pixels[0] = config[0];
            modeConfig.pixels[1] = config[1];
        }
    }

    void PictureHandler::modeToJson(const PictureConfig &modeConfig,Env& env, JsonObject data)
    {

    }
    void PictureHandler::modeFromJson(PictureConfig &modeConfig, Env& env,JsonObjectConst doc)
    {

    }

    void PictureHandler::onActivate(PictureConfig &modeConfig, Env &env)
    {
        env.ledmatrix.gridFlush();
        env.ledmatrix.setBrightness(modeConfig.brightness);
    }

    uint32_t PictureHandler::onLoop(PictureConfig &modeConfig, Env &env, unsigned long millis)
    {
        env.ledmatrix.gridFlush();
        uint64_t pixels = modeConfig.pixels[0];
        for(int i =0;i<64;i++)
        {
            
        }
        return 0;
    }
}