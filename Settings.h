#ifndef SETTINGS_H
#define SETTINGS_H
#include "Arduino.h"
#include <ArduinoJson.h>

class Settings {
    // note! make sure to expand the StaticJsonDocument object memory size if you add properties

  public:
    int maxLeft = -2000;
    int maxRight = 2000;
    int maxHeight = 2000;
    int minHeight = -2000;
    byte maxLaserPower[3] = { 92, 92, 82 }; //rgb

    void saveSettingsInEEPROM();
    void getAndsetFromEEPROM();
    void setFromJson(StaticJsonDocument<300> doc);
    void toJson();
    void saveSettings(String json);
};

#endif
