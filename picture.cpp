#include "picture.h"

namespace picture
{
    void PictureHandler::init(PictureConfig &modeConfig, Env &env)
    {
        for (int i = 0; i < 32; i++)
        {
            modeConfig.pixels[i] = 0;
        }
        for (int i = 0; i < 2; i++)
        {
            modeConfig.additionalColorIndexes[i] = 0;
        }
    }

    uint8_t colors(const PictureConfig &modeConfig)
    {
        int c = 1;
        for (int i = 0; i < 2; i++)
        {
            if (modeConfig.additionalColorIndexes[i])
            {
                c++;
            }
        }
        return c;
    }
    uint8_t getPixelColor(const PictureConfig &modeConfig, uint8_t r, uint8_t c, bool multiColor)
    {
        int _bit = (r * 11 + c) * (multiColor ? 2 : 1);
        uint8_t _byte = modeConfig.pixels[_bit / 8];
        if (multiColor)
        {
            return (_byte >> (_bit % 8)) & 0b11;
        }
        return (_byte >> (_bit % 8)) & 0b1;
    }
    void setPixelColor(PictureConfig &modeConfig, uint8_t r, uint8_t c, bool multiColor, uint8_t color)
    {
        int _bit = (r * 11 + c) * (multiColor ? 2 : 1);
        uint8_t &_byte = modeConfig.pixels[_bit / 8];
        if (multiColor)
        {
            _byte &= ~(0b11 << (_bit % 8));
            _byte |= (color & 0b11) << (_bit % 8);
        }
        else
        {
            _byte &= ~(0b1 << (_bit % 8));
            _byte |= (color == 1 ? 1 : 0) << (_bit % 8);
        }
    }
    uint8_t PictureHandler::toConfig(const PictureConfig &modeConfig, Env &env, uint64_t config[], const uint8_t emptyConfigs)
    {
        bool multiColor = colors(modeConfig) > 1;
        uint8_t bytes = multiColor ? 16 : 32;
        uint8_t configs = bytes / 8;
        if (emptyConfigs < configs)
        {
            return 0;
        }
        memcpy(config, modeConfig.pixels, bytes);
        if (multiColor)
        {
            uint64_t colors = ((modeConfig.additionalColorIndexes[0] & 0xF) << 56) |
                              ((modeConfig.additionalColorIndexes[1] & 0xF) << 60);
            config[configs - 1] |= colors;
        }

        return configs;
    }

    void PictureHandler::fromConfig(PictureConfig &modeConfig, Env &env, const uint64_t config[], const uint8_t usedConfigs)
    {
        if (usedConfigs == 2)
        {
            memcpy(modeConfig.pixels, config, 16);
        }
        else if (usedConfigs == 4)
        {
            memcpy(modeConfig.pixels, config, 32);
            uint64_t colors = config[usedConfigs - 1];
            modeConfig.additionalColorIndexes[0] = (colors >> 56) & 0xf;
            modeConfig.additionalColorIndexes[1] = (colors >> 60) & 0xf;
        }
    }

    void PictureHandler::modeToJson(const PictureConfig &modeConfig, Env &env, JsonObject data)
    {
        bool multiColor = colors(modeConfig) > 1;
        String row;
        row.reserve(LEDMatrix::width);
        data[F("color1")] = colorName(modeConfig.additionalColorIndexes[0]);
        data[F("color2")] = colorName(modeConfig.additionalColorIndexes[1]);
        JsonObject pixelsJson = data[F("pixels")].to<JsonObject>();
        for (int r = 0; r < LEDMatrix::height; r++)
        {
            row.clear();
            for (int c = 0; c < LEDMatrix::width; c++)
            {
                uint8_t color = getPixelColor(modeConfig, r, c, multiColor);
                if (color)
                {
                    row.concat(String(color));
                }
                else
                {
                    row.concat(" ");
                }
            }

            pixelsJson[String(r, HEX)] = row;
        }
    }
    void PictureHandler::modeFromJson(PictureConfig &modeConfig, Env &env, JsonObjectConst doc)
    {
        const char *colorName1 = doc[F("color1")];
        uint8_t color1 = colorName1 ? colorIndex(colorName1) : modeConfig.additionalColorIndexes[0];
        const char *colorName2 = doc[F("color2")];
        uint8_t color2 = colorName2 ? colorIndex(colorName2) : modeConfig.additionalColorIndexes[1];
        bool multiColor = color1 || color2;
        if (colorName1)
        {
            modeConfig.additionalColorIndexes[0] = color1;
        }
        if (colorName2)
        {
            modeConfig.additionalColorIndexes[1] = color2;
        }
        JsonVariantConst pixelJsonVariant = doc[F("pixels")];
        if (!pixelJsonVariant.isNull())
        {
            auto pixelJson = pixelJsonVariant.as<JsonObjectConst>();
            for (int r = 0; r < 11; r++)
            {
                const char *row = pixelJson[String(r, HEX)];
                if (!row)
                {
                    row = "           ";
                }

                for (int c = 0; c < 11 && c < strlen(row); c++)
                {
                    setPixelColor(modeConfig, r, c, multiColor, String(row[c]).toInt());
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
        uint32_t colorMap[] = {color((size_t)0),
                               color(modeConfig.colorIndex),
                               color(modeConfig.additionalColorIndexes[0]),
                               color(modeConfig.additionalColorIndexes[1])};
        env.ledmatrix.gridFlush();
        bool multiColor = colors(modeConfig) > 1;
        for (int r = 0; r < 11; r++)
        {
            for (int c = 0; c < 11; c++)
            {
                uint8_t color = getPixelColor(modeConfig, r, c, multiColor);
                env.ledmatrix.gridAddPixel(c, r, colorMap[color]);
            }
        }
        return 0;
    }
}