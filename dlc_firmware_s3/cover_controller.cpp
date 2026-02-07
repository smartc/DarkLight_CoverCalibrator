/*
  cover_controller.cpp - Servo cover control with easing motion profiles
  DarkLight Cover Calibrator - ESP32-S3 Port

  Ported from dlc_firmware.ino lines 143-1067 (minus secondary servo)

  Key differences from AVR version:
  - Uses ESP32Servo (Kevin Harrington) with writeMicroseconds() for 270-degree support
  - Angle-to-microsecond mapping done internally (never relies on Servo::write/read)
  - Attach before write to avoid undefined initial position
  - Tracks position with _lastPosition member instead of Servo::read()

  (c) Copyright Nathan Woelfle 2020-present day. All Rights Reserved.
  Creative Commons Attribution-NonCommercial 4.0 International License
*/

#include "cover_controller.h"

#ifdef COVER_INSTALLED

#include "storage_manager.h"
#include "Debug.h"

CoverController cover;

void CoverController::begin() {
  #ifdef ENABLE_SAVING_TO_MEMORY
    _currentState = (CoverState)storage.loadCoverState();
    if (_currentState == 0) _currentState = COVER_UNKNOWN;

    // Load servo configuration from NVS
    _openAngle  = storage.loadServoOpenAngle();
    _closeAngle = storage.loadServoCloseAngle();
    _minPulse   = storage.loadServoMinPulse();
    _maxPulse   = storage.loadServoMaxPulse();
    _timeToMove = storage.loadMoveTime();
    _rangeMin   = storage.loadServoRangeMin();
    _rangeMax   = storage.loadServoRangeMax();
  #else
    _currentState = COVER_UNKNOWN;
  #endif

  // CRITICAL: attach servo FIRST, then write position.
  // ESP32Servo ignores write() if not attached.
  attachServo();

  // Move to known position based on saved state
  if (_currentState == COVER_OPEN) {
    writeAngle(_openAngle);
    _lastPosition = _openAngle;
    _currentState = COVER_OPEN;
  } else {
    writeAngle(_closeAngle);
    _lastPosition = _closeAngle;
    _currentState = COVER_CLOSED;
  }

  _previousMoveCoverTo = (uint8_t)_currentState;
  setDetachTimer();

  Debug::infof("COVER", "Initialized: state=%d, open=%d, close=%d, time=%lu",
               _currentState, _openAngle, _closeAngle, _timeToMove);
}

void CoverController::loop() {
  processCoverMovement();

  if (_detachPending) {
    completeDetach();
  }
}

void CoverController::openCover() {
  // If not already moving, open, or not present
  if (_currentState != COVER_MOVING && _currentState != COVER_OPEN && _currentState != COVER_NOT_PRESENT) {
    if (_currentState == COVER_CLOSED) {
      // Notify that we're about to open (light off, heater off callbacks)
      if (_onOpenStart) _onOpenStart();
    }
    _moveCoverTo = 3; // Open
    setMovement();
    Debug::info("COVER", "Opening cover");
  }
}

void CoverController::closeCover() {
  // If not already moving, closed, or not present
  if (_currentState != COVER_MOVING && _currentState != COVER_CLOSED && _currentState != COVER_NOT_PRESENT) {
    _moveCoverTo = 1; // Close
    setMovement();
    Debug::info("COVER", "Closing cover");
  }
}

void CoverController::haltCover() {
  if (_currentState == COVER_MOVING) {
    _halt = true;
    _previousMoveCoverTo = _moveCoverTo;
    _currentState = COVER_UNKNOWN;
    _elapsedMoveTime += millis() - _startServoTimer;
    setDetachTimer();
    Debug::info("COVER", "Halting cover");
  }
}

