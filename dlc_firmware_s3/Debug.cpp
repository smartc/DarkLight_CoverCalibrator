/*
  Debug.cpp - Leveled debug logging
  DarkLight Cover Calibrator - ESP32-S3 Port

  (c) Copyright Nathan Woelfle 2020-present day. All Rights Reserved.
  Creative Commons Attribution-NonCommercial 4.0 International License
*/

#include "Debug.h"
#include <stdarg.h>

DebugLevel Debug::_level = DBG_INFO;

void Debug::begin(DebugLevel level) {
  _level = level;
}

void Debug::setLevel(DebugLevel level) {
  _level = level;
}

DebugLevel Debug::getLevel() {
  return _level;
}

void Debug::error(const char* module, const char* msg) {
  log(DBG_ERROR, module, msg);
}

void Debug::warning(const char* module, const char* msg) {
  log(DBG_WARNING, module, msg);
}

void Debug::info(const char* module, const char* msg) {
  log(DBG_INFO, module, msg);
}

void Debug::debug(const char* module, const char* msg) {
  log(DBG_DEBUG, module, msg);
}

void Debug::verbose(const char* module, const char* msg) {
  log(DBG_VERBOSE, module, msg);
}

void Debug::errorf(const char* module, const char* fmt, ...) {
  va_list args;
  va_start(args, fmt);
  logf(DBG_ERROR, module, fmt, args);
  va_end(args);
}

void Debug::warningf(const char* module, const char* fmt, ...) {
  va_list args;
  va_start(args, fmt);
  logf(DBG_WARNING, module, fmt, args);
  va_end(args);
}

void Debug::infof(const char* module, const char* fmt, ...) {
  va_list args;
  va_start(args, fmt);
  logf(DBG_INFO, module, fmt, args);
  va_end(args);
}

void Debug::debugf(const char* module, const char* fmt, ...) {
  va_list args;
  va_start(args, fmt);
  logf(DBG_DEBUG, module, fmt, args);
  va_end(args);
}

void Debug::verbosef(const char* module, const char* fmt, ...) {
  va_list args;
  va_start(args, fmt);
  logf(DBG_VERBOSE, module, fmt, args);
  va_end(args);
}

void Debug::log(DebugLevel level, const char* module, const char* msg) {
  if (level > _level) return;
  Serial.printf("[%s][%s] %s\n", levelStr(level), module, msg);
}

void Debug::logf(DebugLevel level, const char* module, const char* fmt, va_list args) {
  if (level > _level) return;
  Serial.printf("[%s][%s] ", levelStr(level), module);
  char buf[256];
  vsnprintf(buf, sizeof(buf), fmt, args);
  Serial.println(buf);
}

const char* Debug::levelStr(DebugLevel level) {
  switch (level) {
    case DBG_ERROR:   return "ERR";
    case DBG_WARNING: return "WRN";
    case DBG_INFO:    return "INF";
    case DBG_DEBUG:   return "DBG";
    case DBG_VERBOSE: return "VRB";
    default:          return "???";
  }
}
