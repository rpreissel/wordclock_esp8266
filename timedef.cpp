#include <variant>
#include <tuple>
#include "timedef.h"
#include <Arduino.h>

namespace timedef
{
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

    TimeDefs getConfig()
    {
        return CONFIG;
    }

    /**
     * @brief Converts the given time as sentence (String)
     *
     * @param hours hours of the time value
     * @param minutes minutes of the time value
     * @return String time as sentence
     */
    void timeToString(u32 config, String &sentence, uint8_t hours, uint8_t minutes)
    {
        Serial.println(hours);
        Serial.println(minutes);

        sentence.clear();
        sentence.concat(F("es ist "));

        auto segment = minutes / 5;
        u8 segmentConfig = (config >> (segment * 2)) & 0b00000011;
        auto t = times[segment][segmentConfig];
        if (!t)
        {
            t = times[segment][0];
        }
        t->function(sentence, hours);

        Serial.println(sentence);
    }
}