/*
  web_ui_handler.h - Web dashboard and configuration pages
  DarkLight Cover Calibrator - ESP32-S3 Port

  (c) Copyright Nathan Woelfle 2020-present day. All Rights Reserved.
  Creative Commons Attribution-NonCommercial 4.0 International License
*/

#ifndef WEB_UI_HANDLER_H
#define WEB_UI_HANDLER_H

#include <Arduino.h>
#include <WebServer.h>
#include "config.h"

class WebUIHandler {
public:
  void begin();
  void loop();
  bool isRunning() const { return _running; }

private:
  WebServer _server;
  bool _running = false;

  void setupRoutes();

  // Page handlers
  void handleDashboard();
  void handleSetup();

  // API handlers
  void handleApiStatus();
  void handleApiCommand();
  void handleApiSettings();
  void handleApiSaveWifi();
  void handleApiSaveServo();
  void handleApiServoNudge();
  void handleApiServoSetOpen();
  void handleApiServoSetClose();
  void handleApiSaveLight();
  void handleApiSaveHeater();
  void handleApiRestart();

  // Helper
  String processTemplate(const char* html);

  WebUIHandler() : _server(WEB_PORT) {}
  friend WebUIHandler& getWebUIHandler();
};

WebUIHandler& getWebUIHandler();

#endif // WEB_UI_HANDLER_H
