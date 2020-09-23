// See LICENSE file for details
// Copyright 2016 Florian Link (at) gmx.de
#include "Laser.h"
#include <ArduinoJson.h>
#include "Settings.h"

Laser laser;

byte incomingByte = 0;
unsigned long previousCharacter = 0;
bool startmessage = false;
String receivedMessage;

Settings setting;
byte animationSpeed = 2;

void setup()
{
  setting.getAndsetFromEEPROM();

  laser.init(setting);
  laser.setScale(1);
  laser.setOffset(2048, 2048);

  byte pins[7] = {2, A2, 3, 5, 6, 9, 13};
  for (unsigned int i = 0; i < sizeof(pins) / sizeof(pins[0]); i++) pinMode(i, OUTPUT);

  analogWriteFrequency(2, 50000);
  analogWriteFrequency(3, 50000);
  analogWriteFrequency(4, 50000);

  long baudRate = Serial.baud();
  Serial.begin(baudRate);

  laser.off();
}

void loop() {
  readSerial();
  //  liquidSkyDown();
  //  liquidSkySideWay();
  //  liquidSkyEnlarging();
  //  spiningObjects(random(3, 7));
  //  liquidSkyLarging();
  //  bouncyLines();
  //  randomDots(2000);
  //  randomLinesMoving();
  //  enlargingCircle();
  //  spinningLine();
  //  appearingCircle();
}

void setAnimationSpeed(int animationspeed) {
  if (animationSpeed == 2) {
    for (int i = 0; i < 3; i++)
      setting.maxLaserPower[i] = 0;
    laser.off();
  }
  else
    setting.getAndsetFromEEPROM();

  animationSpeed = animationspeed;
}

void readSerial() {

  unsigned long currentMillis = millis();
  if (Serial.available() > 0) {
    previousCharacter = currentMillis;

    incomingByte = Serial.read();
    char receivedCharacter = (char) incomingByte;

    if (receivedCharacter == '(') startmessage = true;
    if (startmessage == true && receivedCharacter != '(' && receivedCharacter != ')') receivedMessage = receivedMessage + receivedCharacter;

    if (receivedCharacter == ')') {
      startmessage = false;

      byte colonIndex = receivedMessage.indexOf(','); // Get value after ,
      byte colonIndexTwo = receivedMessage.indexOf(':'); // Get value after :
      byte colonIndexThree = receivedMessage.indexOf('|'); // Get value after |
      String Command = receivedMessage.substring(0, colonIndex); // Get value between start and ,

      if (Command == "set-settings") {
        setting.saveSettings(receivedMessage.substring(colonIndex + 1, receivedMessage.length()));
        laser.updateSettings(setting);
      }

      else if (Command == "get-settings")
        setting.toJson();

      else if (Command == "set-animationspeed")
        setAnimationSpeed(receivedMessage.substring(colonIndex + 1, colonIndexTwo).toInt());

      else if (Command == "g")
        laser.sendto(receivedMessage.substring(colonIndex + 1, colonIndexTwo).toInt(), receivedMessage.substring(colonIndexTwo + 1, colonIndexThree).toInt());

      else if (Command == "l")
        laser.on(receivedMessage.substring(colonIndex + 1, colonIndexTwo).toInt(), receivedMessage.substring(colonIndexTwo + 1, colonIndexThree).toInt(), receivedMessage.substring(colonIndexThree + 1, receivedMessage.length()).toInt());

      else if (Command == "off")
        laser.off();

      receivedMessage = "";
    }

    receivedCharacter = "";
  }

  if (currentMillis - previousCharacter > 2) {
    previousCharacter = currentMillis;
    laser.off();
    animationSpeed = 2;
  }
}

void getRandomLaserColors(byte rgb[]) {
  if (animationSpeed != 2) {
    rgb[0] = random(85, setting.maxLaserPower[0]);
    rgb[1] = random(75, setting.maxLaserPower[1]);
    rgb[2] = random(78, setting.maxLaserPower[2]);

    if (rgb[0] < 90 && rgb[1] < 90 && rgb[2] && setting.maxLaserPower[0] > 0 && setting.maxLaserPower[1] > 0 && setting.maxLaserPower[2] > 0) {
      rgb[0] = setting.maxLaserPower[0];
      rgb[1] = setting.maxLaserPower[1];
      rgb[2] = setting.maxLaserPower[2];
    }
  }

  else {
    laser.off();
    for (int i = 0; i < 3; i++)
      rgb[i] = 0;
  }
}

