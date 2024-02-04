#include <ranges>
#include <tuple>
#include <variant>
#include <cstdarg>
#include "wordclock.h"
#include "tools.h"

namespace words
{
  const String clockStringGerman = "espistkfunfdreiviertelzwanzigzehnuminutenullvorqkjanachhalbNELFUNFEINSZWEIUNDDREISIGVIERSECHSXYACHTSIEBENZWOLFZEHNEUNAUHR";

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

  typedef std::function<void(String &sentence, uint8_t hours)> TimePeriodFunction;

  class TimePeriodDef
  {
  public:
    TimePeriodDef(String name, const TimePeriodFunction function)
        : name(name), function(function) {}

    const String name;
    const TimePeriodFunction function;
  };

  const TimePeriodDef TPD_00_UHR =
      {"10 Uhr",
       ([](String &sentence, uint8_t hours)
        { addHours(sentence, hours, true); })};
  const TimePeriodDef TPD_00_UM =
      {"um 10",
       ([](String &sentence, uint8_t hours)
        {sentence.concat(F("um "));addHours(sentence, hours); })};
  const TimePeriodDef TPD_05 =
      {"5 nach 10",
       ([](String &sentence, uint8_t hours)
        {sentence.concat(F("funf nach ")) ;addHours(sentence, hours); })};
  const TimePeriodDef TPD_10 =
      {"10 nach 10",
       ([](String &sentence, uint8_t hours)
        {sentence.concat(F("zehn nach ")) ;addHours(sentence, hours); })};
  const TimePeriodDef TPD_15_NACH =
      {"viertel nach 10",
       ([](String &sentence, uint8_t hours)
        {sentence.concat(F("viertel nach ")) ;addHours(sentence, hours); })};
  const TimePeriodDef TPD_15_KUCHEN =
      {"viertel 11",
       ([](String &sentence, uint8_t hours)
        {sentence.concat(F("viertel ")) ;addHours(sentence, hours + 1); })};
  const TimePeriodDef TPD_20_HALB =
      {"10 vor halb 11",
       ([](String &sentence, uint8_t hours)
        {sentence.concat(F("zehn vor halb ")) ;addHours(sentence, hours + 1); })};
  const TimePeriodDef TPD_20_ZWANZIG =
      {"zwanzig nach 10",
       ([](String &sentence, uint8_t hours)
        {sentence.concat(F("zwanzig nach ")); addHours(sentence, hours); })};
  const TimePeriodDef TPD_25 =
      {"5 vor halb 11",
       ([](String &sentence, uint8_t hours)
        {sentence.concat(F("funf vor ")) ;addHours(sentence, hours + 1); })};
  const TimePeriodDef TPD_30 =
      {"halb 11",
       ([](String &sentence, uint8_t hours)
        {sentence.concat(F("halb ")) ;addHours(sentence, hours + 1); })};
  const TimePeriodDef TPD_35 =
      {"5 nach halb 11",
       ([](String &sentence, uint8_t hours)
        {sentence.concat(F("funf nach halb ")) ;addHours(sentence, hours + 1); })};
  const TimePeriodDef TPD_40_HALB =
      {"10 nach halb 11",
       ([](String &sentence, uint8_t hours)
        {sentence.concat(F("zehn nach halb ")) ;addHours(sentence, hours + 1); })};
  const TimePeriodDef TPD_40_ZWANZIG =
      {"20 vor 11",
       ([](String &sentence, uint8_t hours)
        {sentence.concat(F("zwanzig vor ")) ;addHours(sentence, hours + 1); })};
  const TimePeriodDef TPD_45_VOR =
      {"viertel vor 11",
       ([](String &sentence, uint8_t hours)
        {sentence.concat(F("viertel vor ")) ;addHours(sentence, hours + 1); })};
  const TimePeriodDef TPD_45_KUCHEN =
      {"dreiviertel 11",
       ([](String &sentence, uint8_t hours)
        {sentence.concat(F("dreiviertel ")) ;addHours(sentence, hours + 1); })};
  const TimePeriodDef TPD_50 =
      {"10 vor 11",
       ([](String &sentence, uint8_t hours)
        {sentence.concat(F("zehn vor ")) ;addHours(sentence, hours + 1); })};
  const TimePeriodDef TPD_55 =
      {"5 vor 11",
       ([](String &sentence, uint8_t hours)
        {sentence.concat(F("funf vor ")) ;addHours(sentence, hours + 1); })};

  const TimePeriodDef *times[12][2] = {
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

  /**
 * @brief Converts the given time as sentence (String)
 *
 * @param hours hours of the time value
 * @param minutes minutes of the time value
 * @return String time as sentence
 */
void timeToString(u32 config, String& sentence,uint8_t hours, uint8_t minutes)
{
  Serial.println(hours);
  Serial.println(minutes);

  sentence.clear();
  sentence.concat(F("es ist "));

  auto segment = minutes / 5;
  u8 segmentConfig = (config >> (segment * 2)) & 0b00000011; 
  auto t = words::times[segment][segmentConfig];
  if(!t) {
    t = words::times[segment][0];
  }
  t->function(sentence, hours);

  Serial.println(sentence);
}
}
WordClock::WordClock(LEDMatrix &ledmatrix, UDPLogger &logger)
    : ledmatrix(ledmatrix), logger(logger)
{
  message.reserve(LEDMatrix::width * LEDMatrix::height);
}

void WordClock::show(uint8_t hours, uint8_t minutes, uint32_t color)
{
  showStringOnClock(hours, minutes, color);
  drawMinuteIndicator(minutes, color);
}

/**
 * @brief control the four minute indicator LEDs
 *
 * @param minutes minutes to be displayed [0 ... 59]
 * @param color 24bit color value
 */
void WordClock::drawMinuteIndicator(uint8_t minutes, uint32_t color)
{
  // separate LEDs for minutes in an additional row
  {
    switch (minutes % 5)
    {
    case 0:
      break;

    case 1:
      ledmatrix.setMinIndicator(0b1000, color);
      break;

    case 2:
      ledmatrix.setMinIndicator(0b1100, color);
      break;

    case 3:
      ledmatrix.setMinIndicator(0b1110, color);
      break;

    case 4:
      ledmatrix.setMinIndicator(0b1111, color);
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
int WordClock::showStringOnClock(uint8_t hours,uint8_t minutes, uint32_t color)
{
  words::timeToString(0b00'00'00'00'00'00'00'00'01'00'00'00, message, hours, minutes);
  logger.logString("time as String: " + String(message));

  int messageStart = 0;
  String word = "";
  int lastLetterClock = 0;
  int positionOfWord = 0;
  int nextSpace = 0;
  int index = 0;

  // add space on the end of message for splitting
  message.concat(F(" "));

  // empty the targetgrid
  ledmatrix.gridFlush();

  while (true)
  {
    // extract next word from message
    word = split(message, ' ', index);
    index++;

    if (word.length() > 0)
    {
      // find word in clock string
      positionOfWord = words::clockStringGerman.indexOf(word, lastLetterClock);

      if (positionOfWord >= 0)
      {
        // word found on clock -> enable leds in targetgrid
        for (int i = 0; i < word.length(); i++)
        {
          int x = (positionOfWord + i) % LEDMatrix::width;
          int y = (positionOfWord + i) / LEDMatrix::width;
          ledmatrix.gridAddPixel(x, y, color);          
        }
        // remember end of the word on clock
        lastLetterClock = positionOfWord + word.length();
      }
      else
      {
        // word is not possible to show on clock
        logger.logString("word is not possible to show on clock: " + String(word));
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


