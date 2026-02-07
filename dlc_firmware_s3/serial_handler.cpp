/*
  serial_handler.cpp - USB serial <command> protocol (backward compatible)
  DarkLight Cover Calibrator - ESP32-S3 Port

  Preserves exact protocol from dlc_firmware.ino for INDI/ASCOM driver compatibility.

  (c) Copyright Nathan Woelfle 2020-present day. All Rights Reserved.
  Creative Commons Attribution-NonCommercial 4.0 International License
*/

#include "serial_handler.h"

#ifdef ENABLE_SERIAL_CONTROL

#include "Debug.h"

#ifdef COVER_INSTALLED
  #include "cover_controller.h"
#endif
#ifdef LIGHT_INSTALLED
  #include "light_controller.h"
#endif
#ifdef HEATER_INSTALLED
  #include "heater_controller.h"
#endif

SerialHandler serialHandler;

void SerialHandler::begin() {
  Serial.begin(SERIAL_SPEED);
  Serial.flush();
  Debug::info("SERIAL", "Serial handler initialized");
}

void SerialHandler::loop() {
  checkSerial();
  if (_commandComplete) {
    processCommand();
  }
}

void SerialHandler::checkSerial() {
  static uint8_t index = 0;
  static bool receiveInProgress = false;

  while (Serial.available() > 0 && !_commandComplete) {
    char incomingChar = Serial.read();

    if (receiveInProgress) {
      if (incomingChar != SERIAL_END_MARKER) {
        if (incomingChar == SERIAL_START_MARKER) {
          index = 0;
          memset(_receivedChars, 0, sizeof(_receivedChars));
        } else {
          _receivedChars[index] = incomingChar;
          index++;
          if (index >= MAX_RECV_CHARS) {
            index = MAX_RECV_CHARS - 1;
          }
        }
      } else {
        _receivedChars[index] = '\0';
        receiveInProgress = false;
        _commandComplete = true;
        index = 0;
      }
    } else if (incomingChar == SERIAL_START_MARKER) {
      receiveInProgress = true;
      index = 0;
      memset(_receivedChars, 0, sizeof(_receivedChars));
    }
  }
}

