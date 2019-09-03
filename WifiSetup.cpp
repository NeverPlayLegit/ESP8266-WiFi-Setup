#include "WifiSetup.h"

WifiSetup::WifiSetup() {
    saveConfig();
    SPIFFS.begin();
    loadConfig();
}

void WifiSetup::parseBytes(const char* str, char sep, byte* bytes, int maxBytes, int base) {
  for (int i = 0; i < maxBytes; i++) {
    bytes[i] = strtoul(str, NULL, base);  // Convert byte
    str = strchr(str, sep);               // Find next separator
    if (str == NULL || *str == '\0') {
      break;                            // No more separators, exit
    }
    str++;                                // Point to next character after separator
  }
}

void WifiSetup::loadConfig() {
    EEPROM.begin(4095);
    EEPROM.get(0, config);
    EEPROM.end();
}

void WifiSetup::saveConfig() {
    EEPROM.begin(4095);
    EEPROM.put(0, config);
    EEPROM.commit();
    EEPROM.end();
}

void WifiSetup::handleRoot() {
    File file = SPIFFS.open("/index.htm", "r");
    size_t sent = serverWeb.streamFile(file, "text/html");
    file.close();
}

IPAddress WifiSetup::parseIP(const char *str) {
    byte b[4];
    parseBytes(str, '.', b, 4, 10);
    return IPAddress(b[0], b[1], b[2], b[3]);
}

void WifiSetup::doWifiConnection() {
    WiFi.disconnect(true);
    WiFi.softAPdisconnect(true);
    WiFi.mode(WIFI_AP);
    WiFi.persistent(false);
    serverDNS.stop();

    if(config.mode == 0) {
        IPAddress ip(192, 168, 4, 1);
        IPAddress ip2(255, 255, 255, 0);
        WiFi.softAPConfig(ip, ip, ip2);
        WiFi.softAP(apSsid, apPassword);
        serverDNS.setErrorReplyCode(DNSReplyCode::NoError);
        serverDNS.start(53, "*", ip);
    } else {

        Serial.println("Try connect");
        WiFi.config(config.ip, config.gateway, config.subnet);
        WiFi.begin(config.ssid, config.password);
        int timer = 0;
        while(WiFi.status() != WL_CONNECTED) {
            timer++;
            if(timer > 10) {
                Serial.println("Connect failed");
                config.mode = 0;
                doWifiConnection();
                return;
            }
            delay(1000);
        }
    }

    std::function<void(void)> _handleRoot = std::bind(&WifiSetup::handleRoot, this);
    std::function<void(void)> _handleSettings = std::bind(&WifiSetup::handleSettings, this);
    if(config.mode == 0) addWebServerOn("/generate_204", HTTP_POST, _handleRoot);
    if(config.mode == 0) addWebServerOn("/success.txt", HTTP_POST, _handleRoot);
    if(config.mode == 0) addWebServerOn("/hotspot-detect.html", HTTP_POST, _handleRoot);
    if(config.mode == 0) addWebServerOn("/ncsi.txt", HTTP_POST, _handleRoot);
    if(config.mode == 0) addWebServerOn("/conecttest.txt", HTTP_POST, _handleRoot);
    if(config.mode == 0) addWebServerOn("/hotspot.txt", HTTP_POST, _handleRoot);
    addWebServerOn("/wifi", HTTP_POST, _handleSettings);
    serverWeb.onNotFound(_handleRoot);
    serverWeb.serveStatic("/", SPIFFS, "/", "max-age=86400");
    serverWeb.begin();
}


void WifiSetup::handleSettings() {
    config.mode = strcmp(serverWeb.arg("mode").c_str(), "0") == 0 ? 0 : 1;
    config.staticIP = strcmp(serverWeb.arg("staticIP").c_str(), "0") == 0 ? 0 : 1;

    config.ip = parseIP(serverWeb.arg("ip").c_str());
    config.gateway = parseIP(serverWeb.arg("gateway").c_str());
    config.subnet = parseIP(serverWeb.arg("subnet").c_str());

    strcpy(config.ssid, serverWeb.arg("ssid").c_str());
    strcpy(config.password, serverWeb.arg("password").c_str());

    saveConfig();
    doWifiConnection();
}

const char* WifiSetup::getWebServerArgument(const char* arg) {
    return serverWeb.arg(arg).c_str();
}

void WifiSetup::sendTextWebServer(const char* text) {
    serverWeb.send(200, "text/html", text);
}

typedef std::function<void(void)> THandlerFunction;
void WifiSetup::addWebServerOn(const String &uri, HTTPMethod method, THandlerFunction fn) {
    serverWeb.on(uri, method, fn);
}

void WifiSetup::update() {
    if(config.mode == 0) serverDNS.processNextRequest();
    serverWeb.handleClient();
}
