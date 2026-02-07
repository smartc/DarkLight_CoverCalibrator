/*
  button_handler.h - Single-button state machine for cover/light control
  DarkLight Cover Calibrator - ESP32-S3 Port

  (c) Copyright Nathan Woelfle 2020-present day. All Rights Reserved.
  Creative Commons Attribution-NonCommercial 4.0 International License
*/

#ifndef BUTTON_HANDLER_H
#define BUTTON_HANDLER_H

#include <Arduino.h>
#include "config.h"

#ifdef ENABLE_MANUAL_CONTROL

enum ButtonState : uint8_t {
  BTN_IDLE,
  BTN_PRESSED,
  BTN_AWAITING_RELEASE
};

class ButtonHandler {
public:
  void begin();
  void loop();

private:
  ButtonState _state = BTN_IDLE;
  uint32_t _pressStartTime = 0;
  uint32_t _lastDebounceTime = 0;
  uint8_t  _lastButtonState = HIGH;
  bool     _longPressHandled = false;

  static const uint32_t LONG_PRESS_TIME = 1000; // ms
};

extern ButtonHandler button;

#endif // ENABLE_MANUAL_CONTROL
#endif // BUTTON_HANDLER_H