int16_t CoverController::nudgeServo(int16_t direction) {
  // Don't nudge while cover is in motion
  if (_currentState == COVER_MOVING) return _lastPosition;

  int16_t newPos = _lastPosition + direction;
  newPos = constrain(newPos, (int16_t)_rangeMin, (int16_t)_rangeMax);

  // Attach, write current position first to prevent snap, then move
  attachServo();
  writeAngle(_lastPosition);
  delay(20); // brief settle
  writeAngle(newPos);
  _lastPosition = newPos;
  _currentState = COVER_UNKNOWN;
  setDetachTimer();

  Debug::infof("COVER", "Nudge to %d", newPos);
  return newPos;
}

int16_t CoverController::setCurrentAsOpen() {
  _openAngle = _lastPosition;
  #ifdef ENABLE_SAVING_TO_MEMORY
    storage.saveServoOpenAngle(_openAngle);
  #endif
  Debug::infof("COVER", "Open angle set to %d", _openAngle);
  return _openAngle;
}

int16_t CoverController::setCurrentAsClose() {
  _closeAngle = _lastPosition;
  #ifdef ENABLE_SAVING_TO_MEMORY
    storage.saveServoCloseAngle(_closeAngle);
  #endif
  Debug::infof("COVER", "Close angle set to %d", _closeAngle);
  return _closeAngle;
}

void CoverController::attachServo() {
  _servo.attach(PIN_SERVO, _minPulse, _maxPulse);
}

void CoverController::setDetachTimer() {
  _detachPending = true;
  _startDetachTimer = millis();
}

void CoverController::completeDetach() {
  if (millis() - _startDetachTimer >= SERVO_DETACH_TIME) {
    _servo.detach();
    _detachPending = false;
    Debug::debug("COVER", "Servo detached");
  }
}

// Maps angle (0 to SERVO_MAX_ANGLE) to microseconds and writes to servo.
// This bypasses Servo::write() which only supports 0-180.
void CoverController::writeAngle(int16_t angle) {
  angle = constrain(angle, 0, DEFAULT_SERVO_MAX_ANGLE);
  int us = map(angle, 0, DEFAULT_SERVO_MAX_ANGLE, _minPulse, _maxPulse);
  _servo.writeMicroseconds(us);
  _previousWrittenAngle = angle;
}

void CoverController::setMovement() {
  _detachPending = false; // Reset in case restart issued right after halt

  // Use tracked _lastPosition instead of _servo.read() — _servo.read() is
  // unreliable on ESP32Servo after detach/reattach cycles.
  #ifdef USE_LINEAR
    if (!_halt) {
      // _lastPosition already holds the correct position
    } else {
      if (_moveCoverTo != _previousMoveCoverTo) {
        _elapsedMoveTime = _timeToMove - _elapsedMoveTime;
        _lastPosition = (_moveCoverTo == 3) ? _closeAngle : _openAngle;
      }
    }
  #else
    if (_halt && _moveCoverTo != _previousMoveCoverTo) {
      _elapsedMoveTime = _timeToMove - _elapsedMoveTime;
    }

    // _lastPosition already accurate from tracking
    _remainingDistance = (_moveCoverTo == 3) ? (_openAngle - _lastPosition) : (_closeAngle - _lastPosition);

    if (abs(_remainingDistance) > abs((int16_t)_openAngle - (int16_t)_closeAngle) / 2) {
      _elapsedMoveTime = 0;
    }
  #endif

  attachServo();
  // Write current position immediately after attach to prevent servo snapping
  writeAngle(_lastPosition);

  _currentState = COVER_MOVING;
  _previousWrittenAngle = -1; // reset so first movement frame always writes
  _startServoTimer = millis();
  _halt = false;
}

