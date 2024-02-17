#include <ranges>
#include <tuple>
#include <variant>
#include <cstdarg>
#include "wordclock.h"
#include "timedef.h"
#include "tools.h"

const String clockStringGerman = "espistkfunfdreiviertelzwanzigzehnuminutenullvorqkjanachhalbNELFUNFEINSZWEIUNDDREISIGVIERSECHSXYACHTSIEBENZWOLFZEHNEUNAUHR";



WordClock::WordClock(LEDMatrix &ledmatrix, UDPLogger &logger)
    : ledmatrix(ledmatrix), logger(logger)
{
  message.reserve(LEDMatrix::width * LEDMatrix::height);
}

void WordClock::show(const uint8_t config[12], uint8_t hours, uint8_t minutes, uint32_t color)
{
  showStringOnClock(config, hours, minutes, color);
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
int WordClock::showStringOnClock(const uint8_t config[12], uint8_t hours, uint8_t minutes, uint32_t color)
{
  timedef::timeToString(config, message, hours, minutes);
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
      positionOfWord = clockStringGerman.indexOf(word, lastLetterClock);

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