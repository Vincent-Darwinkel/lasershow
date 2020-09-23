#include <EEPROM.h>
#include "Settings.h"

void Settings::saveSettingsInEEPROM() {
      EEPROM.write(0, (int8_t)map(maxLeft, -2000, 2000, -128, 127));
      EEPROM.write(1, (int8_t)map(maxRight, -2000, 2000, -128, 127));
      EEPROM.write(2, (int8_t)map(maxHeight, -2000, 2000, -128, 127));
      EEPROM.write(3, (int8_t)map(minHeight, -2000, 2000, -128, 127));
      EEPROM.write(4, maxLaserPower[0]);
      EEPROM.write(5, maxLaserPower[1]);
      EEPROM.write(6, maxLaserPower[2]);
    }

void Settings::getAndsetFromEEPROM() {
      maxLeft = map((int8_t)EEPROM.read(0), -128, 127, -2000, 2000);
      maxRight = map((int8_t)EEPROM.read(1), -128, 127, -2000, 2000);
      maxHeight = map((int8_t)EEPROM.read(2), -128, 127, -2000, 2000);
      minHeight = map((int8_t)EEPROM.read(3), -128, 127, -2000, 2000);;
      maxLaserPower[0] = EEPROM.read(4);
      maxLaserPower[1] = EEPROM.read(5);
      maxLaserPower[2] = EEPROM.read(6);
    }

void Settings::setFromJson(StaticJsonDocument<300> doc) {
      maxLeft = doc["maxLeft"];
      maxRight = doc["maxRight"];
      maxHeight = doc["maxHeight"];
      minHeight = doc["minHeight"];
      maxLaserPower[0] = doc["maxLaserPower"][0];
      maxLaserPower[1] = doc["maxLaserPower"][1];
      maxLaserPower[2] = doc["maxLaserPower"][2];

      saveSettingsInEEPROM();
    }

void Settings::toJson() { // prints the json to serial
      StaticJsonDocument<300> doc;
      doc["maxLeft"] = maxLeft;
      doc["maxRight"] = maxRight;
      doc["maxHeight"] = maxHeight;
      doc["minHeight"] = minHeight;

      JsonArray laserPower = doc.createNestedArray("maxLaserPower");
      laserPower.add(maxLaserPower[0]);
      laserPower.add(maxLaserPower[1]);
      laserPower.add(maxLaserPower[2]);

      serializeJson(doc, Serial);
    }

void Settings::saveSettings(String json) {
      StaticJsonDocument<300> doc;
      DeserializationError error = deserializeJson(doc, json);

      if (error) {
        Serial.print("deserializeJson failed: ");
        Serial.println(error.c_str());
      }

      else {
        setFromJson(doc);
      }
    }
