/*
  alpaca_handler.h - ASCOM Alpaca CoverCalibratorV2 REST API
  DarkLight Cover Calibrator - ESP32-S3 Port

  Implements ICoverCalibratorV2 interface for Conform Universal compliance.

  (c) Copyright Nathan Woelfle 2020-present day. All Rights Reserved.
  Creative Commons Attribution-NonCommercial 4.0 International License
*/

#ifndef ALPACA_HANDLER_H
#define ALPACA_HANDLER_H

#include <Arduino.h>
#include <WebServer.h>
#include <WiFiUdp.h>
#include <ArduinoJson.h>
#include "config.h"

class AlpacaHandler {
public:
  void begin();
  void loop();
  bool isRunning() const { return _running; }

private:
  WebServer _server;
  WiFiUDP _udp;
  bool _running = false;
  bool _connected = false;
  uint32_t _serverTransactionID = 0;
  String _uniqueID;

  void setupRoutes();
  void startDiscovery();
  void handleDiscovery();

  // Response helpers
  void sendValueResponse(int errorNumber, const char* errorMessage, int value);
  void sendValueResponse(int errorNumber, const char* errorMessage, bool value);
  void sendValueResponse(int errorNumber, const char* errorMessage, const char* value);
  void sendArrayResponse(JsonArray& arr);
  void sendMethodResponse(int errorNumber, const char* errorMessage);
  void sendErrorResponse(int errorNumber, const char* errorMessage);

  int32_t getClientTransactionID();
  int32_t getClientID();
  String findArgCaseInsensitive(const char* name);
  bool checkConnected(); // Returns false and sends 0x407 if not connected

  // Management API
  void handleApiVersions();
  void handleDescription();
  void handleConfiguredDevices();

  // Setup pages (HTML on Alpaca port)
  void handleSetupPage();
  void handleDeviceSetupPage();

  // Common device properties (GET)
  void handleGetConnected();
  void handleGetConnecting();
  void handleGetDescription();
  void handleGetDriverInfo();
  void handleGetDriverVersion();
  void handleGetInterfaceVersion();
  void handleGetName();
  void handleGetSupportedActions();
  void handleGetDeviceState();

  // Common device methods (PUT)
  void handlePutConnected();
  void handlePutConnect();
  void handlePutDisconnect();
  void handlePutAction();
  void handlePutCommandBlind();
  void handlePutCommandBool();
  void handlePutCommandString();

  // CoverCalibrator properties (GET)
  void handleGetBrightness();
  void handleGetCalibratorState();
  void handleGetCoverState();
  void handleGetMaxBrightness();
  void handleGetCoverMoving();
  void handleGetCalibratorChanging();

  // CoverCalibrator methods (PUT)
  void handlePutCalibratorOff();
  void handlePutCalibratorOn();
  void handlePutCloseCover();
  void handlePutHaltCover();
  void handlePutOpenCover();

  AlpacaHandler() : _server(ALPACA_PORT) {}
  friend AlpacaHandler& getAlpacaHandler();
};

AlpacaHandler& getAlpacaHandler();

#endif // ALPACA_HANDLER_H
