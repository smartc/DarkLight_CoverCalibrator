/*
  heater_controller.h - Dew heater control with DS18B20 + BME280/DHT22 sensors
  DarkLight Cover Calibrator - ESP32-S3 Port

  (c) Copyright Nathan Woelfle 2020-present day. All Rights Reserved.
  Creative Commons Attribution-NonCommercial 4.0 International License
*/

#ifndef HEATER_CONTROLLER_H
#define HEATER_CONTROLLER_H

#include <Arduino.h>
#include "config.h"

#ifdef HEATER_INSTALLED

#include <Wire.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#ifdef ENABLE_BME280
  #include <Adafruit_BME280.h>
#endif

#ifdef ENABLE_DHT22
  #include <DHT.h>
#endif

struct HeaterData {
  float heaterTemp;
  uint8_t heaterPWM;
  float outsideTemp;
  float humidity;
  float dewPoint;
};

class HeaterController {
public:
  void begin();
  void loop(bool coverMoving);

  void setAutoHeat(bool on);
  void setManualHeat(bool on);
  void setHeatOnClose(bool on);
  void turnOff();

  HeaterState getState() const      { return _heaterState; }
  HeaterData  getHeaterData() const;

  bool isAutoHeat() const           { return _autoHeat; }
  bool isManualHeat() const         { return _manualHeat; }
  bool isHeatOnClose() const        { return _heatOnClose; }

  float getDeltaPoint() const       { return _deltaPoint; }
  void  setDeltaPoint(float val)    { _deltaPoint = val; }
  uint32_t getShutoffTime() const   { return _heaterShutoff; }
  void  setShutoffTime(uint32_t ms) { _heaterShutoff = ms; }

  // Called by cover controller when close completes and heatOnClose is armed
  void triggerHeatOnClose();

private:
  HeaterState _heaterState = HEATER_OFF;
  bool _autoHeat = false;
  bool _manualHeat = false;
  bool _heatOnClose = false;
  bool _heaterError = false;
  bool _heaterUnknown = false;
  uint8_t _errorCounter = 0;

  float _deltaPoint = DEFAULT_DELTA_POINT;
  uint32_t _heaterShutoff = DEFAULT_HEATER_SHUTOFF;

  // Sensor readings
  float _outsideTemp = 0.0f;
  float _humidityLevel = 0.0f;
  float _dewPoint = 0.0f;
  float _heaterTemp = 0.0f;
  uint8_t _heaterPWM = 0;

  // Timing
  uint32_t _previousDewMillis = 0;
  uint32_t _startHeaterTimer = 0;

  // Sensor objects
  OneWire _oneWire;
  DallasTemperature _tempSensor;
  bool _asyncConversionStarted = false;
  uint32_t _conversionStartTime = 0;

  #ifdef ENABLE_BME280
    Adafruit_BME280 _bme;
  #endif

  #ifdef ENABLE_DHT22
    DHT _dht;
  #endif

  void setHeaterState();
  void manageHeat();
  void activateHeater();
  bool readSensors();
  void resetErrorReadings();
};

extern HeaterController heater;

#endif // HEATER_INSTALLED
#endif // HEATER_CONTROLLER_H
