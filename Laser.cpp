// See LICENSE file for details
// Copyright 2016 Florian Link (at) gmx.de
#include "Laser.h"
#include "math.h"

// if this is enabled, pins need to be 10 and 7 in dac init below, but it is a big speedup!
#define MCP4X_PORT_WRITE 1
#include "DAC_MCP4X.h"
#include "Settings.h"
MCP4X dac;

byte red, green, blue = 0;

short galvoPositionsx[50];
short galvoPositionsy[50];
Settings _setting;

unsigned long previousInterval;
const byte AudienceScanMaxPower[] = { 90, 80, 87 }; // rgb

short yPos = 0;
short xPos = 0;

Laser::Laser()
{
  _quality = FROM_FLOAT(1. / (LASER_QUALITY));

  _x = 0;
  _y = 0;
  _oldX = 0;
  _oldY = 0;

  _state = 0;

  _scale = 1;
  _offsetX = 0;
  _offsetY = 0;

  _moved = 0;
  _maxMove = -1;
  _laserForceOff = false;
  resetClipArea();

  _enable3D = false;
  _zDist = 1000;
}

void Laser::init(Settings setting)
{
  _setting = setting;
  dac.init(MCP4X_4822, 5000, 5000, 10, 7, 1);
  dac.setGain2x(MCP4X_CHAN_A, 0);
  dac.setGain2x(MCP4X_CHAN_B, 0);
  dac.begin(1);
}

void Laser::sendToDAC(int x, int y)
{
#ifdef LASER_SWAP_XY
  int x1 = y;
  int y1 = x;
#else
  int x1 = x;
  int y1 = y;
#endif
#ifdef LASER_FLIP_X
  x1 = 4095 - x1;
#endif
#ifdef LASER_FLIP_Y
  y1 = 4095 - y1;
#endif
  dac.output2(x1, y1);
}

void Laser::resetClipArea()
{
  _clipXMin = 0;
  _clipYMin = 0;
  _clipXMax = 4095;
  _clipYMax = 4095;
}

void Laser::setClipArea(long x, long y, long x1, long y1)
{
  _clipXMin = x;
  _clipYMin = y;
  _clipXMax = x1;
  _clipYMax = y1;
}

const int INSIDE = 0; // 0000
const int LEFT = 1;   // 0001
const int RIGHT = 2;  // 0010
const int BOTTOM = 4; // 0100
const int TOP = 8;    // 1000

byte occurrencesInArray(short value, short arrayToCheck[]) {
  byte total = 0;

  for (int i = 0; i < 50; i++)
    if (arrayToCheck[i] == value) total++;

  return total;
}

void fillArray(short arrayToFill[]) {
  for (int i = 0; i < 50; i++)
    arrayToFill[i] = 2001;
}

byte countArrayLenght(short arrayToCount[], short defaultValue) {
  byte total = 0;

  for (int i = 0; i < 50; i++)
    if (arrayToCount[i] != defaultValue) total++;

  return total;
}

void limitLaserPower() {
  if (red > AudienceScanMaxPower[0]) {
    red = abs(red - red / 6);
    if (red < AudienceScanMaxPower[0]) red = AudienceScanMaxPower[0];
  }

  if (green > AudienceScanMaxPower[1]) {
    green = abs(green - green / 6);
    if (green < AudienceScanMaxPower[1]) green = AudienceScanMaxPower[1];
  }

  if (blue > AudienceScanMaxPower[2]) {
    blue = abs(blue - blue / 6);
    if (blue < AudienceScanMaxPower[2]) blue = AudienceScanMaxPower[2];
  }
}

void audienceScanCheck() {
  if (yPos < 0) limitLaserPower();
}

// this function will check if a beam is within the same spot in a certain interval value, if so the output of the lasers will be reduced
void Laser::preventHotSpotsAndStaticBeams() {
  unsigned long currentMillis = millis();

  byte xArrayLenght = countArrayLenght(galvoPositionsx, 2001);
  byte yArrayLenght = countArrayLenght(galvoPositionsy, 2001);

  galvoPositionsx[xArrayLenght + 1] = xPos;
  galvoPositionsy[yArrayLenght + 1] = yPos;

  if (currentMillis - previousInterval > 50 || xArrayLenght >= 50 || yArrayLenght >= 50) {
    fillArray(galvoPositionsx);
    fillArray(galvoPositionsy);

    previousInterval = currentMillis;
  }

  int occurencesX = occurrencesInArray(xPos, galvoPositionsx);
  int occurencesY = occurrencesInArray(yPos, galvoPositionsy);
  
  if (occurencesX > 3 && occurencesX < 8 && occurencesY > 3 && occurencesY < 8) {
    limitLaserPower();
    return;
  }

  if (occurencesX > 8 && occurencesY > 8) off();
}

void Laser::turnOnLasers() {
  preventHotSpotsAndStaticBeams();
  audienceScanCheck();

  analogWrite(2, red);
  analogWrite(3, green);
  analogWrite(4, blue);
}

void Laser::on(byte r, byte g, byte b)
{
  green = g;
  red = r;
  blue = b;

  turnOnLasers();
}

void Laser::off()
{
  analogWrite(2, 0);
  analogWrite(3, 0);
  analogWrite(4, 0);

  green = 0;
  red = 0;
  blue = 0;
}

void Laser::updateSettings(Settings setting) {
  _setting = setting;
}

short Laser::checkXAxisBoundary(short x) {
  if (x < _setting.maxLeft) x = _setting.maxLeft;
  if (x > _setting.maxRight) x = _setting.maxRight;

  return x;
}

short Laser::checkYAxisBoundary(short y) {
  if (y > _setting.maxHeight) y = _setting.maxHeight;
  if (y < _setting.minHeight) y = _setting.minHeight;

  return y;
}

void Laser::sendto (short xpos, short ypos)
{
  xpos = checkXAxisBoundary(xpos);
  ypos = checkYAxisBoundary(ypos);

  long xNew = TO_INT(xpos * _scale) + _offsetX;
  long yNew = TO_INT(ypos * _scale) + _offsetY;

  yPos = ypos;
  xPos = xpos;

  sendtoRaw(xNew, yNew);
  turnOnLasers();
}

void Laser::sendtoRaw (long xNew, long yNew)
{
  // devide into equal parts, using _quality
  long fdiffx = xNew - _x;
  long fdiffy = yNew - _y;
  long diffx = TO_INT(abs(fdiffx) * _quality);
  long diffy = TO_INT(abs(fdiffy) * _quality);

  // use the bigger direction
  if (diffx < diffy)
    diffx = diffy;

  fdiffx = FROM_INT(fdiffx) / diffx;
  fdiffy = FROM_INT(fdiffy) / diffx;
  // interpolate in FIXPT
  FIXPT tmpx = 0;
  FIXPT tmpy = 0;
  for (int i = 0; i < diffx - 1; i++)
  {
    tmpx += fdiffx;
    tmpy += fdiffy;
    sendToDAC(_x + TO_INT(tmpx), _y + TO_INT(tmpy));
  }

  _x = xNew;
  _y = yNew;
  sendToDAC(_x, _y);
}

void Laser::setScale(float scale)
{
  _scale = FROM_FLOAT(scale);
}

void Laser::setOffset(long offsetX, long offsetY)
{
  _offsetX = offsetX;
  _offsetY = offsetY;
}
