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
      {"zwanzig nach 10",
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

  void WordClockHandler::toJson(const WordClockConfig &clock, Env &env, JsonObject data, JsonObject config)
  {
    baseConfigToJson(clock, env, data, config);

    auto timesArray = data[F("times")].to<JsonArray>();
    auto configArray = config[F("times")].to<JsonArray>();
    for (int c = 0; c < 12; c++)
    {
      timesArray.add(clock.config[c]);

      auto timeArray = configArray.add<JsonArray>();
      auto ta = timedef::CONFIG.periods[c];
      if (ta[0])
      {
        timeArray.add(ta[0]);
        if (ta[1])
        {
          timeArray.add(ta[1]);
        }
      }
    }
    data[F("fixed")] = clock.fixed;
    data[F("hours")] = clock.hours;
    data[F("minutes")] = clock.minutes;
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

  uint32_t WordClockHandler::onActivate(WordClockConfig &modeConfig, Env &env)
  {
    env.ledmatrix.gridFlush();
    env.ledmatrix.setBrightness(modeConfig.brightness);
    return 1;
  }

  uint32_t WordClockHandler::onLoop(WordClockConfig &modeConfig, Env &env, unsigned long millis)
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