void SerialHandler::processCommand() {
  char cmd = _receivedChars[0];
  char* cmdParameter = &_receivedChars[1];

  switch (cmd) {

    // Cover state: 0:NotPresent, 1:Closed, 2:Moving, 3:Open, 4:Unknown, 5:Error
    case 'P': {
      #ifdef COVER_INSTALLED
        itoa(cover.getState(), _response, 10);
      #else
        itoa(COVER_NOT_PRESENT, _response, 10);
      #endif
      respondToCommand(_response);
      break;
    }

    #ifdef COVER_INSTALLED
      case 'O':
        cover.openCover();
        respondToCommand(_receivedChars);
        break;

      case 'C':
        cover.closeCover();
        respondToCommand(_receivedChars);
        break;

      case 'H':
        cover.haltCover();
        respondToCommand(_receivedChars);
        break;
    #endif

    // Calibrator state: 0:NotPresent, 1:Off, 2:NotReady, 3:Ready, 4:Unknown, 5:Error
    case 'L': {
      #ifdef LIGHT_INSTALLED
        itoa(light.getState(), _response, 10);
      #else
        itoa(CAL_NOT_PRESENT, _response, 10);
      #endif
      respondToCommand(_response);
      break;
    }

    #ifdef LIGHT_INSTALLED
      case 'B':
        itoa(light.getCurrentBrightness(), _response, 10);
        respondToCommand(_response);
        break;

      case 'M':
        itoa(light.getMaxBrightness(), _response, 10);
        respondToCommand(_response);
        break;

      case 'T': {
        uint16_t value = atoi(cmdParameter);
        value = constrain(value, (uint16_t)0, light.getMaxBrightness());
        light.turnPanelTo(value);
        respondToCommand(_receivedChars);
        break;
      }

      case 'F':
        light.turnPanelOff();
        respondToCommand(_receivedChars);
        break;

      case 'A':
        light.setAutoON(true);
        respondToCommand(_receivedChars);
        break;

      case 'a':
        light.setAutoON(false);
        respondToCommand(_receivedChars);
        break;

      case 'S':
        light.setStabilizeTime(atoi(cmdParameter));
        respondToCommand(_receivedChars);
        break;

      case 'D':
        if (cmdParameter[0] == 'B') {
          light.saveBroadband();
        } else {
          light.saveNarrowband();
        }
        respondToCommand(_receivedChars);
        break;

      case 'G':
        if (cmdParameter[0] == 'B') {
          itoa(light.getBroadbandStep(), _response, 10);
        } else {
          itoa(light.getNarrowbandStep(), _response, 10);
        }
        respondToCommand(_response);
        break;
    #endif // LIGHT_INSTALLED

    // Heater state: 0:NotPresent, 1:Off, 2:Auto, 3:On, 4:Unknown, 5:Error, 6:Set
    case 'R': {
      #ifdef HEATER_INSTALLED
        itoa(heater.getState(), _response, 10);
      #else
        itoa(HEATER_NOT_PRESENT, _response, 10);
      #endif
      respondToCommand(_response);
      break;
    }

    #ifdef HEATER_INSTALLED
      case 'Y': {
        // Send all current heater data values
        // Format: h1t:<temp>:h1p:<pwm>|h2t:na:h2p:na|o:<temp>:h:<humidity>:d:<dewpoint>
        HeaterData data = heater.getHeaterData();
        char tempBuf[10];
        _response[0] = '\0';

        // Heater one data
        dtostrf(data.heaterTemp, 0, 1, tempBuf);
        snprintf(_response, MAX_SEND_CHARS, "h1t:%s:h1p:%d", tempBuf, data.heaterPWM);

        // Heater two - always "na" (removed in S3 port, field count preserved)
        snprintf(_response + strlen(_response), MAX_SEND_CHARS - strlen(_response),
                 "|h2t:na:h2p:na");

        // Outside temp
        dtostrf(data.outsideTemp, 0, 1, tempBuf);
        snprintf(_response + strlen(_response), MAX_SEND_CHARS - strlen(_response),
                 "|o:%s", tempBuf);

        // Humidity
        dtostrf(data.humidity, 0, 1, tempBuf);
        snprintf(_response + strlen(_response), MAX_SEND_CHARS - strlen(_response),
                 ":h:%s", tempBuf);

        // Dew point
        dtostrf(data.dewPoint, 0, 1, tempBuf);
        snprintf(_response + strlen(_response), MAX_SEND_CHARS - strlen(_response),
                 ":d:%s", tempBuf);

        respondToCommand(_response);
        break;
      }

      case 'Q':
        heater.setAutoHeat(true);
        respondToCommand(_receivedChars);
        break;

      case 'q':
        heater.setAutoHeat(false);
        respondToCommand(_receivedChars);
        break;

      case 'E':
        heater.setHeatOnClose(true);
        respondToCommand(_receivedChars);
        break;

      case 'e':
        heater.setHeatOnClose(false);
        respondToCommand(_receivedChars);
        break;

      case 'W':
        heater.setManualHeat(true);
        respondToCommand(_receivedChars);
        break;

      case 'w':
        heater.setManualHeat(false);
        respondToCommand(_receivedChars);
        break;
    #endif // HEATER_INSTALLED

    // Firmware version
    case 'V':
      respondToCommand(DLC_VERSION);
      break;

    // Handshake
    case 'Z':
      respondToCommand("?");
      break;

    // Unknown command
    default:
      respondToCommand("?");
      break;
  }
}

void SerialHandler::respondToCommand(const char* resp) {
  char buffer[MAX_SEND_CHARS];
  snprintf(buffer, sizeof(buffer), "%c%s%c", SERIAL_START_MARKER, resp, SERIAL_END_MARKER);
  Serial.print(buffer);
  _commandComplete = false;
}

#endif // ENABLE_SERIAL_CONTROL
