/*
  storage_manager.cpp - NVS Preferences wrapper replacing EEPROMWearLevel
  DarkLight Cover Calibrator - ESP32-S3 Port

  (c) Copyright Nathan Woelfle 2020-present day. All Rights Reserved.
  Creative Commons Attribution-NonCommercial 4.0 International License
*/

#include "storage_manager.h"
#include "Debug.h"

StorageManager storage;

void StorageManager::begin() {
  _prefs.begin("dlc", false);
  Debug::info("STORAGE", "NVS preferences initialized");
}

// --- Original firmware values ---

uint8_t StorageManager::loadCoverState() {
  return _prefs.getUChar(KEY_COVER_STATE, COVER_UNKNOWN);
}

void StorageManager::saveCoverState(uint8_t state) {
  _prefs.putUChar(KEY_COVER_STATE, state);
}

uint16_t StorageManager::loadPanelValue() {
  return _prefs.getUShort(KEY_PANEL_VALUE, LIGHT_PWM_MAX);
}

void StorageManager::savePanelValue(uint16_t value) {
  _prefs.putUShort(KEY_PANEL_VALUE, value);
}

uint16_t StorageManager::loadBroadband() {
  return _prefs.getUShort(KEY_BROADBAND, 25);
}

void StorageManager::saveBroadband(uint16_t value) {
  _prefs.putUShort(KEY_BROADBAND, value);
}

uint16_t StorageManager::loadNarrowband() {
  return _prefs.getUShort(KEY_NARROWBAND, LIGHT_PWM_MAX);
}

void StorageManager::saveNarrowband(uint16_t value) {
  _prefs.putUShort(KEY_NARROWBAND, value);
}

// --- Servo configuration ---

uint16_t StorageManager::loadServoOpenAngle() {
  return _prefs.getUShort(KEY_SERVO_OPEN, DEFAULT_SERVO_OPEN_ANGLE);
}

void StorageManager::saveServoOpenAngle(uint16_t angle) {
  _prefs.putUShort(KEY_SERVO_OPEN, angle);
}

uint16_t StorageManager::loadServoCloseAngle() {
  return _prefs.getUShort(KEY_SERVO_CLOSE, DEFAULT_SERVO_CLOSE_ANGLE);
}

void StorageManager::saveServoCloseAngle(uint16_t angle) {
  _prefs.putUShort(KEY_SERVO_CLOSE, angle);
}

uint16_t StorageManager::loadServoMinPulse() {
  return _prefs.getUShort(KEY_SERVO_MIN_PW, DEFAULT_SERVO_MIN_PULSE);
}

void StorageManager::saveServoMinPulse(uint16_t pw) {
  _prefs.putUShort(KEY_SERVO_MIN_PW, pw);
}

uint16_t StorageManager::loadServoMaxPulse() {
  return _prefs.getUShort(KEY_SERVO_MAX_PW, DEFAULT_SERVO_MAX_PULSE);
}

void StorageManager::saveServoMaxPulse(uint16_t pw) {
  _prefs.putUShort(KEY_SERVO_MAX_PW, pw);
}

uint32_t StorageManager::loadMoveTime() {
  return _prefs.getULong(KEY_MOVE_TIME, DEFAULT_TIME_TO_MOVE);
}

void StorageManager::saveMoveTime(uint32_t ms) {
  _prefs.putULong(KEY_MOVE_TIME, ms);
}

uint16_t StorageManager::loadServoRangeMin() {
  return _prefs.getUShort(KEY_SERVO_RANGE_MIN, DEFAULT_SERVO_RANGE_MIN);
}

void StorageManager::saveServoRangeMin(uint16_t angle) {
  _prefs.putUShort(KEY_SERVO_RANGE_MIN, angle);
}

uint16_t StorageManager::loadServoRangeMax() {
  return _prefs.getUShort(KEY_SERVO_RANGE_MAX, DEFAULT_SERVO_RANGE_MAX);
}

void StorageManager::saveServoRangeMax(uint16_t angle) {
  _prefs.putUShort(KEY_SERVO_RANGE_MAX, angle);
}

// --- Light configuration ---

uint16_t StorageManager::loadMaxBrightness() {
  return _prefs.getUShort(KEY_MAX_BRIGHT, DEFAULT_MAX_BRIGHTNESS);
}

void StorageManager::saveMaxBrightness(uint16_t value) {
  _prefs.putUShort(KEY_MAX_BRIGHT, value);
}

uint32_t StorageManager::loadStabilizeTime() {
  return _prefs.getULong(KEY_STAB_TIME, DEFAULT_STABILIZE_TIME);
}

void StorageManager::saveStabilizeTime(uint32_t ms) {
  _prefs.putULong(KEY_STAB_TIME, ms);
}

// --- Heater configuration ---

uint8_t StorageManager::loadHeaterMode() {
  return _prefs.getUChar(KEY_HEATER_MODE, HEATER_OFF);
}

void StorageManager::saveHeaterMode(uint8_t mode) {
  _prefs.putUChar(KEY_HEATER_MODE, mode);
}

float StorageManager::loadDeltaPoint() {
  return _prefs.getFloat(KEY_DELTA_POINT, DEFAULT_DELTA_POINT);
}

void StorageManager::saveDeltaPoint(float value) {
  _prefs.putFloat(KEY_DELTA_POINT, value);
}

uint32_t StorageManager::loadShutoffTime() {
  return _prefs.getULong(KEY_SHUTOFF_TIME, DEFAULT_HEATER_SHUTOFF);
}

void StorageManager::saveShutoffTime(uint32_t ms) {
  _prefs.putULong(KEY_SHUTOFF_TIME, ms);
}

// --- WiFi configuration ---

String StorageManager::loadWifiSSID() {
  return _prefs.getString(KEY_WIFI_SSID, "");
}

void StorageManager::saveWifiSSID(const String& ssid) {
  _prefs.putString(KEY_WIFI_SSID, ssid);
}

String StorageManager::loadWifiPass() {
  return _prefs.getString(KEY_WIFI_PASS, "");
}

void StorageManager::saveWifiPass(const String& pass) {
  _prefs.putString(KEY_WIFI_PASS, pass);
}
