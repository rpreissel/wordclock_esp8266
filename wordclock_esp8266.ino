/**
 * Wordclock 2.0 - Wordclock with ESP8266 and NTP time update
 *
 * created by techniccontroller 04.12.2021
 *
 * components:
 * - ESP8266
 * - Neopixelstrip
 *
 * Board settings:
 * - Board: NodeMCU 1.0 (ESP-12E Module)
 * - Flash Size: 4MB (FS:2MB OTA:~1019KB)
 * - Upload Speed: 115200
 *
 *
 * with code parts from:
 * - Adafruit NeoPixel strandtest.ino, https://github.com/adafruit/Adafruit_NeoPixel/blob/master/examples/strandtest/strandtest.ino
 * - Esp8266 und Esp32 webserver https://fipsok.de/
 * - https://github.com/pmerlin/PMR-LED-Table/blob/master/tetrisGame.ino
 * - https://randomnerdtutorials.com/wifimanager-with-esp8266-autoconnect-custom-parameter-and-manage-your-ssid-and-password/
 *
 */

#include "secrets.h" // rename the file example_secrets.h to secrets.h after cloning the project. More information in README.md
#include <LittleFS.h>
#include <Adafruit_GFX.h>       // https://github.com/adafruit/Adafruit-GFX-Library
#include <Adafruit_NeoMatrix.h> // https://github.com/adafruit/Adafruit_NeoMatrix
#include <Adafruit_NeoPixel.h>  // NeoPixel library used to run the NeoPixel LEDs: https://github.com/adafruit/Adafruit_NeoPixel
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <ESP8266WebServer.h>
#include <uri/UriBraces.h>
#include "Base64.h" // copied from https://github.com/Xander-Electronics/Base64
#include <DNSServer.h>
#include <WiFiManager.h> //https://github.com/tzapu/WiFiManager WiFi Configuration Magic
#include <EEPROM.h>      //from ESP8266 Arduino Core (automatically installed when ESP8266 was installed via Boardmanager)

#include <pins_arduino.h>

// own libraries
#include "udplogger.h"
#include "LittleFSServer.h"
#include "ntp_client_plus.h"
#include "ledmatrix.h"
#include "tools.h"
#include "state.h"

// ----------------------------------------------------------------------------------
//                                        CONSTANTS
// ----------------------------------------------------------------------------------

#define NEOPIXELPIN 12 // pin to which the NeoPixels are attached
#define NUMPIXELS 125  // number of pixels attached to Attiny85

#define PERIOD_HEARTBEAT 1000
#define PERIOD_NTPUPDATE 30000
#define PERIOD_MATRIXUPDATE 100

#define CURRENT_LIMIT_LED 2500 // limit the total current sonsumed by LEDs (mA)

#define DEFAULT_SMOOTHING_FACTOR 0.5

// number of colors in colors array
#define NUM_COLORS 7

// own datatype for state machine states

// ports
const unsigned int localPort = 2390;
const unsigned int HTTPPort = 80;
const unsigned int logMulticastPort = 8123;
const unsigned int DNSPort = 53;

// ip addresses for multicast logging
IPAddress logMulticastIP = IPAddress(230, 120, 10, 2);

// ip addresses for Access Point
IPAddress IPAdress_AccessPoint(192, 168, 10, 2);
IPAddress Gateway_AccessPoint(192, 168, 10, 0);
IPAddress Subnetmask_AccessPoint(255, 255, 255, 0);

// hostname
const String hostname = "wordclock2";

// URL DNS server
const char WebserverURL[] = "www.wordclock.local";

// ----------------------------------------------------------------------------------
//                                        GLOBAL VARIABLES
// ----------------------------------------------------------------------------------

// Webserver
ESP8266WebServer server(HTTPPort);

// DNS Server
DNSServer DnsServer;

// Wifi server. keep around to support resetting.
WiFiManager wifiManager;

// LittleFSServer
LittleFSServer littleFSServer(server);

