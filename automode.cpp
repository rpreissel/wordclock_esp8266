#include <ranges>
#include <tuple>
#include <variant>
#include <cstdarg>
#include "automode.h"
#include "tools.h"

namespace automode
{
  uint8_t TimerModeHandler::toConfig(const TimerModeConfig &modeConfig, Env &env, uint64_t config[], const uint8_t emptyConfigs)
  {
    return 0;
  }

  void TimerModeHandler::fromConfig(TimerModeConfig &modeConfig, Env &env, const uint64_t config[], const uint8_t usedConfigs)
  {
  }

  void TimerModeHandler::toJson(const TimerModeConfig &clock, Env &env, JsonObject data, JsonObject config)
  {
    baseConfigToJson(clock, env, data, config);
  }

  void TimerModeHandler::fromJson(TimerModeConfig &config, Env &env, JsonObjectConst doc)
  {
    baseConfigFromJson(config, env, doc);
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
    int minutes = env.ntp.getHours24() * 60 + env.ntp.getMinutes();
    int newMode = modeConfig.mainMode;
    for(int i=0; i <modeConfig.timerCount; i++)
    {
      auto& timer = modeConfig.timers[i];
      if(timer.startMinute < minutes && minutes < timer.endMinute)
      {
        if(currentMode != timer.mode)
        {
          newMode = timer.mode;
        }
        break;
      }
    }
    if(currentMode != newMode) 
    {
      currentMode = newMode;
      modeInterval = 1;
      lastModeTime = 0;
      env.activateNextMode(currentMode);
    }
    
    if(millis -  lastModeTime > modeInterval)
    {
      lastModeTime = millis;
      modeInterval = env.loopNextMode(millis);
    }

    if(modeInterval < 1000)
    {
      return modeInterval;
    }
    return 1000;
  }

}