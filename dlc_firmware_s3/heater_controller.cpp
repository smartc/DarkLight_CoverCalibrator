/*
  heater_controller.cpp - Dew heater control with DS18B20 + BME280/DHT22 sensors
  DarkLight Cover Calibrator - ESP32-S3 Port

  Ported from dlc_firmware.ino lines 199-1276 (single heater channel)

  (c) Copyright Nathan Woelfle 2020-present day. All Rights Reserved.
  Creative Commons Attribution-NonCommercial 4.0 International License
*/

#include "heater_controller.h"

#ifdef HEATER_INSTALLED

#include "storage_manager.h"
#include "Debug.h"

HeaterController heater;

void HeaterController::begin() {
  pinMode(PIN_HEATER, OUTPUT);
  analogWrite(PIN_HEATER, 0);

  // Initialize I2C on custom pins for BME280
  #ifdef ENABLE_BME280
    Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL);
  #endif

  // Initialize DS18B20
  _oneWire = OneWire(PIN_DS18B20);
  _tempSensor = DallasTemperature(&_oneWire);
  _tempSensor.begin();
  _tempSensor.setWaitForConversion(false); // Non-blocking reads

  #ifdef ENABLE_BME280
    bool bmeStatus = _bme.begin(0x76, &Wire);
    if (!bmeStatus) {
      bmeStatus = _bme.begin(0x77, &Wire);
      if (!bmeStatus) {
        _heaterError = true;
        Debug::error("HEATER", "BME280 not found at 0x76 or 0x77");
      }
    }
  #endif

  #ifdef ENABLE_DHT22
    _dht = DHT(PIN_DHT, DHT22);
    _dht.begin();
  #endif

  #ifdef ENABLE_SAVING_TO_MEMORY
    _deltaPoint = storage.loadDeltaPoint();
    _heaterShutoff = storage.loadShutoffTime();
  #endif

  setHeaterState();

  Debug::infof("HEATER", "Initialized: delta=%.1f, shutoff=%lu", _deltaPoint, _heaterShutoff);
}

void HeaterController::loop(bool coverMoving) {
  // Don't run heater control while cover is moving or in error state
  if (coverMoving || _heaterState == HEATER_ERROR) {
    if (!coverMoving && (_heaterError || (_heaterUnknown && _heatOnClose))) {
      readSensors();
    }
    return;
  }

  manageHeat();
}

void HeaterController::setAutoHeat(bool on) {
  if (on) {
    _autoHeat = true;
    _heatOnClose = false;
    _manualHeat = false;
    Debug::info("HEATER", "Auto heat ON");
  } else {
    if (_autoHeat) {
      _autoHeat = false;
      resetErrorReadings();
      Debug::info("HEATER", "Auto heat OFF");
    }
  }
  setHeaterState();
}

void HeaterController::setManualHeat(bool on) {
  if (on) {
    _manualHeat = true;
    setHeaterState();
    _startHeaterTimer = millis();
    Debug::info("HEATER", "Manual heat ON");
  } else {
    if (_heaterUnknown || _heaterError) {
      _manualHeat = false;
      _heatOnClose = false;
      _autoHeat = false;
      resetErrorReadings();
    } else {
      if (_manualHeat) {
        _manualHeat = false;
      } else {
        _heatOnClose = false;
        _autoHeat = false;
      }
    }
    setHeaterState();
    Debug::info("HEATER", "Manual heat OFF");
  }
}

void HeaterController::setHeatOnClose(bool on) {
  if (on) {
    _heatOnClose = true;
    _autoHeat = false;
    _manualHeat = false;
    readSensors(); // Verify sensors work before event
    Debug::info("HEATER", "Heat-on-close ON");
  } else {
    if (_heatOnClose) {
      _heatOnClose = false;
      resetErrorReadings();
      Debug::info("HEATER", "Heat-on-close OFF");
    }
  }
  setHeaterState();
}

void HeaterController::turnOff() {
  _autoHeat = false;
  _manualHeat = false;
  _heatOnClose = false;
  resetErrorReadings();
  setHeaterState();
  Debug::info("HEATER", "All heating OFF");
}

void HeaterController::triggerHeatOnClose() {
  if (_heatOnClose) {
    _manualHeat = true;
    setHeaterState();
    _startHeaterTimer = millis();
    Debug::info("HEATER", "Heat-on-close triggered");
  }
}

HeaterData HeaterController::getHeaterData() const {
  HeaterData data;
  data.heaterTemp = _heaterTemp;
  data.heaterPWM = _heaterPWM;
  data.outsideTemp = _outsideTemp;
  data.humidity = _humidityLevel;
  data.dewPoint = _dewPoint;
  return data;
}