// When we setup the NeoPixel library, we tell it how many pixels, and which pin to use to send signals.
// Note that for older NeoPixel strips you might need to change the third parameter--see the strandtest
// example for more information on possible values.
Adafruit_NeoMatrix matrix = Adafruit_NeoMatrix(LEDMatrix::width, LEDMatrix::height + 1, NEOPIXELPIN,
                                               NEO_MATRIX_TOP + NEO_MATRIX_LEFT +
                                                   NEO_MATRIX_ROWS + NEO_MATRIX_ZIGZAG,
                                               NEO_GRB + NEO_KHZ800);

// seven predefined colors24bit (green, red, yellow, purple, orange, lightgreen, blue)
const uint32_t colors24bit[NUM_COLORS] = {
    LEDMatrix::Color24bit(0, 255, 0),
    LEDMatrix::Color24bit(255, 0, 0),
    LEDMatrix::Color24bit(200, 200, 0),
    LEDMatrix::Color24bit(255, 0, 200),
    LEDMatrix::Color24bit(255, 128, 0),
    LEDMatrix::Color24bit(0, 128, 0),
    LEDMatrix::Color24bit(0, 0, 255)};

// timestamp variables
long lastheartbeat = millis();                             // time of last heartbeat sending
long lastNTPUpdate = millis() - (PERIOD_NTPUPDATE - 5000); // time of last NTP update
long lastAnimationStep = millis();                         // time of last Matrix update

// Create necessary global objects

UDPLogger logger;
WiFiUDP NTPUDP;
NTPClientPlus ntp = NTPClientPlus(NTPUDP, "pool.ntp.org", 1, true);
LEDMatrix ledmatrix = LEDMatrix(&matrix, 40, &logger);

float filterFactor = DEFAULT_SMOOTHING_FACTOR; // stores smoothing factor for led transition
bool apmode = false;                           // stores if WiFi AP mode is active

// Watchdog counter to trigger restart if NTP update was not possible 30 times in a row (5min)
int watchdogCounter = 30;

// ----------------------------------------------------------------------------------
//                                        SETUP
// ----------------------------------------------------------------------------------

