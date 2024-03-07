#include <ranges>
#include <tuple>
#include <variant>
#include <cstdarg>
#include "wordclock.h"
#include "tools.h"

namespace timedef
{

  struct TimeDefs
  {
    const char *periods[12][2];
  };

  typedef void (*TimePeriodFunction)(String &sentence, uint8_t hours);

  class TimePeriodDef
  {
  public:
    constexpr TimePeriodDef(const char *name, const TimePeriodFunction function)
        : name(name), function(function) {}

    const char *name;
    const TimePeriodFunction function;
  };

  const std::variant<String, std::tuple<String, String>> HOURS[12] = {
      "ZWOLF", std::make_tuple("EIN", "EINS"), "ZWEI", "DREI", "VIER", "FUNF", "SECHS", "SIEBEN", "ACHT", "NEUN", "ZEHN", "ELF"};

  void addHours(String &sentence, uint8_t hours, bool withUhr = false)
  {
    auto hoursDef = HOURS[hours % 12];
    if (auto h = std::get_if<std::tuple<String, String>>(&hoursDef))
    {
      if (withUhr)
      {
        sentence.concat(std::get<0>(*h));
        sentence.concat(F(" UHR"));
      }
      else
      {
        sentence.concat(std::get<1>(*h));
      }
    }
    else
    {
      sentence.concat(std::get<String>(hoursDef));
      if (withUhr)
      {
        sentence.concat(F(" UHR"));
      }
    }
  }

  constexpr TimePeriodDef TPD_00_UHR =
      {"10 Uhr",
       ([](String &sentence, uint8_t hours)
        { addHours(sentence, hours, true); })};
  constexpr TimePeriodDef TPD_00_UM =
      {"um 10",
       ([](String &sentence, uint8_t hours)
        {sentence.concat(F("um "));addHours(sentence, hours); })};
  constexpr TimePeriodDef TPD_05 =
      {"5 nach 10",
       ([](String &sentence, uint8_t hours)
        {sentence.concat(F("funf nach ")) ;addHours(sentence, hours); })};
  constexpr TimePeriodDef TPD_10 =
      {"10 nach 10",
       ([](String &sentence, uint8_t hours)
        {sentence.concat(F("zehn nach ")) ;addHours(sentence, hours); })};
  constexpr TimePeriodDef TPD_15_NACH =
      {"viertel nach 10",
       ([](String &sentence, uint8_t hours)
        {sentence.concat(F("viertel nach ")) ;addHours(sentence, hours); })};
  constexpr TimePeriodDef TPD_15_KUCHEN =
      {"viertel 11",
       ([](String &sentence, uint8_t hours)
        {sentence.concat(F("viertel ")) ;addHours(sentence, hours + 1); })};
  constexpr TimePeriodDef TPD_20_HALB =
      {"10 vor halb 11",
       ([](String &sentence, uint8_t hours)
        {sentence.concat(F("zehn vor halb ")) ;addHours(sentence, hours + 1); })};
  constexpr TimePeriodDef TPD_20_ZWANZIG =
      {"20 nach 10",
       ([](String &sentence, uint8_t hours)
        {sentence.concat(F("zwanzig nach ")); addHours(sentence, hours); })};
  constexpr TimePeriodDef TPD_25 =
      {"5 vor halb 11",
       ([](String &sentence, uint8_t hours)
        {sentence.concat(F("funf vor halb ")) ;addHours(sentence, hours + 1); })};
  constexpr TimePeriodDef TPD_30 =
      {"halb 11",
       ([](String &sentence, uint8_t hours)
        {sentence.concat(F("halb ")) ;addHours(sentence, hours + 1); })};
  constexpr TimePeriodDef TPD_35 =
      {"5 nach halb 11",
       ([](String &sentence, uint8_t hours)
        {sentence.concat(F("funf nach halb ")) ;addHours(sentence, hours + 1); })};
  constexpr TimePeriodDef TPD_40_HALB =
      {"10 nach halb 11",
       ([](String &sentence, uint8_t hours)
        {sentence.concat(F("zehn nach halb ")) ;addHours(sentence, hours + 1); })};
  constexpr TimePeriodDef TPD_40_ZWANZIG =
      {"20 vor 11",
       ([](String &sentence, uint8_t hours)
        {sentence.concat(F("zwanzig vor ")) ;addHours(sentence, hours + 1); })};
  constexpr TimePeriodDef TPD_45_VOR =
      {"viertel vor 11",
       ([](String &sentence, uint8_t hours)
        {sentence.concat(F("viertel vor ")) ;addHours(sentence, hours + 1); })};
  constexpr TimePeriodDef TPD_45_KUCHEN =
      {"dreiviertel 11",
       ([](String &sentence, uint8_t hours)
        {sentence.concat(F("dreiviertel ")) ;addHours(sentence, hours + 1); })};
  constexpr TimePeriodDef TPD_50 =
      {"10 vor 11",
       ([](String &sentence, uint8_t hours)
        {sentence.concat(F("zehn vor ")) ;addHours(sentence, hours + 1); })};
  constexpr TimePeriodDef TPD_55 =
      {"5 vor 11",
       ([](String &sentence, uint8_t hours)
        {sentence.concat(F("funf vor ")) ;addHours(sentence, hours + 1); })};

