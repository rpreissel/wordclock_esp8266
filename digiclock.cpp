#include "digiclock.h"

namespace digiclock
{
    void DigiClockHandler::toJson(const DigiClockConfig &clock, Env &env, JsonObject data, JsonObject config)
    {
        baseConfigToJson(clock, env, data, config);
    }

    void DigiClockHandler::fromJson(DigiClockConfig &config, Env &env, JsonObjectConst current)
    {
        baseConfigFromJson(config, env, current);
    }

    void DigiClockHandler::init(DigiClockConfig &config, Env &env, const BaseConfig *old)
    {
        baseConfigInit(config, env, old, TYPE);
    }

    uint32_t DigiClockHandler::onActivate(DigiClockConfig &modeConfig, Env &env)
    {
        env.ledmatrix.gridFlush();
        env.ledmatrix.setBrightness(modeConfig.brightness);
        return 1;
    }

    uint32_t DigiClockHandler::onLoop(DigiClockConfig &modeConfig, Env &env, unsigned long millis)
    {
        env.ledmatrix.gridFlush();
        auto hours = env.ntp.getHours24();
        auto minutes = env.ntp.getMinutes();

        uint8_t fstDigitH = hours / 10;
        uint8_t sndDigitH = hours % 10;
        uint8_t fstDigitM = minutes / 10;
        uint8_t sndDigitM = minutes % 10;
        env.ledmatrix.printNumber(2, 0, fstDigitH, modeConfig.color);
        env.ledmatrix.printNumber(6, 0, sndDigitH, modeConfig.color);
        env.ledmatrix.printNumber(2, 6, fstDigitM, modeConfig.color);
        env.ledmatrix.printNumber(6, 6, sndDigitM, modeConfig.color);
        return 500;
    }
}