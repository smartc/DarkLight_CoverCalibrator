/*
  light_controller.cpp - PWM light panel control with K1 relay power-gating
  DarkLight Cover Calibrator - ESP32-S3 Port

  Ported from dlc_firmware.ino lines 177-1116

  (c) Copyright Nathan Woelfle 2020-present day. All Rights Reserved.
  Creative Commons Attribution-NonCommercial 4.0 International License
*/

#include "light_controller.h"

#ifdef LIGHT_INSTALLED

#include "storage_manager.h"
#include "Debug.h"

LightController light;

void LightController::begin() {
  pinMode(PIN_LIGHT, OUTPUT);
  pinMode(PIN_RELAY_K1, OUTPUT);
  digitalWrite(PIN_RELAY_K1, LOW); // Relay off at startup
  analogWriteResolution(LIGHT_PWM_BITS);

  #ifdef ENABLE_SAVING_TO_MEMORY
    _previousLightPanelValue = storage.loadPanelValue();
    _broadbandValue = storage.loadBroadband();
    _narrowbandValue = storage.loadNarrowband();
    _maxBrightness = storage.loadMaxBrightness();
    _stabilizeTime = storage.loadStabilizeTime();

    if (_previousLightPanelValue == 0) _previousLightPanelValue = LIGHT_PWM_MAX;
    if (_broadbandValue == 0) _broadbandValue = 25;
    if (_narrowbandValue == 0) _narrowbandValue = LIGHT_PWM_MAX;
  #else
    _previousLightPanelValue = LIGHT_PWM_MAX;
    _broadbandValue = 0;
    _narrowbandValue = 0;
  #endif

  _calibratorState = CAL_OFF;

  Debug::infof("LIGHT", "Initialized: maxBright=%d, pwmMax=%d, stabilize=%lu",
               _maxBrightness, LIGHT_PWM_MAX, _stabilizeTime);
}

void LightController::loop() {
  processLightStabilization();
}

void LightController::turnPanelTo(uint16_t value) {
  value = constrain(value, (uint16_t)0, _maxBrightness);
  _lightValue = map(value, 0, _maxBrightness, 0, LIGHT_PWM_MAX);
  _calibratorState = CAL_NOT_READY;

  // Power-gate: energize relay before PWM
  setRelay(true);
  analogWrite(PIN_LIGHT, _lightValue);
  _startLightTimer = millis();

  Debug::infof("LIGHT", "Panel set to step=%d, PWM=%d", value, _lightValue);
}

void LightController::turnPanelOff() {
  analogWrite(PIN_LIGHT, 0);
  _lightValue = 0;
  _calibratorState = CAL_OFF;

  // Power-gate: de-energize relay after PWM off
  setRelay(false);

  Debug::info("LIGHT", "Panel off");
}

void LightController::setMaxBrightness(uint16_t value) {
  _maxBrightness = value;
  #ifdef ENABLE_SAVING_TO_MEMORY
    storage.saveMaxBrightness(value);
  #endif
}

uint16_t LightController::getCurrentBrightness() const {
  if (_maxBrightness == 0) return 0;
  return map(_lightValue, 0, LIGHT_PWM_MAX, 0, _maxBrightness);
}

uint16_t LightController::getBroadbandStep() const {
  if (_maxBrightness == 0) return 0;
  return map(_broadbandValue, 0, LIGHT_PWM_MAX, 0, _maxBrightness);
}

uint16_t LightController::getNarrowbandStep() const {
  if (_maxBrightness == 0) return 0;
  return map(_narrowbandValue, 0, LIGHT_PWM_MAX, 0, _maxBrightness);
}

void LightController::saveBroadband() {
  _broadbandValue = _lightValue;
  #ifdef ENABLE_SAVING_TO_MEMORY
    storage.saveBroadband(_broadbandValue);
  #endif
  Debug::infof("LIGHT", "Broadband saved: %d", _broadbandValue);
}

void LightController::saveNarrowband() {
  _narrowbandValue = _lightValue;
  #ifdef ENABLE_SAVING_TO_MEMORY
    storage.saveNarrowband(_narrowbandValue);
  #endif
  Debug::infof("LIGHT", "Narrowband saved: %d", _narrowbandValue);
}

void LightController::restorePreviousLight() {
  if (_autoON) {
    // Convert saved PWM value back to step for turnPanelTo
    uint16_t step = (_maxBrightness > 0) ? map(_previousLightPanelValue, 0, LIGHT_PWM_MAX, 0, _maxBrightness) : 0;
    turnPanelTo(step);
  }
}

void LightController::setRelay(bool on) {
  digitalWrite(PIN_RELAY_K1, on ? HIGH : LOW);
  Debug::debugf("LIGHT", "Relay K1 %s", on ? "ON" : "OFF");
}

void LightController::processLightStabilization() {
  if (_calibratorState == CAL_NOT_READY) {
    if (millis() - _startLightTimer >= _stabilizeTime) {
      _calibratorState = CAL_READY;
      _previousLightPanelValue = _lightValue;
      #ifdef ENABLE_SAVING_TO_MEMORY
        storage.savePanelValue(_previousLightPanelValue);
      #endif
      Debug::info("LIGHT", "Stabilized - Ready");
    }
  }
}

#endif // LIGHT_INSTALLED
