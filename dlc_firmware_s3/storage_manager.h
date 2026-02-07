/*
  storage_manager.h - NVS Preferences wrapper replacing EEPROMWearLevel
  DarkLight Cover Calibrator - ESP32-S3 Port

  (c) Copyright Nathan Woelfle 2020-present day. All Rights Reserved.
  Creative Commons Attribution-NonCommercial 4.0 International License
*/

#ifndef STORAGE_MANAGER_H
#define STORAGE_MANAGER_H

#include <Arduino.h>
#include <Preferences.h>
#include "config.h"

class StorageManager {
public:
  void begin();

  // Original firmware values
  uint8_t  loadCoverState();
  void     saveCoverState(uint8_t state);

  uint16_t loadPanelValue();
  void     savePanelValue(uint16_t value);

  uint16_t loadBroadband();
  void     saveBroadband(uint16_t value);

  uint16_t loadNarrowband();
  void     saveNarrowband(uint16_t value);

  // Servo configuration (uint16_t for 270-degree servo support)
  uint16_t loadServoOpenAngle();
  void     saveServoOpenAngle(uint16_t angle);
  uint16_t loadServoCloseAngle();
  void     saveServoCloseAngle(uint16_t angle);
  uint16_t loadServoMinPulse();
  void     saveServoMinPulse(uint16_t pw);
  uint16_t loadServoMaxPulse();
  void     saveServoMaxPulse(uint16_t pw);
  uint32_t loadMoveTime();
  void     saveMoveTime(uint32_t ms);
  uint16_t loadServoRangeMin();
  void     saveServoRangeMin(uint16_t angle);
  uint16_t loadServoRangeMax();
  void     saveServoRangeMax(uint16_t angle);

  // Light configuration
  uint16_t loadMaxBrightness();
  void     saveMaxBrightness(uint16_t value);
  uint32_t loadStabilizeTime();
  void     saveStabilizeTime(uint32_t ms);

  // Heater configuration
  uint8_t loadHeaterMode();
  void    saveHeaterMode(uint8_t mode);
  float   loadDeltaPoint();
  void    saveDeltaPoint(float value);
  uint32_t loadShutoffTime();
  void     saveShutoffTime(uint32_t ms);

  // WiFi configuration
  String loadWifiSSID();
  void   saveWifiSSID(const String& ssid);
  String loadWifiPass();
  void   saveWifiPass(const String& pass);

private:
  Preferences _prefs;
};

extern StorageManager storage;

#endif // STORAGE_MANAGER_H
