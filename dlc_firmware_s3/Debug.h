/*
  Debug.h - Leveled debug logging
  DarkLight Cover Calibrator - ESP32-S3 Port

  (c) Copyright Nathan Woelfle 2020-present day. All Rights Reserved.
  Creative Commons Attribution-NonCommercial 4.0 International License
*/

#ifndef DEBUG_H
#define DEBUG_H

#include <Arduino.h>

// Debug levels
enum DebugLevel : uint8_t {
  DBG_NONE    = 0,
  DBG_ERROR   = 1,
  DBG_WARNING = 2,
  DBG_INFO    = 3,
  DBG_DEBUG   = 4,
  DBG_VERBOSE = 5
};

class Debug {
public:
  static void begin(DebugLevel level = DBG_INFO);
  static void setLevel(DebugLevel level);
  static DebugLevel getLevel();

  static void error(const char* module, const char* msg);
  static void warning(const char* module, const char* msg);
  static void info(const char* module, const char* msg);
  static void debug(const char* module, const char* msg);
  static void verbose(const char* module, const char* msg);

  // Formatted variants
  static void errorf(const char* module, const char* fmt, ...);
  static void warningf(const char* module, const char* fmt, ...);
  static void infof(const char* module, const char* fmt, ...);
  static void debugf(const char* module, const char* fmt, ...);
  static void verbosef(const char* module, const char* fmt, ...);

private:
  static DebugLevel _level;
  static void log(DebugLevel level, const char* module, const char* msg);
  static void logf(DebugLevel level, const char* module, const char* fmt, va_list args);
  static const char* levelStr(DebugLevel level);
};

#endif // DEBUG_H
