#include "WifiSetup.h"

WifiSetup wifi;

void setup() {
    Serial.begin(9600);
  
    wifi.addWebServerOn("/api", HTTP_GET, handleWebRequest);
    wifi.doWifiConnection();
}

void handleWebRequest() {
    //wifi.getWebServerArgument("type...");
    wifi.sendTextWebServer("OK");
}

void loop() {
  // put your main code here, to run repeatedly:
    wifi.update();
}
