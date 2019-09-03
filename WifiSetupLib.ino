#include "WifiSetup.h"

WifiSetup wifi("test", "test7777");

void setup() {
    Serial.begin(9600);
  
    wifi.addWebServerOn("/somerequest", HTTP_GET, handleWebRequest);
    wifi.doWifiConnection();
}

void handleWebRequest() {
    //wifi.getWebServerArgument("argument");
    wifi.getWebServer()->send(200, "text/html", "Test!");
}

void loop() {
  // put your main code here, to run repeatedly:
    wifi.update();
}
