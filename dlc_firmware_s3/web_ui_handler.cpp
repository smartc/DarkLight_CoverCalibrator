/*
  web_ui_handler.cpp - Web dashboard and configuration pages
  DarkLight Cover Calibrator - ESP32-S3 Port

  (c) Copyright Nathan Woelfle 2020-present day. All Rights Reserved.
  Creative Commons Attribution-NonCommercial 4.0 International License
*/

#include "web_ui_handler.h"
#include "html_templates.h"
#include "storage_manager.h"
#include "Debug.h"
#include <ArduinoJson.h>
#include <ElegantOTA.h>

#ifdef COVER_INSTALLED
  #include "cover_controller.h"
#endif
#ifdef LIGHT_INSTALLED
  #include "light_controller.h"
#endif
#ifdef HEATER_INSTALLED
  #include "heater_controller.h"
#endif

// Singleton accessor
WebUIHandler& getWebUIHandler() {
  static WebUIHandler instance;
  return instance;
}

void WebUIHandler::begin() {
  setupRoutes();
  ElegantOTA.begin(&_server);
  _server.begin();
  _running = true;
  Debug::infof("WEBUI", "Web server started on port %d (OTA at /update)", WEB_PORT);
}

void WebUIHandler::loop() {
  if (!_running) return;
  _server.handleClient();
  ElegantOTA.loop();
}

void WebUIHandler::setupRoutes() {
  _server.on("/", HTTP_GET, [this]() { handleDashboard(); });
  _server.on("/setup", HTTP_GET, [this]() { handleSetup(); });
  _server.on("/api/status", HTTP_GET, [this]() { handleApiStatus(); });
  _server.on("/api/cmd", HTTP_POST, [this]() { handleApiCommand(); });
  _server.on("/api/settings", HTTP_GET, [this]() { handleApiSettings(); });
  _server.on("/api/wifi", HTTP_POST, [this]() { handleApiSaveWifi(); });
  _server.on("/api/servo", HTTP_POST, [this]() { handleApiSaveServo(); });
  _server.on("/api/servo/nudge", HTTP_POST, [this]() { handleApiServoNudge(); });
  _server.on("/api/servo/setopen", HTTP_POST, [this]() { handleApiServoSetOpen(); });
  _server.on("/api/servo/setclose", HTTP_POST, [this]() { handleApiServoSetClose(); });
  _server.on("/api/light", HTTP_POST, [this]() { handleApiSaveLight(); });
  _server.on("/api/heater", HTTP_POST, [this]() { handleApiSaveHeater(); });
  _server.on("/api/restart", HTTP_POST, [this]() { handleApiRestart(); });
}

String WebUIHandler::processTemplate(const char* html) {
  String page = String(html);
  page.replace("%STYLE%", getStyleCSS());
  return page;
}

void WebUIHandler::handleDashboard() {
  String page = processTemplate(getDashboardHTML());
  _server.send(200, "text/html", page);
}

void WebUIHandler::handleSetup() {
  String page = processTemplate(getSetupHTML());
  _server.send(200, "text/html", page);
}

void WebUIHandler::handleApiStatus() {
  JsonDocument doc;

  #ifdef COVER_INSTALLED
    doc["coverState"] = (int)cover.getState();
  #else
    doc["coverState"] = (int)COVER_NOT_PRESENT;
  #endif

  #ifdef LIGHT_INSTALLED
    doc["calState"] = (int)light.getState();
    doc["brightness"] = (int)light.getCurrentBrightness();
    doc["maxBrightness"] = (int)light.getMaxBrightness();
  #else
    doc["calState"] = (int)CAL_NOT_PRESENT;
    doc["brightness"] = 0;
    doc["maxBrightness"] = 0;
  #endif

  #ifdef HEATER_INSTALLED
    doc["heaterState"] = (int)heater.getState();
    HeaterData hd = heater.getHeaterData();
    doc["heaterTemp"] = hd.heaterTemp;
    doc["outsideTemp"] = hd.outsideTemp;
    doc["humidity"] = hd.humidity;
    doc["dewPoint"] = hd.dewPoint;
    doc["heaterPWM"] = (int)hd.heaterPWM;
  #else
    doc["heaterState"] = (int)HEATER_NOT_PRESENT;
    doc["heaterTemp"] = nullptr;
    doc["outsideTemp"] = nullptr;
    doc["humidity"] = nullptr;
    doc["dewPoint"] = nullptr;
    doc["heaterPWM"] = nullptr;
  #endif

  doc["version"] = DLC_VERSION;

  char buffer[512];
  serializeJson(doc, buffer, sizeof(buffer));
  _server.send(200, "application/json", buffer);
}

