#include <ranges>
#include <tuple>
#include <variant>
#include <cstdarg>
#include "automode.h"
#include "tools.h"

namespace automode
{
  uint8_t TimerModeHandler::toConfig(const TimerModeConfig &timerMode, Env &env, uint64_t config[], const uint8_t emptyConfigs)
  {
    uint8_t configCount = 1 + (timerMode.timerCount / 2) + (timerMode.timerCount % 2);
    if (emptyConfigs < configCount)
    {
      return 0;
    }

    for (int i = 0; i < configCount; i++)
    {
      config[i] = 0;
    }

    auto &config0 = config[0];
    config0 = (timerMode.mainMode & 0b11111) | (timerMode.timerCount << 5);
    for (int i = 0; i < timerMode.timerCount; i++)
    {
      auto &timer = timerMode.timers[i];
      config0 |= (timer.mode & 0b11111) << (5 * i + 10);

      auto &configX = config[i / 2 + 1];
      if (i % 2)
      {
        configX |= (timer.startMinute << 32);
        configX |= (timer.endMinute << 48);
      }
      else
      {
        configX |= (timer.startMinute);
        configX |= (timer.endMinute << 16);
      }
    }

    return configCount;
  }

  void TimerModeHandler::fromConfig(TimerModeConfig &timerMode, Env &env, const uint64_t config[], const uint8_t usedConfigs)
  {
    auto config0 = config[0];
    timerMode.mainMode = (config0 & 0b11111);
    if (timerMode.mainMode > 15)
    {
      timerMode.mainMode -= 32;
    }
    timerMode.timerCount = ((config0 >> 5) & 0xF);

    for (int i = 0; i < timerMode.timerCount; i++)
    {
      int mode = ((config0 >> (i * 5 + 10)) & 0b11111);
      if (mode > 15)
      {
        mode -= 32;
      }
      auto configX = config[i / 2 + 1];
      uint16_t startMinute;
      uint16_t endMinute;
      if (i % 2)
      {
        startMinute = (configX >> 32) & 0xFFFF;
        endMinute = (configX >> 48) & 0xFFFF;
      }
      else
      {
        startMinute = configX & 0xFFFF;
        endMinute = (configX >> 16) & 0xFFFF;
      }
      timerMode.timers[i] = {mode, startMinute, endMinute};
    }
  }

  void TimerModeHandler::toJson(const TimerModeConfig &timerMode, Env &env, JsonObject data, JsonObject config)
  {
    baseConfigToJson(timerMode, env, data, config);
    data[F("mainMode")] = timerMode.mainMode;

    JsonArray timers = data[F("timers")].to<JsonArray>();
    env.logger.logFormatted(F("Timers %d"), timerMode.timerCount);
    for (int i = 0; i < timerMode.timerCount; i++)
    {
      const auto &timer = timerMode.timers[i];
      auto timerJson = timers.add<JsonObject>();
      env.logger.logFormatted(F("Add Timer %d"), i);
      timerJson[F("mode")] = timer.mode;
      timerJson[F("startHour")] = timer.startMinute / 60;
      timerJson[F("startMinute")] = timer.startMinute % 60;
      timerJson[F("endHour")] = timer.endMinute / 60;
      timerJson[F("endMinute")] = timer.endMinute % 60;
    }
  }

  void TimerModeHandler::fromJson(TimerModeConfig &timerMode, Env &env, JsonObjectConst doc)
  {
    baseConfigFromJson(timerMode, env, doc);
    JsonVariantConst mainMode = doc[F("mainMode")];
    if (!mainMode.isNull()) 
    {
      timerMode.mainMode = mainMode.as<uint8_t>();
    }

    JsonVariantConst timersVariant = doc[F("timers")];
    if (!timersVariant.isNull())
    {
      JsonArrayConst timersArray = timersVariant.as<JsonArrayConst>();
      timerMode.timerCount = 0;
      for (JsonVariantConst value : timersArray) 
      {
        auto timerJson = value.as<JsonObjectConst>();
        int mode = timerJson[F("mode")];
        uint8_t startHour = timerJson[F("startHour")];
        uint8_t startMinute = timerJson[F("startMinute")];
        uint8_t endHour = timerJson[F("endHour")];
        uint8_t endMinute = timerJson[F("endMinute")];
        uint16_t startMinutes = startHour * 60 + startMinute;
        uint16_t endMinutes = endHour * 60 + endMinute;
        timerMode.timers[ timerMode.timerCount++] = {mode, startMinutes, endMinutes};
      }
      env.logger.logFormatted(F("timers %d"),timerMode.timerCount);
    }
  }

  void TimerModeHandler::init(TimerModeConfig &config, Env &env, const BaseConfig *old)
  {
    baseConfigInit(config, env, old, TYPE);
    config.timerCount = 1;
    config.timers[0] = Timer{OffConfig::MODE_OFF_INDEX, 0, 60};
  }

  void TimerModeHandler::onActivate(TimerModeConfig &modeConfig, Env &env)
  {
    currentMode = modeConfig.mainMode;
    modeInterval = 1;
    lastModeTime = 0;
    env.activateNextMode(currentMode);
  }

  uint32_t TimerModeHandler::onLoop(TimerModeConfig &modeConfig, Env &env, unsigned long millis)
  {
    int minutes = env.dayMinutes();
    int newMode = modeConfig.mainMode;
    for (int i = 0; i < modeConfig.timerCount; i++)
    {
      auto &timer = modeConfig.timers[i];
      if (timer.startMinute <= minutes && minutes <= timer.endMinute)
      {
        newMode = timer.mode;
        break;
      }
    }
    if (currentMode != newMode)
    {
      currentMode = newMode;
      modeInterval = 1;
      lastModeTime = 0;
      env.activateNextMode(currentMode);
    }

    if (millis - lastModeTime > modeInterval)
    {
      lastModeTime = millis;
      modeInterval = env.loopNextMode(millis);
    }

    if (modeInterval < 1000)
    {
      return modeInterval;
    }
    return 1000;
  }

}