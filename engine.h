#ifndef MODES_ENGINE_H
#define MODES_ENGINE_H

#include <ESP8266WebServer.h>

#include "udplogger.h"
#include "ledmatrix.h"
#include "ntp_client_plus.h"

namespace modes
{
    enum ResetFlags : uint8_t
    {
        NOTHING = 0,
        WIFI = 1,
        ESP = 2,
    };
    using ResetCallback = std::function<void(uint8_t)>;

    void init(ESP8266WebServer &server,
              LEDMatrix &ledmatrix,
              UDPLogger &logger,
              NTPClientPlus &ntp,
              ResetCallback resetCallback);
    void currentModeDescription(String &desc);
    void loop(unsigned long millis);
}
#endif