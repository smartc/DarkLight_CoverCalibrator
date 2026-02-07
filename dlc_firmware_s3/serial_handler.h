/*
  serial_handler.h - USB serial <command> protocol (backward compatible)
  DarkLight Cover Calibrator - ESP32-S3 Port

  (c) Copyright Nathan Woelfle 2020-present day. All Rights Reserved.
  Creative Commons Attribution-NonCommercial 4.0 International License
*/

#ifndef SERIAL_HANDLER_H
#define SERIAL_HANDLER_H

#include <Arduino.h>
#include "config.h"

#ifdef ENABLE_SERIAL_CONTROL

class SerialHandler {
public:
  void begin();
  void loop();

private:
  char _receivedChars[MAX_RECV_CHARS];
  char _response[MAX_SEND_CHARS];
  bool _commandComplete = false;

  void checkSerial();
  void processCommand();
  void respondToCommand(const char* resp);
};

extern SerialHandler serialHandler;

#endif // ENABLE_SERIAL_CONTROL
#endif // SERIAL_HANDLER_H