void spiningObjects(byte total) {
  if (animationSpeed != 2) {
    const int scale = 12;
    laser.sendto(SIN(0) / scale, COS(0) / scale);

    byte rgb[3];

    for (int k = 0; k < 10; k++) {
      for (int i = 0; i < 360 / total - 5; i += abs(animationSpeed / 4) + 1) {
        for (int j = 0; j < 20; j += (animationSpeed / 2) + 1) {
          readSerial();
          for (int r = 5; r <= 360; r += 360 / total)
          {
            short y = COS(r + i) / scale;
            short x = SIN(r + i) / scale;

            laser.sendto(x, y);
            delayMicroseconds(800);
          }
        }

        delayMicroseconds(300);
        getRandomLaserColors(rgb);
        laser.on(rgb[0], rgb[1], rgb[2]);
      }
    }
  }

  laser.off();
  delayMicroseconds(20);
}

void liquidSkyDown() {
  if (animationSpeed != 2) {
    byte rgb[3];
    byte totalLines = 20;
    short distance = (abs(setting.maxRight - setting.maxLeft) / totalLines);
    for (int y = setting.maxHeight; y > setting.minHeight; y -= animationSpeed * 2) {
      readSerial();
      for (int i = 0; i < totalLines; i++) {
        laser.sendto(setting.maxLeft + (distance * i), y);
        getRandomLaserColors(rgb);
        delayMicroseconds(200);
        laser.on(rgb[0], rgb[1], rgb[2]);

        delayMicroseconds(200);
        laser.off();
        delayMicroseconds(100);
      }
    }
  }

  laser.off();
}

void liquidSkyLarging() {
  if (animationSpeed != 2) {
    short center = (setting.maxLeft + setting.maxRight) / 2;

    for (int i = 0; i < 3; i++) {
      short left = center - 50;
      short right = center + 50;
      byte rgb[3];
      getRandomLaserColors(rgb);
      short y = random(setting.minHeight, setting.maxHeight);
      readSerial();

      while (left > setting.maxLeft || right < setting.maxRight) {
        laser.sendto(left -= abs(animationSpeed / 2), y);
        laser.on(rgb[0], rgb[1], rgb[2]);
        delayMicroseconds(800);

        laser.sendto(right += abs(animationSpeed / 2), y);
        delayMicroseconds(800);
      }
    }
  }

  laser.off();
}

void bouncyLines() {
  if (animationSpeed != 2) {
    const short center = (setting.maxRight + setting.maxLeft) / 2;
    const short centerY = (setting.maxHeight + setting.minHeight) / 2;

    const byte totalLines = 6;
    const short linesDistance = (abs(setting.maxRight) + abs(setting.maxLeft)) / totalLines;
    byte rgb[3];

    short previousColorChange = 0;

    for (int i = 0; i < 4000; i += animationSpeed) {
      readSerial();
      for (int line = 0; line < totalLines; line++) {
        short x = setting.maxLeft + (linesDistance * line);
        short y = 1000 * sin(line * 2 + (animationSpeed / 2) * millis() / 1000.0) + centerY;

        laser.sendto(x, y);
        delayMicroseconds(450);

        laser.on(rgb[0], rgb[1], rgb[2]);
        delayMicroseconds(200);

        laser.off();
        delayMicroseconds(200);
      }

      if (i - previousColorChange > 400) {
        getRandomLaserColors(rgb);
        previousColorChange = i;
      }
    }
  }

  laser.off();
}

void randomDots(short total) {
  if (animationSpeed != 2) {
    byte rgb[3];

    for (int i = 0; i < total; i++) {
      readSerial();
      laser.sendto(random(setting.maxLeft, setting.maxRight), random(setting.minHeight, setting.maxHeight));
      getRandomLaserColors(rgb);
      delayMicroseconds(650);

      laser.on(rgb[0], rgb[1], rgb[2]);
      delayMicroseconds(350);

      laser.off();
      delayMicroseconds(350);
    }
  }

  laser.off();
}

void randomLinesMoving() {
  if (animationSpeed != 2) {
    byte rgb[3];
    const short center = (setting.maxRight + setting.maxLeft) / 2;

    byte totalLines = random(3, 10);

    for (int total = 0; total < 30; total++) {
      getRandomLaserColors(rgb);
      for (int i = 0; i < 40; i++) {
        readSerial();
        for (int line = 0; line < totalLines; line++) {
          short x = ((abs(setting.maxLeft) + abs(setting.maxRight)) / 2) * cos(line * 2 + (animationSpeed / 4) * millis() / 1000.0);

          laser.sendto(x, setting.minHeight + 200);
          delayMicroseconds(800);
          laser.on(rgb[0], rgb[1], rgb[2]);
          delayMicroseconds(150);
          laser.off();
          delayMicroseconds(200);
        }
      }
    }
  }

  laser.off();
}