void setup()
{
  // put your setup code here, to run once:
  Serial.begin(115200);
  delay(100);
  Serial.println();
  Serial.printf("\nSketchname: %s\nBuild: %s\n", (__FILE__), (__TIMESTAMP__));
  Serial.println();

  // setup Matrix LED functions
  ledmatrix.setupMatrix();
  ledmatrix.setCurrentLimit(CURRENT_LIMIT_LED);

  // Turn on minutes leds (blue)
  ledmatrix.setMinIndicator(15, colors24bit[6]);
  ledmatrix.drawOnMatrixInstant();

  /** Use WiFiMaanger for handling initial Wifi setup **/

  /*
    // Local intialization. Once its business is done, there is no need to keep it around


    // Uncomment and run it once, if you want to erase all the stored information
    //wifiManager.resetSettings();

    // set custom ip for portal
    //wifiManager.setAPStaticIPConfig(IPAdress_AccessPoint, Gateway_AccessPoint, Subnetmask_AccessPoint);

    // fetches ssid and pass from eeprom and tries to connect
    // if it does not connect it starts an access point with the specified name
    // here "wordclockAP"
    // and goes into a blocking loop awaiting configuration
    wifiManager.autoConnect(AP_SSID);

    // if you get here you have connected to the WiFi
    Serial.println("Connected.");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());

    // Turn off minutes leds
    ledmatrix.setMinIndicator(15, 0);
    ledmatrix.drawOnMatrixInstant();
  */

  /** (alternative) Use directly STA/AP Mode of ESP8266   **/

  // We start by connecting to a WiFi network
  Serial.print("Connecting to ");
  Serial.println(WIFI_SSID);

  // We start by connecting to a WiFi network
  WiFi.mode(WIFI_STA);
  // Set new hostname
  WiFi.hostname(hostname.c_str());
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  // wifi_station_set_hostname("esplamp");

  int timeoutcounter = 0;
  while (WiFi.status() != WL_CONNECTED && timeoutcounter < 30)
  {
    ledmatrix.setMinIndicator(15, colors24bit[6]);
    ledmatrix.drawOnMatrixInstant();
    delay(250);
    ledmatrix.setMinIndicator(15, 0);
    ledmatrix.drawOnMatrixInstant();
    delay(250);
    Serial.print(".");
    timeoutcounter++;
  }

  // start request of program
  if (WiFi.status() == WL_CONNECTED)
  { // Check WiFi connection status
    Serial.println("");

    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
    WiFi.setAutoReconnect(true);
    WiFi.persistent(true);
  }
  else
  {
    // no wifi found -> open access point
    WiFi.mode(WIFI_AP);
    WiFi.softAPConfig(IPAdress_AccessPoint, Gateway_AccessPoint, Subnetmask_AccessPoint);
    WiFi.softAP(AP_SSID, AP_PASS);
    apmode = true;

    // start DNS Server
    DnsServer.setTTL(300);
    DnsServer.start(DNSPort, WebserverURL, IPAdress_AccessPoint);

    IPAddress myIP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(myIP);
  }

  // init ESP8266 File manager (LittleFS)
  littleFSServer.setup();

  // setup OTA
  setupOTA(hostname);

  server.begin();

  // create UDP Logger to send logging messages via UDP multicast
  logger = UDPLogger(WiFi.localIP(), logMulticastIP, logMulticastPort);
  logger.setName("Wordclock 2.0");
  logger.logFormatted(F("Start program\n"));
  delay(10);
  logger.logFormatted(F("Sketchname: %s"),__FILE__);
  delay(10);
  logger.logFormatted(F("Build: %s"), __TIMESTAMP__);
  delay(10);
  logger.logFormatted(F("IP: %s"), WiFi.localIP().toString().c_str());
  delay(10);
  logger.logFormatted(F("Reset Reason: %s"), ESP.getResetReason().c_str());

  if (!ESP.getResetReason().equals("Software/System restart"))
  {
    // test quickly each LED
    for (int r = 0; r < LEDMatrix::height; r++)
    {
      for (int c = 0; c < LEDMatrix::width; c++)
      {
        matrix.fillScreen(0);
        matrix.drawPixel(c, r, LEDMatrix::color24to16bit(colors24bit[2]));
        matrix.show();
        delay(10);
      }
    }

    // clear Matrix
    matrix.fillScreen(0);
    matrix.show();
    delay(200);

    // display IP
    uint8_t address = WiFi.localIP()[3];
    ledmatrix.printChar(1, 0, 'I', colors24bit[2]);
    ledmatrix.printChar(5, 0, 'P', colors24bit[2]);
    ledmatrix.printNumber(0, 6, (address / 100), colors24bit[2]);
    ledmatrix.printNumber(4, 6, (address / 10) % 10, colors24bit[2]);
    ledmatrix.printNumber(8, 6, address % 10, colors24bit[2]);
    ledmatrix.drawOnMatrixInstant();
    delay(2000);

    // clear matrix
    ledmatrix.gridFlush();
    ledmatrix.drawOnMatrixInstant();
  }


  // setup NTP
  ntp.setupNTPClient();
  logger.logFormatted(F("NTP running"));
  logger.logFormatted(F("Time: %s"), ntp.getFormattedTime());
  logger.logFormatted(F("TimeOffset (seconds): %d"), ntp.getTimeOffset());

  config::init(server, ledmatrix, logger,ntp);
}

// ----------------------------------------------------------------------------------
//                                        LOOP
// ----------------------------------------------------------------------------------

