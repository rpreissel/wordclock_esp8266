#include <ranges>
#include <tuple>
#include <variant>
#include <cstdarg>
#include "wordclock.h"
#include "timedef.h"
#include "tools.h"

namespace wordclock
{
  const String clockStringGerman = "espistkfunfdreiviertelzwanzigzehnuminutenullvorqkjanachhalbNELFUNFEINSZWEIUNDDREISIGVIERSECHSXYACHTSIEBENZWOLFZEHNEUNAUHR";
  String message(clockStringGerman);

  void show(Env &env, const uint8_t config[12], uint8_t hours, uint8_t minutes, uint32_t color);
  int showStringOnClock(Env &env, const uint8_t config[12], uint8_t hours, uint8_t minutes, uint32_t color);
  void drawMinuteIndicator(Env &env, uint8_t minutes, uint32_t color);

  void WordClockHandler::toJson(const WordClockConfig &config, Env &env, JsonObject current)
  {
    baseConfigToJson(config, env, current);

    auto configArray = current[F("config")].to<JsonArray>();
    for (int c = 0; c < 12; c++)
    {
      configArray.add(config.config[c]);
    }
    current[F("fixed")] = config.fixed;
    current[F("hours")] = config.hours;
    current[F("minutes")] = config.minutes;
  }

  void WordClockHandler::fromJson(WordClockConfig &config, Env &env, JsonObjectConst doc)
  {
    baseConfigFromJson(config, env, doc);
    Serial.println(F("j1"));
    JsonVariantConst clockConfig = doc[F("config")];
    if (!clockConfig.isNull())
    {
      Serial.println(F("j2"));
      JsonArrayConst ar = clockConfig.as<JsonArrayConst>();
      for (int i = 0; i < 12; i++)
      {
        config.config[i] = ar[i];
      }
    }
    JsonVariantConst fixed = doc[F("fixed")];
    if (!fixed.isNull())
    {
      config.fixed = fixed.as<bool>();
    }
    JsonVariantConst hours = doc[F("hours")];
    if (!hours.isNull())
    {
      config.hours = hours.as<uint8_t>();
    }
    JsonVariantConst minutes = doc[F("minutes")];
    if (!minutes.isNull())
    {
      config.minutes = minutes.as<uint8_t>();
    }
  }

  void WordClockHandler::init(WordClockConfig &config, Env &env, const BaseConfig *old)
  {
    baseConfigInit(config, env, old, TYPE);
  }

  uint16_t WordClockHandler::onActivate(WordClockConfig &modeConfig, Env &env)
  {
    env.ledmatrix.gridFlush();
    env.ledmatrix.setBrightness(modeConfig.brightness);
    return 1;
  }

  uint16_t WordClockHandler::onLoop(WordClockConfig &modeConfig, Env &env, unsigned long millis)
  {
    env.ledmatrix.gridFlush();
    if (modeConfig.fixed)
    {
      show(env, modeConfig.config, modeConfig.hours, modeConfig.minutes, modeConfig.color);
    }
    else
    {
      show(env, modeConfig.config, env.ntp.getHours12(), env.ntp.getMinutes(), modeConfig.color);
    }
    return 500;
  }

  void show(Env &env, const uint8_t config[12], uint8_t hours, uint8_t minutes, uint32_t color)
  {
    showStringOnClock(env, config, hours, minutes, color);
    drawMinuteIndicator(env, minutes, color);
  }

  /**
   * @brief control the four minute indicator LEDs
   *
   * @param minutes minutes to be displayed [0 ... 59]
   * @param color 24bit color value
   */
  void drawMinuteIndicator(Env &env, uint8_t minutes, uint32_t color)
  {
    // separate LEDs for minutes in an additional row
    {
      switch (minutes % 5)
      {
      case 0:
        break;

      case 1:
        env.ledmatrix.setMinIndicator(0b1000, color);
        break;

      case 2:
        env.ledmatrix.setMinIndicator(0b1100, color);
        break;

      case 3:
        env.ledmatrix.setMinIndicator(0b1110, color);
        break;

      case 4:
        env.ledmatrix.setMinIndicator(0b1111, color);
        break;
      }
    }
  }

  /**
   * @brief Draw the given sentence to the word clock
   *
   * @param message sentence to be displayed
   * @param color 24bit color value
   * @return int: 0 if successful, -1 if sentence not possible to display
   */
  int showStringOnClock(Env &env, const uint8_t config[12], uint8_t hours, uint8_t minutes, uint32_t color)
  {

    timedef::timeToString(config, message, hours, minutes);
    // logger.logString("time as String: " + String(message));

    int messageStart = 0;
    String word = "";
    int lastLetterClock = 0;
    int positionOfWord = 0;
    int nextSpace = 0;
    int index = 0;

    // add space on the end of message for splitting
    message.concat(F(" "));

    while (true)
    {
      // extract next word from message
      word = split(message, ' ', index);
      index++;

      if (word.length() > 0)
      {
        // find word in clock string
        positionOfWord = clockStringGerman.indexOf(word, lastLetterClock);

        if (positionOfWord >= 0)
        {
          // word found on clock -> enable leds in targetgrid
          for (int i = 0; i < word.length(); i++)
          {
            int x = (positionOfWord + i) % LEDMatrix::width;
            int y = (positionOfWord + i) / LEDMatrix::width;
            env.ledmatrix.gridAddPixel(x, y, color);
          }
          // remember end of the word on clock
          lastLetterClock = positionOfWord + word.length();
        }
        else
        {
          // word is not possible to show on clock
          env.logger.logString("word is not possible to show on clock: " + String(word));
          return -1;
        }
        // logger.logString(String(nextSpace) + " - " + String());
      }
      else
      {
        // end - no more word in message
        break;
      }
    }
    // return success
    return 0;
  }
}