void circle(int scale, byte rgb[]) {
  readSerial();

  if (animationSpeed != 2) {
    for (int r = 5; r <= 360; r++)
    {
      laser.on(rgb[0], rgb[1], rgb[2]);

      short y = COS(r) / scale + (setting.maxHeight + setting.minHeight) / 2;
      short x = SIN(r) / scale;

      laser.sendto(x, y);
      delayMicroseconds(10);
    }
  }

  laser.off();
}

void enlargingCircle() {
  if (animationSpeed != 2) {
    byte rgb[3];
    getRandomLaserColors(rgb);
    for (int total = 0; total < 5; total++) {
      for (double i = 42; i > 12; i -= (double)animationSpeed / 30)
        circle(i, rgb);

      getRandomLaserColors(rgb);

      for (double i = 12; i < 42; i += (double)animationSpeed / 30)
        circle(i, rgb);
    }
  }

  laser.off();
}

void liquidSkySideWay() {
  if (animationSpeed != 2) {
    const short center = (setting.maxRight + setting.maxLeft) / 2;

    byte rgb[3];
    getRandomLaserColors(rgb);

    for (int total = 0; total < 15; total++) {
      getRandomLaserColors(rgb);
      for (int i = 0; i < 250; i += animationSpeed / 2) {
        readSerial();
        laser.sendto(setting.maxLeft, setting.minHeight);
        delayMicroseconds(800);
        laser.on(rgb[0], rgb[1], rgb[2]);
        laser.sendto(center, setting.maxHeight);

        delayMicroseconds(800);
      }

      laser.off();

      for (int i = 0; i < 250; i += animationSpeed / 2) {
        readSerial();
        laser.sendto(setting.maxRight, setting.minHeight);
        delayMicroseconds(800);
        laser.on(rgb[0], rgb[1], rgb[2]);
        laser.sendto(center, setting.maxHeight);

        delayMicroseconds(800);
      }

      laser.off();
    }
  }
}

void liquidSkyEnlarging() {
  if (animationSpeed != 2) {
    const short center = (setting.maxRight + setting.maxLeft) / 2;

    byte rgb[3];

    for (int total = 0; total < 10; total++) {
      int y = random(setting.minHeight, setting.maxHeight);
      getRandomLaserColors(rgb);
      for (int x = setting.maxLeft; x < setting.maxRight; x += animationSpeed) {
        readSerial();
        laser.sendto(setting.maxLeft, y);
        laser.on(rgb[0], rgb[1], rgb[2]);
        delayMicroseconds(800);

        laser.sendto(x, y);
        delayMicroseconds(800);
      }

      laser.off();
    }
  }
}

void spinningLine() {
  if (animationSpeed != 2) {
    const short center = (setting.maxRight + setting.maxLeft) / 2;
    const short centerY = (setting.maxHeight + setting.minHeight) / 2;

    byte rgb[3];

    for (int total = 0; total < 3; total++) {
      for (int r = 0; r < 360; r += animationSpeed / 2) {
        getRandomLaserColors(rgb);

        for (int i = 0; i < 10; i++) {
          readSerial();
          laser.sendto(center, centerY);
          delayMicroseconds(800);

          laser.on(rgb[0], rgb[1], rgb[2]);
          short y = COS(r) / 12 + (setting.maxHeight + setting.minHeight) / 2;
          short x = SIN(r) / 12;

          laser.sendto(x, y);
          delayMicroseconds(800);
        }
      }
    }
  }
}

void appearingCircle() {
  if (animationSpeed != 2) {
    byte rgb[3];
    getRandomLaserColors(rgb);

    for (int total = 0; total < 3; total++) {
      for (int r = 0; r < 360; r += animationSpeed / 4 + 1) {
        readSerial();

        short x = SIN(r) / 16;
        short y = COS(r) / 16 + (setting.maxHeight + setting.minHeight) / 2;

        laser.sendto(x, y);
        delayMicroseconds(100);

        for (int reverse = r; reverse > 0; reverse -= animationSpeed / 4 + 1) {
          laser.on(rgb[0], rgb[1], rgb[2]);

          laser.sendto(SIN(reverse) / 16, COS(reverse) / 16 + (setting.maxHeight + setting.minHeight) / 2);
          delayMicroseconds(100);
        }
      }
    }

    laser.off();
  }
}
