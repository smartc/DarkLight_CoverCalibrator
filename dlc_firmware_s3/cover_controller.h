/*
  cover_controller.h - Servo cover control with easing motion profiles
  DarkLight Cover Calibrator - ESP32-S3 Port

  (c) Copyright Nathan Woelfle 2020-present day. All Rights Reserved.
  Creative Commons Attribution-NonCommercial 4.0 International License
*/

#ifndef COVER_CONTROLLER_H
#define COVER_CONTROLLER_H

#include <Arduino.h>
#include "config.h"

#ifdef COVER_INSTALLED

#include <ESP32Servo.h>

class CoverController {
public:
  void begin();
  void loop();

  void openCover();
  void closeCover();
  void haltCover();

  // Manual positioning for setup
  int16_t  nudgeServo(int16_t direction);   // move +/-1 degree, returns new position
  int16_t  setCurrentAsOpen();              // set current position as open angle, returns angle
  int16_t  setCurrentAsClose();             // set current position as close angle, returns angle

  CoverState getState() const { return _currentState; }
  uint8_t    getMoveTo() const { return _moveCoverTo; }
  uint8_t    getPreviousMoveTo() const { return _previousMoveCoverTo; }
  int16_t    getCurrentPosition() const { return _lastPosition; }

  // Configuration accessors (uint16_t for 270-degree servo support)
  void setServoOpenAngle(uint16_t angle)  { _openAngle = angle; }
  void setServoCloseAngle(uint16_t angle) { _closeAngle = angle; }
  void setServoMinPulse(uint16_t pw)      { _minPulse = pw; }
  void setServoMaxPulse(uint16_t pw)      { _maxPulse = pw; }
  void setMoveTime(uint32_t ms)           { _timeToMove = ms; }
  void setRangeMin(uint16_t angle)        { _rangeMin = angle; }
  void setRangeMax(uint16_t angle)        { _rangeMax = angle; }

  uint16_t getServoOpenAngle() const  { return _openAngle; }
  uint16_t getServoCloseAngle() const { return _closeAngle; }
  uint16_t getServoMinPulse() const   { return _minPulse; }
  uint16_t getServoMaxPulse() const   { return _maxPulse; }
  uint32_t getMoveTime() const        { return _timeToMove; }
  uint16_t getRangeMin() const        { return _rangeMin; }
  uint16_t getRangeMax() const        { return _rangeMax; }

  // Callbacks for cross-module coordination
  using CoverCallback = void (*)();
  void setOnCloseComplete(CoverCallback cb) { _onCloseComplete = cb; }
  void setOnOpenStart(CoverCallback cb)     { _onOpenStart = cb; }

private:
  Servo _servo;
  CoverState _currentState = COVER_UNKNOWN;
  uint8_t  _moveCoverTo = 0;
  uint8_t  _previousMoveCoverTo = 0;

  // Servo configuration (uint16_t for 0-270 degree range)
  uint16_t _openAngle  = DEFAULT_SERVO_OPEN_ANGLE;
  uint16_t _closeAngle = DEFAULT_SERVO_CLOSE_ANGLE;
  uint16_t _minPulse   = DEFAULT_SERVO_MIN_PULSE;
  uint16_t _maxPulse   = DEFAULT_SERVO_MAX_PULSE;
  uint32_t _timeToMove = DEFAULT_TIME_TO_MOVE;
  uint16_t _rangeMin   = DEFAULT_SERVO_RANGE_MIN;
  uint16_t _rangeMax   = DEFAULT_SERVO_RANGE_MAX;

  // Movement state (int16_t to handle negative intermediate values)
  uint32_t _startServoTimer = 0;
  uint32_t _elapsedMoveTime = 0;
  bool     _halt = false;
  int16_t  _lastPosition = 0;
  int16_t  _remainingDistance = 0;
  int16_t  _previousWrittenAngle = -1; // tracks last angle sent to servo

  // Detach state
  uint32_t _startDetachTimer = 0;
  bool     _detachPending = false;

  // Callbacks
  CoverCallback _onCloseComplete = nullptr;
  CoverCallback _onOpenStart = nullptr;

  void attachServo();
  void setDetachTimer();
  void completeDetach();
  void setMovement();
  void processCoverMovement();
  void writeAngle(int16_t angle); // maps angle to microseconds and writes
  int  calculateServoPosition(uint32_t currentTime, uint32_t startTime,
                              int lastPos, int targetPos, float progress,
                              int remainDist, int openAngle, int closeAngle);
  float calculateEasedProgress(float progress);
};

extern CoverController cover;

#endif // COVER_INSTALLED
#endif // COVER_CONTROLLER_H