void CoverController::processCoverMovement() {
  // Monitor moving and unknown cover
  if (_currentState == COVER_MOVING || _currentState == COVER_UNKNOWN) {
    uint32_t currentMillis = millis();

    // Report ERROR if timeToMove * 2 reached
    if (currentMillis - _startServoTimer >= _timeToMove * 2) {
      _currentState = COVER_ERROR;
      #ifdef ENABLE_SAVING_TO_MEMORY
        storage.saveCoverState((uint8_t)_currentState);
      #endif
      Debug::error("COVER", "Movement timeout - ERROR state");
      return;
    }

    // If moving, then move cover
    if (_currentState == COVER_MOVING) {
      uint32_t currentServoTimer = millis();
      float progress = (float)(currentServoTimer - _startServoTimer + _elapsedMoveTime) / _timeToMove;
      progress = constrain(progress, 0.0f, 1.0f);

      int16_t targetPosition = (_moveCoverTo == 3) ? _openAngle : _closeAngle;

      int16_t currentAngle = calculateServoPosition(
        currentServoTimer, _startServoTimer, _lastPosition, targetPosition,
        progress, _remainingDistance, _openAngle, _closeAngle
      );

      // Only write to servo if angle actually changed (reduces bus noise)
      if (currentAngle != _previousWrittenAngle) {
        writeAngle(currentAngle);
      }

      if (progress >= 1.0f) {
        // Movement complete
        if (_moveCoverTo == 1) {
          // Cover closed - trigger callbacks
          if (_onCloseComplete) _onCloseComplete();
        }

        _elapsedMoveTime = 0;
        _lastPosition = currentAngle;
        _previousMoveCoverTo = _currentState = (_moveCoverTo == 3) ? COVER_OPEN : COVER_CLOSED;

        #ifdef ENABLE_SAVING_TO_MEMORY
          storage.saveCoverState((uint8_t)_currentState);
        #endif

        setDetachTimer();
        Debug::infof("COVER", "Movement complete: state=%d", _currentState);
      }
    }
  }
}

int CoverController::calculateServoPosition(uint32_t currentTime, uint32_t startTime,
                                             int lastPos, int targetPos, float progress,
                                             int remainDist, int openAngle, int closeAngle) {
  #ifdef USE_LINEAR
    return lastPos + (targetPos - lastPos) * progress;
  #else
    if (abs(remainDist) > abs(openAngle - closeAngle) / 2) {
      float easedProgress = calculateEasedProgress(progress);
      return lastPos + (targetPos - lastPos) * easedProgress;
    } else {
      float adjustedProgress = (float)(currentTime - startTime) / (_timeToMove - _elapsedMoveTime);
      adjustedProgress = constrain(adjustedProgress, 0.0f, 1.0f);
      return lastPos + (targetPos - lastPos) * adjustedProgress;
    }
  #endif
}

float CoverController::calculateEasedProgress(float progress) {
  #ifdef USE_CIRCULAR
    return (progress < 0.5f) ? 0.5f * (1.0f - sqrt(1.0f - 4.0f * pow(progress, 2)))
                             : 0.5f * (sqrt(-((2.0f * progress) - 3.0f) * ((2.0f * progress) - 1.0f)) + 1.0f);
  #elif defined(USE_CUBIC)
    return (progress < 0.5f) ? 4.0f * progress * progress * progress
                             : 1.0f - pow(-2.0f * progress + 2.0f, 3) / 2.0f;
  #elif defined(USE_EXPO)
    return (progress == 0.0f) ? 0.0f : (progress == 1.0f) ? 1.0f
           : (progress < 0.5f) ? pow(2.0f, 20.0f * progress - 10.0f) / 2.0f
                               : (2.0f - pow(2.0f, -20.0f * progress + 10.0f)) / 2.0f;
  #elif defined(USE_QUAD)
    return (progress < 0.5f) ? 2.0f * progress * progress
                             : 1.0f - pow(-2.0f * progress + 2.0f, 2) / 2.0f;
  #elif defined(USE_QUART)
    return (progress < 0.5f) ? 8.0f * progress * progress * progress * progress
                             : 1.0f - pow(-2.0f * progress + 2.0f, 4) / 2.0f;
  #elif defined(USE_QUINT)
    return (progress < 0.5f) ? 16.0f * progress * progress * progress * progress * progress
                             : 1.0f - pow(-2.0f * progress + 2.0f, 5) / 2.0f;
  #elif defined(USE_SINE)
    return -(cos(M_PI * progress) - 1.0f) / 2.0f;
  #else
    return progress; // Linear fallback
  #endif
}

#endif // COVER_INSTALLED