void WebUIHandler::handleApiCommand() {
  String action = _server.arg("action");

  #ifdef COVER_INSTALLED
    if (action == "opencover") cover.openCover();
    else if (action == "closecover") cover.closeCover();
    else if (action == "haltcover") cover.haltCover();
  #endif

  #ifdef LIGHT_INSTALLED
    if (action == "lighton") {
      int brightness = _server.arg("brightness").toInt();
      if (brightness <= 0) brightness = light.getMaxBrightness();
      light.turnPanelTo(brightness);
    }
    else if (action == "lightoff") light.turnPanelOff();
  #endif

  #ifdef HEATER_INSTALLED
    if (action == "autoheat") heater.setAutoHeat(true);
    else if (action == "manualheat") heater.setManualHeat(true);
    else if (action == "heatonclose") heater.setHeatOnClose(true);
    else if (action == "heateroff") heater.turnOff();
  #endif

  _server.send(200, "application/json", "{\"ok\":true}");
}

void WebUIHandler::handleApiSettings() {
  JsonDocument doc;

  doc["wifiSSID"] = storage.loadWifiSSID();

  #ifdef COVER_INSTALLED
    doc["servoOpen"] = cover.getServoOpenAngle();
    doc["servoClose"] = cover.getServoCloseAngle();
    doc["servoMinPW"] = cover.getServoMinPulse();
    doc["servoMaxPW"] = cover.getServoMaxPulse();
    doc["moveTime"] = cover.getMoveTime();
    doc["rangeMin"] = cover.getRangeMin();
    doc["rangeMax"] = cover.getRangeMax();
    doc["servoPos"] = cover.getCurrentPosition();
  #else
    doc["servoOpen"] = DEFAULT_SERVO_OPEN_ANGLE;
    doc["servoClose"] = DEFAULT_SERVO_CLOSE_ANGLE;
    doc["servoMinPW"] = DEFAULT_SERVO_MIN_PULSE;
    doc["servoMaxPW"] = DEFAULT_SERVO_MAX_PULSE;
    doc["moveTime"] = DEFAULT_TIME_TO_MOVE;
    doc["rangeMin"] = DEFAULT_SERVO_RANGE_MIN;
    doc["rangeMax"] = DEFAULT_SERVO_RANGE_MAX;
    doc["servoPos"] = 0;
  #endif

  #ifdef LIGHT_INSTALLED
    doc["maxBright"] = light.getMaxBrightness();
    doc["stabTime"] = light.getStabilizeTime();
  #else
    doc["maxBright"] = DEFAULT_MAX_BRIGHTNESS;
    doc["stabTime"] = DEFAULT_STABILIZE_TIME;
  #endif

  #ifdef HEATER_INSTALLED
    doc["deltaPoint"] = heater.getDeltaPoint();
    doc["shutoffTime"] = heater.getShutoffTime();
  #else
    doc["deltaPoint"] = DEFAULT_DELTA_POINT;
    doc["shutoffTime"] = DEFAULT_HEATER_SHUTOFF;
  #endif

  char buffer[512];
  serializeJson(doc, buffer, sizeof(buffer));
  _server.send(200, "application/json", buffer);
}

void WebUIHandler::handleApiSaveWifi() {
  String ssid = _server.arg("ssid");
  String pass = _server.arg("pass");

  if (ssid.length() == 0) {
    _server.send(200, "application/json", "{\"ok\":false,\"error\":\"SSID required\"}");
    return;
  }

  storage.saveWifiSSID(ssid);
  storage.saveWifiPass(pass);

  Debug::infof("WEBUI", "WiFi credentials saved: %s", ssid.c_str());
  _server.send(200, "application/json", "{\"ok\":true}");
}

