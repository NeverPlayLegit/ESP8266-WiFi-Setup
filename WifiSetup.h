#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <FS.h>
#include <EEPROM.h>
#include "DNSServer.h"

#include <functional>
#include <memory>

struct WifiConfig {
    byte mode;
    byte staticIP;
    char ssid[32];
    char password[64];
    IPAddress ip;
    IPAddress subnet;
    IPAddress gateway;
};

class WifiSetup {
private:
    const char *apSsid = "test";
    const char *apPassword = "test7777";

    WifiConfig config;
    DNSServer serverDNS;
    ESP8266WebServer serverWeb;

    void loadConfig();
    void saveConfig();

    void handleRoot();
    void handleSettings();

    void parseBytes(const char* str, char sep, byte* bytes, int maxBytes, int base);
    IPAddress parseIP(const char* str);

public:
    WifiSetup(const char* apSsid, const char* apPassword);

    void doWifiConnection();

    const char *getWebServerArgument(const char* arg);
    void addWebServerOn(const String& uri, HTTPMethod method, std::function<void(void)> fn);
    
    ESP8266WebServer* getWebServer();

    void update();
};
