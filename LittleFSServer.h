#ifndef LITTLEFSSERVER_H
#define LITTLEFSSERVER_H

#include <ESP8266WebServer.h>

class LittleFSServer
{
public:
    LittleFSServer(ESP8266WebServer& server): server(server) {}
    void setup();
private:
    bool handleList();
    bool handleFile(String &&path);
    void handleUpload();
    void formatFS();
    void sendResponce();
    ESP8266WebServer& server;
};

#endif