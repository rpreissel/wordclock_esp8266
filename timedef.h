#ifndef TIMEDEF_H
#define TIMEDEF_H

#include <WString.h>

namespace timedef
{
    

    struct TimeDefs
    {
        const char *periods[12][2];
    };


    TimeDefs getConfig();
    void timeToString(u32 config, String &sentence, uint8_t hours, uint8_t minutes);
}

#endif