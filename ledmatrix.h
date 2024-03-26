#ifndef ledmatrix_h
#define ledmatrix_h

#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_NeoMatrix.h>
#include <tuple>
#include "udplogger.h"

#define DEFAULT_CURRENT_LIMIT 9999

class LEDMatrix
{
public:
    LEDMatrix(Adafruit_NeoMatrix *mymatrix, uint8_t mybrightness, UDPLogger *mylogger);
    static uint32_t Color24bit(uint8_t r, uint8_t g, uint8_t b);
    static uint16_t color24to16bit(uint32_t color24bit);
    static uint32_t Wheel(uint8_t WheelPos);
    static uint32_t interpolateColor24bit(uint32_t color1, uint32_t color2, float factor);
    void setupMatrix();
    void setMinIndicator(uint8_t pattern, uint32_t color);
    void gridAddPixel(uint8_t x, uint8_t y, uint32_t color);
    void gridFlush(void);
    void drawOnMatrixInstant();
    void drawOnMatrixSmooth(float factor);
    void printNumber(uint8_t xpos, uint8_t ypos, uint8_t number, uint32_t color);
    void printChar(uint8_t xpos, uint8_t ypos, char character, uint32_t color);
    void setBrightness(uint8_t mybrightness);
    void setCurrentLimit(uint16_t mycurrentLimit);

    constexpr static int width = 11;
    constexpr static int height = 11;
    const static String clockString;
    const static String clockStringUmlaut;

    uint8_t colorIndexGrid(uint8_t x, uint8_t y);
    uint8_t colorIndexMinIndicator(uint8_t x);
private:
    Adafruit_NeoMatrix *neomatrix;
    UDPLogger *logger;

    uint8_t brightness;
    uint16_t currentLimit;

    // target representation of matrix as 2D array
    uint32_t targetgrid[height][width] = {0};

    // current representation of matrix as 2D array
    uint32_t currentgrid[height][width] = {0};

    // target representation of minutes indicator leds
    uint32_t targetindicators[4] = {0, 0, 0, 0};

    // current representation of minutes indicator leds
    uint32_t currentindicators[4] = {0, 0, 0, 0};

    void drawOnMatrix(float factor);
    uint16_t calcEstimatedLEDCurrent(uint32_t color);
};


using Color = std::tuple<const char *, uint32_t>;

// seven predefined colors24bit (green, red, yellow, purple, orange, lightgreen, blue)
const Color COLORS[] = {
    {"off", LEDMatrix::Color24bit(0, 0, 0)},
    {"red", LEDMatrix::Color24bit(255, 0, 0)},
    {"rose", LEDMatrix::Color24bit(255, 0, 128)},
    {"magenta", LEDMatrix::Color24bit(255, 0, 255)},
    {"violet", LEDMatrix::Color24bit(128, 0, 255)},
    {"purple", LEDMatrix::Color24bit(128, 0, 128)},
    {"blue", LEDMatrix::Color24bit(0, 0, 255)},
    {"skyblue", LEDMatrix::Color24bit(0, 128, 255)},
    {"white", LEDMatrix::Color24bit(255, 255, 255)},
    {"lime", LEDMatrix::Color24bit(0, 255, 0)},
    {"springgreen", LEDMatrix::Color24bit(0, 255, 128)},
    {"green", LEDMatrix::Color24bit(0, 128, 0)},
    {"lawngreen", LEDMatrix::Color24bit(128, 255, 0)},
    {"yellow", LEDMatrix::Color24bit(200, 200, 0)},
    {"orange", LEDMatrix::Color24bit(255, 128, 0)}};

constexpr uint8_t NUM_COLORS  = sizeof(COLORS) / sizeof(Color);

uint32_t color(size_t index);
uint8_t colorIndex(uint32_t color);
uint8_t colorIndex(const char * name);
const char *colorName(uint8_t index);
uint32_t color(const char * name);
#endif