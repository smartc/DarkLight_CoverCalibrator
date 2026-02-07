/*
  button_handler.cpp - Single-button state machine for cover/light control
  DarkLight Cover Calibrator - ESP32-S3 Port

  Single button replaces the two-button system from the original firmware.
  - Any press while cover moving -> immediate halt
  - Short press (<1s) -> toggle light on/off
  - Long press (>=1s) -> toggle cover open/close

  (c) Copyright Nathan Woelfle 2020-present day. All Rights Reserved.
  Creative Commons Attribution-NonCommercial 4.0 International License
*/

#include "button_handler.h"

#ifdef ENABLE_MANUAL_CONTROL

#include "Debug.h"

#ifdef COVER_INSTALLED
  #include "cover_controller.h"
#endif
#ifdef LIGHT_INSTALLED
  #include "light_controller.h"
#endif

ButtonHandler button;

void ButtonHandler::begin() {
  pinMode(PIN_BUTTON, INPUT_PULLUP);
  Debug::info("BUTTON", "Button handler initialized on IO46");
}

void ButtonHandler::loop() {
  uint8_t reading = digitalRead(PIN_BUTTON);

  switch (_state) {
    case BTN_IDLE:
      // Detect falling edge (button pressed, active LOW)
      if (reading == LOW && _lastButtonState == HIGH) {
        if (millis() - _lastDebounceTime > DEBOUNCE_DELAY) {
          _lastDebounceTime = millis();
          _pressStartTime = millis();
          _longPressHandled = false;

          // Immediate halt if cover is moving
          #ifdef COVER_INSTALLED
            if (cover.getState() == COVER_MOVING) {
              cover.haltCover();
              _state = BTN_AWAITING_RELEASE;
              Debug::info("BUTTON", "Halt (press during move)");
              break;
            }
          #endif

          _state = BTN_PRESSED;
        }
      }
      break;

    case BTN_PRESSED:
      // Check for long press while still held
      if (reading == LOW) {
        if (!_longPressHandled && (millis() - _pressStartTime >= LONG_PRESS_TIME)) {
          _longPressHandled = true;

          // Long press: toggle cover
          #ifdef COVER_INSTALLED
            CoverState cs = cover.getState();
            if (cs == COVER_CLOSED || cs == COVER_UNKNOWN || cs == COVER_ERROR) {
              cover.openCover();
              Debug::info("BUTTON", "Long press -> Open cover");
            } else if (cs == COVER_OPEN) {
              cover.closeCover();
              Debug::info("BUTTON", "Long press -> Close cover");
            }
          #endif

          _state = BTN_AWAITING_RELEASE;
        }
      }
      // Button released before long press threshold
      else {
        if (!_longPressHandled && (millis() - _pressStartTime < LONG_PRESS_TIME)) {
          // Short press: toggle light
          #ifdef LIGHT_INSTALLED
            if (light.getState() == CAL_OFF) {
              light.turnPanelTo(light.getMaxBrightness());
              Debug::info("BUTTON", "Short press -> Light ON");
            } else {
              light.turnPanelOff();
              Debug::info("BUTTON", "Short press -> Light OFF");
            }
          #endif
        }
        _state = BTN_IDLE;
      }
      break;

    case BTN_AWAITING_RELEASE:
      if (reading == HIGH) {
        _state = BTN_IDLE;
      }
      break;
  }

  _lastButtonState = reading;
}

#endif // ENABLE_MANUAL_CONTROL
