/*
  light_controller.h - PWM light panel control with K1 relay power-gating
  DarkLight Cover Calibrator - ESP32-S3 Port

  (c) Copyright Nathan Woelfle 2020-present day. All Rights Reserved.
  Creative Commons Attribution-NonCommercial 4.0 International License
*/

#ifndef LIGHT_CONTROLLER_H
#define LIGHT_CONTROLLER_H

#include <Arduino.h>
#include "config.h"

#ifdef LIGHT_INSTALLED

class LightController {
public:
  void begin();
  void loop();

  void turnPanelTo(uint16_t value);
  void turnPanelOff();

  CalibratorState getState() const       { return _calibratorState; }
  uint16_t getCurrentBrightness() const;
  uint16_t getMaxBrightness() const      { return _maxBrightness; }
  uint16_t getRawLightValue() const      { return _lightValue; }
  uint16_t getBroadband() const          { return _broadbandValue; }
  uint16_t getNarrowband() const         { return _narrowbandValue; }
  uint16_t getPreviousValue() const      { return _previousLightPanelValue; }
  bool     getAutoON() const             { return _autoON; }

  void setAutoON(bool value)             { _autoON = value; }
  void setStabilizeTime(uint32_t ms)     { _stabilizeTime = ms; }
  uint32_t getStabilizeTime() const      { return _stabilizeTime; }
  void setMaxBrightness(uint16_t value);

  void saveBroadband();
  void saveNarrowband();
  void setBroadband(uint16_t value)      { _broadbandValue = value; }
  void setNarrowband(uint16_t value)     { _narrowbandValue = value; }
  uint16_t getBroadbandStep() const;
  uint16_t getNarrowbandStep() const;

  // Called when cover closes with autoON
  void restorePreviousLight();

private:
  CalibratorState _calibratorState = CAL_OFF;
  uint16_t _maxBrightness = DEFAULT_MAX_BRIGHTNESS;
  uint32_t _stabilizeTime = DEFAULT_STABILIZE_TIME;
  uint16_t _lightValue = 0;
  uint16_t _broadbandValue = 25;
  uint16_t _narrowbandValue = LIGHT_PWM_MAX;
  uint16_t _previousLightPanelValue = LIGHT_PWM_MAX;
  bool     _autoON = false;
  uint32_t _startLightTimer = 0;

  void setRelay(bool on);
  void processLightStabilization();
};

extern LightController light;

#endif // LIGHT_INSTALLED
#endif // LIGHT_CONTROLLER_H
