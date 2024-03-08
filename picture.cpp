#include "picture.h"

namespace picture
{
    void PictureHandler::init(PictureConfig &modeConfig, Env &env)
    {
        for (int i = 0; i < 8; i++)
        {
            modeConfig.pixels[i] = i;
        }
    }

    uint8_t PictureHandler::toConfig(const PictureConfig &modeConfig, Env &env, uint64_t config[], const uint8_t emptyConfigs)
    {
        if (emptyConfigs < 1)
        {
            return 0;
        }
        memcpy(config, modeConfig.pixels, 8);

        return 1;
    }

    void PictureHandler::fromConfig(PictureConfig &modeConfig, Env &env, const uint64_t config[], const uint8_t usedConfigs)
    {
        if (usedConfigs == 1)
        {
            memcpy(modeConfig.pixels, config, 8);
        }
    }

    void PictureHandler::modeToJson(const PictureConfig &modeConfig, Env &env, JsonObject data)
    {
        String row;
        row.reserve(8);
        JsonObject pixelsJson = data[F("pixels")].to<JsonObject>();
        for (int r = 0; r < 8; r++)
        {
            row.clear();
            for (int c = 0; c < 8; c++)
            {
                if (modeConfig.pixels[r] & (1 << c))
                {
                    row.concat("*");
                }
                else
                {
                    row.concat(" ");
                }
            }

            pixelsJson[String(r)] = row;
        }
    }
    void PictureHandler::modeFromJson(PictureConfig &modeConfig, Env &env, JsonObjectConst doc)
    {
        JsonVariantConst pixelJsonVariant = doc[F("pixels")];
        if (!pixelJsonVariant.isNull()) 
        {
            auto pixelJson = pixelJsonVariant.as<JsonObjectConst>();
            for (int r = 0; r < 8; r++)
            {
                const char* row = pixelJson[String(r)];
                if(!row) {
                    continue;
                }
                modeConfig.pixels[r] = 0;
                for (int c = 0; c < 8 && c < strlen(row); c++)
                {
                    if (row[c] == '*')
                    {
                        modeConfig.pixels[r]|= (1<<c);
                    } 
                }
            }
        }
    }

    void PictureHandler::onActivate(PictureConfig &modeConfig, Env &env)
    {
        env.ledmatrix.gridFlush();
        env.ledmatrix.setBrightness(modeConfig.brightness);
    }

    uint32_t PictureHandler::onLoop(PictureConfig &modeConfig, Env &env, unsigned long millis)
    {
        env.ledmatrix.gridFlush();

        for (int r = 0; r < 8; r++)
        {
            for (int c = 0; c < 8; c++)
            {
                if (modeConfig.pixels[r] & (1 << c))
                {
                    env.ledmatrix.gridAddPixel(c + 2, r + 2, modeConfig.color);
                }
            }
        }
        return 0;
    }
}