void HeaterController::setHeaterState() {
  if (_heaterError) {
    _heaterState = HEATER_ERROR;
  } else if (_heaterUnknown) {
    _heaterState = HEATER_UNKNOWN;
  } else if (_manualHeat) {
    _heaterState = HEATER_ON;
  } else if (_autoHeat) {
    _heaterState = HEATER_AUTO;
  } else if (_heatOnClose) {
    _heaterState = HEATER_SET;
  } else {
    _heaterState = HEATER_OFF;
    resetErrorReadings();
  }

  // SAFETY: ensure PWM is shut off unless heater is ON or AUTO
  if (_heaterState != HEATER_AUTO && _heaterState != HEATER_ON) {
    analogWrite(PIN_HEATER, 0);
    _heaterPWM = 0;
  }
}

void HeaterController::manageHeat() {
  if (!_heaterError && (_autoHeat || _manualHeat)) {
    uint32_t currentDewMillis = millis();

    if (currentDewMillis - _previousDewMillis >= DEW_INTERVAL) {
      _previousDewMillis = currentDewMillis;

      // Handle async DS18B20 conversion
      if (!_asyncConversionStarted) {
        _tempSensor.requestTemperatures();
        _asyncConversionStarted = true;
        _conversionStartTime = millis();
        return; // Wait for conversion
      }

      // Check if conversion is complete (750ms for 12-bit)
      if (millis() - _conversionStartTime < 750) {
        return; // Still converting
      }

      _asyncConversionStarted = false;

      if (readSensors()) {
        return; // Skip if error
      }

      // Calculate dew point (August-Roche-Magnus formula)
      float temp = ((DEW_POINT_ALPHA * _outsideTemp) / (DEW_POINT_BETA + _outsideTemp)) + log(_humidityLevel / 100.0f);
      _dewPoint = (DEW_POINT_BETA * temp) / (DEW_POINT_ALPHA - temp);

      activateHeater();
    }
  }

  // Handle manual heat timeout
  if (!_heaterError && _manualHeat && millis() - _startHeaterTimer >= _heaterShutoff) {
    _manualHeat = false;
    setHeaterState();
    Debug::info("HEATER", "Manual heat timeout");
  }

  // If reading issue, attempt to reset error state
  if (_heaterError || (_heaterUnknown && _heatOnClose)) {
    readSensors();
  }
}

void HeaterController::activateHeater() {
  if (_heaterTemp < _dewPoint + _deltaPoint) {
    float tempDiff = (_dewPoint + _deltaPoint) - _heaterTemp;
    _heaterPWM = map((long)(tempDiff * PWM_MAP_MULTIPLIER), 0, (long)PWM_MAP_RANGE, 0, (long)MAX_HEATER_PWM);
    _heaterPWM = constrain(_heaterPWM, (uint8_t)0, (uint8_t)MAX_HEATER_PWM);
    analogWrite(PIN_HEATER, _heaterPWM);
  } else {
    analogWrite(PIN_HEATER, 0);
    _heaterPWM = 0;
  }
}

bool HeaterController::readSensors() {
  bool errorReading = false;
  static bool lastErrorReading = true;

  // Read DS18B20 heater temperature
  _heaterTemp = _tempSensor.getTempCByIndex(0);
  if (_heaterTemp == DEVICE_DISCONNECTED_C) {
    errorReading = true;
  }

  #ifdef ENABLE_BME280
    _outsideTemp = _bme.readTemperature();
    _humidityLevel = _bme.readHumidity();

    if (isnan(_outsideTemp) || isnan(_humidityLevel) ||
        _humidityLevel < 0 || _humidityLevel > 100 ||
        _outsideTemp < -40 || _outsideTemp > 85) {
      errorReading = true;
    }
  #endif

  #ifdef ENABLE_DHT22
    _outsideTemp = _dht.readTemperature();
    _humidityLevel = _dht.readHumidity();

    if (isnan(_outsideTemp) || isnan(_humidityLevel)) {
      errorReading = true;
    }
  #endif

  if (errorReading) {
    _errorCounter++;
    if (_errorCounter >= MAX_ERROR_COUNT) {
      _heaterUnknown = false;
      _heaterError = true;
      Debug::error("HEATER", "Max error count reached");
    } else {
      _heaterUnknown = true;
    }
    setHeaterState();
  } else {
    if (lastErrorReading) {
      resetErrorReadings();
      setHeaterState();
    }
  }

  lastErrorReading = errorReading;
  return errorReading;
}

void HeaterController::resetErrorReadings() {
  _errorCounter = 0;
  _heaterUnknown = false;
  _heaterError = false;
}

#endif // HEATER_INSTALLED