  constexpr TimePeriodDef const *times[12][2] = {
      {&TPD_00_UHR, &TPD_00_UM},
      {&TPD_05},
      {&TPD_10},
      {&TPD_15_NACH, &TPD_15_KUCHEN},
      {&TPD_20_HALB, &TPD_20_ZWANZIG},
      {&TPD_25},
      {&TPD_30},
      {&TPD_35},
      {&TPD_40_HALB, &TPD_40_ZWANZIG},
      {&TPD_45_VOR, &TPD_45_KUCHEN},
      {&TPD_50},
      {&TPD_55},
  };

  constexpr TimeDefs convertToCharArray()
  {
    TimeDefs defs = {};
    for (int i = 0; i < 12; i++)
    {
      auto a = times[i][0];
      auto b = times[i][1];
      defs.periods[i][0] = a ? (*a).name : nullptr;
      defs.periods[i][1] = b ? (*b).name : nullptr;
    }
    return defs;
  }

  constexpr TimeDefs CONFIG = convertToCharArray();

  /**
   * @brief Converts the given time as sentence (String)
   *
   * @param hours hours of the time value
   * @param minutes minutes of the time value
   * @return String time as sentence
   */
  void timeToString(const uint8_t config[12], String &sentence, uint8_t hours, uint8_t minutes)
  {
    sentence.clear();
    sentence.concat(F("es ist "));

    auto segment = minutes / 5;
    u8 segmentConfig = config[segment];
    auto t = times[segment][segmentConfig];
    if (!t)
    {
      t = times[segment][0];
    }
    t->function(sentence, hours);
  }
}

namespace wordclock
{
  const String clockStringGerman = "espistkfunfdreiviertelzwanzigzehnuminutenullvorqkjanachhalbNELFUNFEINSZWEIUNDDREISIGVIERSECHSXYACHTSIEBENZWOLFZEHNEUNAUHR";
  String message(clockStringGerman);

  void show(Env &env, const uint8_t config[12], uint8_t hours, uint8_t minutes, uint32_t color);
  int showStringOnClock(Env &env, const uint8_t config[12], uint8_t hours, uint8_t minutes, uint32_t color);
  void drawMinuteIndicator(Env &env, uint8_t minutes, uint32_t color);

  uint8_t WordClockHandler::toConfig(const WordClockConfig &modeConfig, Env &env, uint64_t config[], const uint8_t emptyConfigs)
  {
    if (emptyConfigs < 1)
    {
      return 0;
    }
    auto &config0 = config[0];
    config0 = 0;
    for (int i = 11; i >= 0; i--)
    {
      config0 = config0 << 2;
      config0 = config0 | (modeConfig.config[i] & 0b11);
    }

    return 1;
  }

  void WordClockHandler::fromConfig(WordClockConfig &modeConfig, Env &env, const uint64_t config[], const uint8_t usedConfigs)
  {
    auto config0 = 0;
    if (usedConfigs == 1)
    {
      config0 = config[0];
    }

    for (int i = 0; i < 12; i++)
    {
      modeConfig.config[i] = (config0 & 0b11);
      config0 = config0 >> 2;
    }
  }

  void WordClockHandler::modeToJson(const WordClockConfig &clock, Env &env, JsonObject data)
  {
    auto timesJson = data[F("times")].to<JsonObject>();
    for (int c = 0; c < 12; c++)
    {
      auto ta = timedef::CONFIG.periods[c];
      if (ta[1])
      {
        timesJson[String(c * 5)] = ta[clock.config[c]];
      }
    }
  }

  void WordClockHandler::configToJson(Env &env, JsonObject config)
  {
    auto timesJson = config[F("times")].to<JsonObject>();
    for (int c = 0; c < 12; c++)
    {
      auto ta = timedef::CONFIG.periods[c];
      if (ta[1])
      {
        auto timeArray = timesJson[String(c * 5)].to<JsonArray>();
        timeArray.add(ta[0]);
        timeArray.add(ta[1]);
      }
    }
  }

  void WordClockHandler::modeFromJson(WordClockConfig &config, Env &env, JsonObjectConst doc)
  {
    JsonVariantConst clockConfig = doc[F("times")];
    if (!clockConfig.isNull())
    {
      JsonObjectConst timesJson = clockConfig.as<JsonObjectConst>();
      for (int i = 0; i < 12; i++)
      {
        auto ta = timedef::CONFIG.periods[i];
        if (!ta[1])
        {
          continue;
        }
        JsonVariantConst timeVariant = timesJson[String(i * 5)];
        if (timeVariant.isNull())
        {
          continue;
        }
        auto time = timeVariant.as<String>();
        if (time.compareTo(ta[0]) == 0)
        {
          config.config[i] = 0;
        }
        else if (time.compareTo(ta[1]) == 0)
        {
          config.config[i] = 1;
        }
      }
    }
  }

  void WordClockHandler::onActivate(WordClockConfig &modeConfig, Env &env)
  {
    env.ledmatrix.gridFlush();
    env.ledmatrix.setBrightness(modeConfig.brightness);
  }

  uint32_t WordClockHandler::onLoop(WordClockConfig &modeConfig, Env &env, unsigned long millis)
  {
    env.ledmatrix.gridFlush();
    show(env, modeConfig.config, env.hours(), env.minutes(), modeConfig.color);
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
    env.logger.logFormatted(F("time (%d:%d) as String: %s"), hours, minutes, message.c_str());

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
          env.logger.logFormatted(F("word is not possible to show on clock: %s"), word.c_str());
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