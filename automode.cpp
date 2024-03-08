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

  void TimerModeHandler::modeToJson(const TimerModeConfig &timerMode, Env &env, JsonObject data)
  {
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

  void TimerModeHandler::modeFromJson(TimerModeConfig &timerMode, Env &env, JsonObjectConst doc)
  {
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

  void TimerModeHandler::init(TimerModeConfig &config, Env &env)
  {
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

    if (modeInterval > 0 && modeInterval < 1000)
    {
      return modeInterval;
    }
    return 1000;
  }

  uint8_t IntervalModeHandler::toConfig(const IntervalModeConfig &intervalMode, Env &env, uint64_t config[], const uint8_t emptyConfigs)
  {
    uint8_t configCount = 1 + (intervalMode.intervalCount / 4) + ((intervalMode.intervalCount % 4) ? 1 : 0);
    if (emptyConfigs < configCount)
    {
      return 0;
    }

    for (int i = 0; i < configCount; i++)
    {
      config[i] = 0;
    }

    auto &config0 = config[0];
    config0 = intervalMode.intervalCount;
    for (int i = 0; i < intervalMode.intervalCount; i++)
    {
      auto &interval = intervalMode.intervals[i];
      config0 |= (interval.mode & 0b11111) << (5 * i + 4);

      auto &configX = config[i / 4 + 1];
      auto pos= i % 4;
      configX |= (((uint64_t)interval.intervalSeconds) << (pos*16));      
    }

    return configCount;
  }

  void IntervalModeHandler::fromConfig(IntervalModeConfig &intervalMode, Env &env, const uint64_t config[], const uint8_t usedConfigs)
  {
    auto config0 = config[0];
    intervalMode.intervalCount = (config0 & 0xF);
    
    for (int i = 0; i < intervalMode.intervalCount; i++)
    {
      int mode = ((config0 >> (i * 5 + 4)) & 0b11111);
      if (mode > 15)
      {
        mode -= 32;
      }
      auto configX = config[i / 4 + 1];
      auto pos= i % 4;
      uint16_t  seconds = (configX >> (pos*16)) & 0xFFFF;
      intervalMode.intervals[i] = {mode, seconds};
    }
  }

  void IntervalModeHandler::modeToJson(const IntervalModeConfig &intervalMode, Env &env, JsonObject data)
  {
    JsonArray intervals = data[F("intervals")].to<JsonArray>();
    for (int i = 0; i < intervalMode.intervalCount; i++)
    {
      const auto &interval = intervalMode.intervals[i];
      auto intervalJson = intervals.add<JsonObject>();
      intervalJson[F("mode")] = interval.mode;
      intervalJson[F("seconds")] = interval.intervalSeconds;
    }
  }

  void IntervalModeHandler::modeFromJson(IntervalModeConfig &intervalMode, Env &env, JsonObjectConst doc)
  {
    JsonVariantConst intervalsVariant = doc[F("intervals")];
    if (!intervalsVariant.isNull())
    {
      JsonArrayConst intervalsArray = intervalsVariant.as<JsonArrayConst>();
      intervalMode.intervalCount = 0;
      for (JsonVariantConst value : intervalsArray) 
      {
        auto intervalJson = value.as<JsonObjectConst>();
        int mode = intervalJson[F("mode")];
        uint16_t seconds = intervalJson[F("seconds")];
        intervalMode.intervals[ intervalMode.intervalCount++] = {mode, seconds};
      }
    }
    env.logger.logFormatted(F("intervals %d"),intervalMode.intervalCount);
  }

  void IntervalModeHandler::init(IntervalModeConfig &config, Env &env)
  {
    env.logger.logFormatted(F("init intervals"));
    config.intervalCount = 2;
    config.intervals[0] = Interval{0, 60};
    config.intervals[1] = Interval{1, 60};
  }

  void IntervalModeHandler::onActivate(IntervalModeConfig &modeConfig, Env &env)
  {
    currentInterval=-1;
    intervalEndTime=-1;
  }

  uint32_t IntervalModeHandler::onLoop(IntervalModeConfig &modeConfig, Env &env, unsigned long millis)
  {
    if(modeConfig.intervalCount == 0)
    {
      return 0;
    }
    if(currentInterval==-1 || millis > intervalEndTime)
    {
      currentInterval++;
      if(currentInterval > modeConfig.intervalCount)
      {
        currentInterval = 0;
      }
      intervalEndTime = millis + (modeConfig.intervals[currentInterval].intervalSeconds * 1000);
      lastModeTime = 0;
      modeInterval = 1;
      env.activateNextMode(modeConfig.intervals[currentInterval].mode);
    }

    if(modeInterval>0 && millis - lastModeTime > modeInterval)
    {
      lastModeTime = millis;
      modeInterval = env.loopNextMode(millis);
    }

    int millisToIntervalEnd = intervalEndTime - millis;
    if(millisToIntervalEnd <=0) 
    {
      return 1;
    }

    if(modeInterval <=0 || modeInterval > millisToIntervalEnd)
    {
      return millisToIntervalEnd;
    }

    return modeInterval;
  }

}