void WebUIHandler::handleApiSaveServo() {
  #ifdef COVER_INSTALLED
    uint16_t openAngle = _server.arg("open").toInt();
    uint16_t closeAngle = _server.arg("close").toInt();
    uint16_t minPW = _server.arg("minpw").toInt();
    uint16_t maxPW = _server.arg("maxpw").toInt();
    uint32_t moveTime = _server.arg("movetime").toInt();
    uint16_t rangeMin = _server.arg("rangemin").toInt();
    uint16_t rangeMax = _server.arg("rangemax").toInt();

    cover.setServoOpenAngle(openAngle);
    cover.setServoCloseAngle(closeAngle);
    cover.setServoMinPulse(minPW);
    cover.setServoMaxPulse(maxPW);
    cover.setMoveTime(moveTime);
    cover.setRangeMin(rangeMin);
    cover.setRangeMax(rangeMax);

    #ifdef ENABLE_SAVING_TO_MEMORY
      storage.saveServoOpenAngle(openAngle);
      storage.saveServoCloseAngle(closeAngle);
      storage.saveServoMinPulse(minPW);
      storage.saveServoMaxPulse(maxPW);
      storage.saveMoveTime(moveTime);
      storage.saveServoRangeMin(rangeMin);
      storage.saveServoRangeMax(rangeMax);
    #endif

    Debug::info("WEBUI", "Servo settings saved");
  #endif

  _server.send(200, "application/json", "{\"ok\":true}");
}

void WebUIHandler::handleApiServoNudge() {
  #ifdef COVER_INSTALLED
    int16_t dir = _server.arg("dir").toInt();
    int16_t pos = cover.nudgeServo(dir);
    char buf[64];
    snprintf(buf, sizeof(buf), "{\"ok\":true,\"pos\":%d,\"open\":%d,\"close\":%d}",
             pos, cover.getServoOpenAngle(), cover.getServoCloseAngle());
    _server.send(200, "application/json", buf);
  #else
    _server.send(200, "application/json", "{\"ok\":false,\"error\":\"Cover not installed\"}");
  #endif
}

void WebUIHandler::handleApiServoSetOpen() {
  #ifdef COVER_INSTALLED
    int16_t angle = cover.setCurrentAsOpen();
    char buf[48];
    snprintf(buf, sizeof(buf), "{\"ok\":true,\"open\":%d}", angle);
    _server.send(200, "application/json", buf);
  #else
    _server.send(200, "application/json", "{\"ok\":false,\"error\":\"Cover not installed\"}");
  #endif
}

void WebUIHandler::handleApiServoSetClose() {
  #ifdef COVER_INSTALLED
    int16_t angle = cover.setCurrentAsClose();
    char buf[48];
    snprintf(buf, sizeof(buf), "{\"ok\":true,\"close\":%d}", angle);
    _server.send(200, "application/json", buf);
  #else
    _server.send(200, "application/json", "{\"ok\":false,\"error\":\"Cover not installed\"}");
  #endif
}

void WebUIHandler::handleApiSaveLight() {
  #ifdef LIGHT_INSTALLED
    uint16_t maxBright = _server.arg("maxbright").toInt();
    uint32_t stabTime = _server.arg("stabtime").toInt();

    light.setMaxBrightness(maxBright);
    light.setStabilizeTime(stabTime);

    #ifdef ENABLE_SAVING_TO_MEMORY
      storage.saveStabilizeTime(stabTime);
    #endif

    Debug::info("WEBUI", "Light settings saved");
  #endif

  _server.send(200, "application/json", "{\"ok\":true}");
}

void WebUIHandler::handleApiSaveHeater() {
  #ifdef HEATER_INSTALLED
    float delta = _server.arg("delta").toFloat();
    uint32_t shutoff = _server.arg("shutoff").toInt();

    heater.setDeltaPoint(delta);
    heater.setShutoffTime(shutoff);

    #ifdef ENABLE_SAVING_TO_MEMORY
      storage.saveDeltaPoint(delta);
      storage.saveShutoffTime(shutoff);
    #endif

    Debug::info("WEBUI", "Heater settings saved");
  #endif

  _server.send(200, "application/json", "{\"ok\":true}");
}

void WebUIHandler::handleApiRestart() {
  _server.send(200, "application/json", "{\"ok\":true}");
  delay(500);
  ESP.restart();
}
