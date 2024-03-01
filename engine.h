#ifndef MODES_ENGINE_H
#define MODES_ENGINE_H

#include <ESP8266WebServer.h>

#include "udplogger.h"
#include "ledmatrix.h"
#include "ntp_client_plus.h"

namespace modes
{
    void init(ESP8266WebServer &server, LEDMatrix& ledmatrix, UDPLogger &logger, NTPClientPlus &ntp);
    String currentModeDescription();
    void loop(unsigned long millis);
}
#endif