void loop()
{
  // handle OTA
  handleOTA();

  // handle Webserver
  server.handleClient();

  // send regularly heartbeat messages via UDP multicast
  if (millis() - lastheartbeat > PERIOD_HEARTBEAT)
  {
    logger.logFormatted(F("Heartbeat, state: %s, FreeHeap: %d, HeapFrag: %d, MaxFreeBlock: %d"), config::currentModeDescription().c_str(), ESP.getFreeHeap(), ESP.getHeapFragmentation(),ESP.getMaxFreeBlockSize() );
    lastheartbeat = millis();

    // Check wifi status (only if no apmode)
    if (!apmode && WiFi.status() != WL_CONNECTED)
    {
      Serial.println("connection lost");
      ledmatrix.gridAddPixel(0, 5, colors24bit[1]);
      ledmatrix.drawOnMatrixInstant();
    }
  }

  // handle mode behaviours (trigger loopCycles of different modes depending on current mode)
  config::loop(millis());

  // periodically write colors to matrix
  if (millis() - lastAnimationStep > PERIOD_MATRIXUPDATE)
  {
    ledmatrix.drawOnMatrixSmooth(filterFactor);
    lastAnimationStep = millis();
  }

  // NTP time update
  if (millis() - lastNTPUpdate > PERIOD_NTPUPDATE)
  {
    int res = ntp.updateNTP();
    if (res == 0)
    {
      ntp.calcDate();
      logger.logFormatted(F("NTP-Update successful"));
      logger.logFormatted(F("Time: %s"), ntp.getFormattedTime().c_str());
      logger.logFormatted(F("Date: %s"), ntp.getFormattedDate().c_str());
      logger.logFormatted(F("Day of Week (Mon=1, Sun=7): %d"),ntp.getDayOfWeek());
      logger.logFormatted(F("TimeOffset (seconds): %d"), ntp.getTimeOffset());
      logger.logFormatted(F("Summertime: %d")), ntp.updateSWChange();
    
      lastNTPUpdate = millis();
      watchdogCounter = 30;
    }
    else if (res == -1)
    {
      logger.logFormatted(F("NTP-Update not successful. Reason: Timeout"));
      lastNTPUpdate += 10000;
      watchdogCounter--;
    }
    else if (res == 1)
    {
      logger.logFormatted(F("NTP-Update not successful. Reason: Too large time difference"));
      logger.logFormatted(F("Time: %s"), ntp.getFormattedTime().c_str());
      logger.logFormatted(F("Date: %s"), ntp.getFormattedDate().c_str());
      logger.logFormatted(F("Day of Week (Mon=1, Sun=7): %d"),ntp.getDayOfWeek());
      logger.logFormatted(F("TimeOffset (seconds): %d"), ntp.getTimeOffset());
      logger.logFormatted(F("Summertime: %d")), ntp.updateSWChange();
      lastNTPUpdate += 10000;
      watchdogCounter--;
    }
    else
    {
      logger.logFormatted(F("NTP-Update not successful. Reason: NTP time not valid (<1970)"));
      lastNTPUpdate += 10000;
      watchdogCounter--;
    }

    logger.logFormatted(F("Watchdog Counter: %d"), watchdogCounter);
    if (watchdogCounter <= 0)
    {
      logger.logFormatted(F("Trigger restart due to watchdog..."));
      delay(100);
      ESP.restart();
    }
  }
}

// ----------------------------------------------------------------------------------
//                                        OTHER FUNCTIONS
// ----------------------------------------------------------------------------------

/**
 * @brief Convert Integer to String with leading zero
 *
 * @param value
 * @return String
 */
String leadingZero2Digit(int value)
{
  String msg = "";
  if (value < 10)
  {
    msg = "0";
  }
  msg += String(value);
  return msg;
}


// setup Arduino OTA
void setupOTA(String hostname)
{
  // Port defaults to 8266
  // ArduinoOTA.setPort(8266);

  // Hostname defaults to esp8266-[ChipID]
  ArduinoOTA.setHostname(hostname.c_str());

  // No authentication by default
  ArduinoOTA.setPassword("admin");

  // Password can be set with it's md5 value as well
  // MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
  // ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");

  ArduinoOTA.onStart([]()
                     {
                       String type;
                       if (ArduinoOTA.getCommand() == U_FLASH)
                       {
                         type = "sketch";
                       }
                       else
                       { // U_FS
                         type = "filesystem";
                       }

                       // NOTE: if updating FS this would be the place to unmount FS using FS.end()
                       // Serial.println("Start updating " + type);
                     });
  ArduinoOTA.onEnd([]()
                   {
                     // Serial.println("\nEnd");
                   });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total)
                        {
                          // Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
                        });
  ArduinoOTA.onError([](ota_error_t error)
                     {
    //Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      //Serial.println("Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) {
      //Serial.println("Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
      //Serial.println("Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
      //Serial.println("Receive Failed");
    } else if (error == OTA_END_ERROR) {
      //Serial.println("End Failed");
    } });
  ArduinoOTA.begin();
}

void handleOTA()
{
  // handle OTA
  ArduinoOTA.handle();
}