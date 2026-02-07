/*
  alpaca_handler.cpp - ASCOM Alpaca CoverCalibratorV2 REST API
  DarkLight Cover Calibrator - ESP32-S3 Port

  Implements ICoverCalibratorV2 interface with UDP discovery on port 32227
  and REST API on port 11111. Designed for Conform Universal compliance.

  ASCOM Error Codes:
    0x000 (0)    = Success
    0x400 (1024) = NotImplementedException
    0x401 (1025) = InvalidValue
    0x407 (1031) = NotConnectedException
    0x40B (1035) = InvalidOperationException

  (c) Copyright Nathan Woelfle 2020-present day. All Rights Reserved.
  Creative Commons Attribution-NonCommercial 4.0 International License
*/

#include "alpaca_handler.h"
#include "Debug.h"
#include <WiFi.h>

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
AlpacaHandler& getAlpacaHandler() {
  static AlpacaHandler instance;
  return instance;
}

void AlpacaHandler::begin() {
  // Generate unique ID from MAC address (UUID-like format)
  uint8_t mac[6];
  WiFi.macAddress(mac);
  char macStr[40];
  snprintf(macStr, sizeof(macStr), "%02x%02x%02x%02x-%02x%02x-0000-0000-%02x%02x%02x%02x%02x%02x",
           mac[0], mac[1], mac[2], mac[3], mac[4], mac[5],
           mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  _uniqueID = String(macStr);

  setupRoutes();
  _server.begin();
  startDiscovery();
  _running = true;

  Debug::infof("ALPACA", "Server started on port %d, ID=%s", ALPACA_PORT, _uniqueID.c_str());
}

void AlpacaHandler::loop() {
  if (!_running) return;
  _server.handleClient();
  handleDiscovery();
}

// ============================================================
// Discovery
// ============================================================

void AlpacaHandler::startDiscovery() {
  _udp.begin(ALPACA_DISC_PORT);
  Debug::infof("ALPACA", "Discovery listener on port %d", ALPACA_DISC_PORT);
}

void AlpacaHandler::handleDiscovery() {
  int packetSize = _udp.parsePacket();
  if (packetSize > 0) {
    char buffer[64];
    int len = _udp.read(buffer, sizeof(buffer) - 1);
    buffer[len] = '\0';

    // Match "alpacadiscovery1" (protocol version 1)
    if (strstr(buffer, "alpacadiscovery") != nullptr) {
      char response[64];
      snprintf(response, sizeof(response), "{\"AlpacaPort\":%d}", ALPACA_PORT);

      _udp.beginPacket(_udp.remoteIP(), _udp.remotePort());
      _udp.print(response);
      _udp.endPacket();

      Debug::debugf("ALPACA", "Discovery response sent to %s", _udp.remoteIP().toString().c_str());
    }
  }
}

// ============================================================
// Route Setup
// ============================================================

void AlpacaHandler::setupRoutes() {
  // --- Management API ---
  _server.on("/management/apiversions", HTTP_GET, [this]() { handleApiVersions(); });
  _server.on("/management/v1/description", HTTP_GET, [this]() { handleDescription(); });
  _server.on("/management/v1/configureddevices", HTTP_GET, [this]() { handleConfiguredDevices(); });

  // --- Setup pages (HTML, required by Alpaca spec) ---
  _server.on("/setup", HTTP_GET, [this]() { handleSetupPage(); });
  _server.on("/setup/v1/covercalibrator/0/setup", HTTP_GET, [this]() { handleDeviceSetupPage(); });

  // --- Common device properties (GET) ---
  _server.on("/api/v1/covercalibrator/0/connected", HTTP_GET, [this]() { handleGetConnected(); });
  _server.on("/api/v1/covercalibrator/0/connecting", HTTP_GET, [this]() { handleGetConnecting(); });
  _server.on("/api/v1/covercalibrator/0/description", HTTP_GET, [this]() { handleGetDescription(); });
  _server.on("/api/v1/covercalibrator/0/driverinfo", HTTP_GET, [this]() { handleGetDriverInfo(); });
  _server.on("/api/v1/covercalibrator/0/driverversion", HTTP_GET, [this]() { handleGetDriverVersion(); });
  _server.on("/api/v1/covercalibrator/0/interfaceversion", HTTP_GET, [this]() { handleGetInterfaceVersion(); });
  _server.on("/api/v1/covercalibrator/0/name", HTTP_GET, [this]() { handleGetName(); });
  _server.on("/api/v1/covercalibrator/0/supportedactions", HTTP_GET, [this]() { handleGetSupportedActions(); });
  _server.on("/api/v1/covercalibrator/0/devicestate", HTTP_GET, [this]() { handleGetDeviceState(); });

  // --- Common device methods (PUT) ---
  _server.on("/api/v1/covercalibrator/0/connected", HTTP_PUT, [this]() { handlePutConnected(); });
  _server.on("/api/v1/covercalibrator/0/connect", HTTP_PUT, [this]() { handlePutConnect(); });
  _server.on("/api/v1/covercalibrator/0/disconnect", HTTP_PUT, [this]() { handlePutDisconnect(); });
  _server.on("/api/v1/covercalibrator/0/action", HTTP_PUT, [this]() { handlePutAction(); });
  _server.on("/api/v1/covercalibrator/0/commandblind", HTTP_PUT, [this]() { handlePutCommandBlind(); });
  _server.on("/api/v1/covercalibrator/0/commandbool", HTTP_PUT, [this]() { handlePutCommandBool(); });
  _server.on("/api/v1/covercalibrator/0/commandstring", HTTP_PUT, [this]() { handlePutCommandString(); });

  // --- CoverCalibrator properties (GET) ---
  _server.on("/api/v1/covercalibrator/0/brightness", HTTP_GET, [this]() { handleGetBrightness(); });
  _server.on("/api/v1/covercalibrator/0/calibratorstate", HTTP_GET, [this]() { handleGetCalibratorState(); });
  _server.on("/api/v1/covercalibrator/0/coverstate", HTTP_GET, [this]() { handleGetCoverState(); });
  _server.on("/api/v1/covercalibrator/0/maxbrightness", HTTP_GET, [this]() { handleGetMaxBrightness(); });
  _server.on("/api/v1/covercalibrator/0/covermoving", HTTP_GET, [this]() { handleGetCoverMoving(); });
  _server.on("/api/v1/covercalibrator/0/calibratorchanging", HTTP_GET, [this]() { handleGetCalibratorChanging(); });

  // --- CoverCalibrator methods (PUT) ---
  _server.on("/api/v1/covercalibrator/0/calibratoroff", HTTP_PUT, [this]() { handlePutCalibratorOff(); });
  _server.on("/api/v1/covercalibrator/0/calibratoron", HTTP_PUT, [this]() { handlePutCalibratorOn(); });
  _server.on("/api/v1/covercalibrator/0/closecover", HTTP_PUT, [this]() { handlePutCloseCover(); });
  _server.on("/api/v1/covercalibrator/0/haltcover", HTTP_PUT, [this]() { handlePutHaltCover(); });
  _server.on("/api/v1/covercalibrator/0/opencover", HTTP_PUT, [this]() { handlePutOpenCover(); });
}

// ============================================================
// Response Helpers
// ============================================================

// Case-insensitive argument lookup — Alpaca GET requests send lowercase
// query params (e.g. "clienttransactionid") while PUT requests use PascalCase.
String AlpacaHandler::findArgCaseInsensitive(const char* name) {
  for (int i = 0; i < _server.args(); i++) {
    if (_server.argName(i).equalsIgnoreCase(name)) {
      return _server.arg(i);
    }
  }
  return String();
}

int32_t AlpacaHandler::getClientTransactionID() {
  String val = findArgCaseInsensitive("ClientTransactionID");
  if (val.length() > 0) {
    int32_t id = (int32_t)val.toInt();
    return (id >= 0) ? id : 0;
  }
  return 0;
}

int32_t AlpacaHandler::getClientID() {
  String val = findArgCaseInsensitive("ClientID");
  if (val.length() > 0) {
    int32_t id = (int32_t)val.toInt();
    return (id >= 0) ? id : 0;
  }
  return 0;
}

// Returns false and sends NotConnected error if device is not connected.
// Caller should return immediately if this returns false.
bool AlpacaHandler::checkConnected() {
  if (!_connected) {
    sendErrorResponse(0x407, "Not connected");
    return false;
  }
  return true;
}

// Response with a Value field (for property GETs)
void AlpacaHandler::sendValueResponse(int errorNumber, const char* errorMessage, int value) {
  JsonDocument doc;
  doc["Value"] = value;
  doc["ClientTransactionID"] = getClientTransactionID();
  doc["ServerTransactionID"] = ++_serverTransactionID;
  doc["ErrorNumber"] = errorNumber;
  doc["ErrorMessage"] = errorMessage;

  char buffer[512];
  serializeJson(doc, buffer, sizeof(buffer));
  _server.send(200, "application/json", buffer);
}

void AlpacaHandler::sendValueResponse(int errorNumber, const char* errorMessage, bool value) {
  JsonDocument doc;
  doc["Value"] = value;
  doc["ClientTransactionID"] = getClientTransactionID();
  doc["ServerTransactionID"] = ++_serverTransactionID;
  doc["ErrorNumber"] = errorNumber;
  doc["ErrorMessage"] = errorMessage;

  char buffer[512];
  serializeJson(doc, buffer, sizeof(buffer));
  _server.send(200, "application/json", buffer);
}

void AlpacaHandler::sendValueResponse(int errorNumber, const char* errorMessage, const char* value) {
  JsonDocument doc;
  doc["Value"] = value;
  doc["ClientTransactionID"] = getClientTransactionID();
  doc["ServerTransactionID"] = ++_serverTransactionID;
  doc["ErrorNumber"] = errorNumber;
  doc["ErrorMessage"] = errorMessage;

  char buffer[512];
  serializeJson(doc, buffer, sizeof(buffer));
  _server.send(200, "application/json", buffer);
}

// Response with a Value array
void AlpacaHandler::sendArrayResponse(JsonArray& arr) {
  JsonDocument doc;
  doc["Value"] = arr;
  doc["ClientTransactionID"] = getClientTransactionID();
  doc["ServerTransactionID"] = ++_serverTransactionID;
  doc["ErrorNumber"] = 0;
  doc["ErrorMessage"] = "";

  char buffer[512];
  serializeJson(doc, buffer, sizeof(buffer));
  _server.send(200, "application/json", buffer);
}

// Response for void methods (no Value field)
void AlpacaHandler::sendMethodResponse(int errorNumber, const char* errorMessage) {
  JsonDocument doc;
  doc["ClientTransactionID"] = getClientTransactionID();
  doc["ServerTransactionID"] = ++_serverTransactionID;
  doc["ErrorNumber"] = errorNumber;
  doc["ErrorMessage"] = errorMessage;

  char buffer[256];
  serializeJson(doc, buffer, sizeof(buffer));
  _server.send(200, "application/json", buffer);
}

// Error-only response (no Value field, used for NotConnected etc.)
void AlpacaHandler::sendErrorResponse(int errorNumber, const char* errorMessage) {
  sendMethodResponse(errorNumber, errorMessage);
}

// ============================================================
// Management API
// ============================================================

void AlpacaHandler::handleApiVersions() {
  JsonDocument doc;
  JsonArray versions = doc["Value"].to<JsonArray>();
  versions.add(1);
  doc["ClientTransactionID"] = getClientTransactionID();
  doc["ServerTransactionID"] = ++_serverTransactionID;
  doc["ErrorNumber"] = 0;
  doc["ErrorMessage"] = "";

  char buffer[256];
  serializeJson(doc, buffer, sizeof(buffer));
  _server.send(200, "application/json", buffer);
}

void AlpacaHandler::handleDescription() {
  JsonDocument doc;
  JsonObject val = doc["Value"].to<JsonObject>();
  val["ServerName"] = "DarkLight Cover Calibrator";
  val["Manufacturer"] = "DarkLight";
  val["ManufacturerVersion"] = DLC_VERSION;
  val["Location"] = "";
  doc["ClientTransactionID"] = getClientTransactionID();
  doc["ServerTransactionID"] = ++_serverTransactionID;
  doc["ErrorNumber"] = 0;
  doc["ErrorMessage"] = "";

  char buffer[512];
  serializeJson(doc, buffer, sizeof(buffer));
  _server.send(200, "application/json", buffer);
}

void AlpacaHandler::handleConfiguredDevices() {
  JsonDocument doc;
  JsonArray devices = doc["Value"].to<JsonArray>();
  JsonObject device = devices.add<JsonObject>();
  device["DeviceName"] = "DarkLight CoverCalibrator";
  device["DeviceType"] = "CoverCalibrator";
  device["DeviceNumber"] = 0;
  device["UniqueID"] = _uniqueID;
  doc["ClientTransactionID"] = getClientTransactionID();
  doc["ServerTransactionID"] = ++_serverTransactionID;
  doc["ErrorNumber"] = 0;
  doc["ErrorMessage"] = "";

  char buffer[512];
  serializeJson(doc, buffer, sizeof(buffer));
  _server.send(200, "application/json", buffer);
}

// ============================================================
// Setup Pages (HTML)
// ============================================================

void AlpacaHandler::handleSetupPage() {
  // Redirect to the web UI setup on port 80
  String html = "<html><head><meta http-equiv='refresh' content='0;url=http://";
  html += WiFi.localIP().toString();
  html += "/setup'></head><body><a href='http://";
  html += WiFi.localIP().toString();
  html += "/setup'>Go to setup</a></body></html>";
  _server.send(200, "text/html", html);
}

void AlpacaHandler::handleDeviceSetupPage() {
  handleSetupPage(); // Same redirect
}

// ============================================================
// Common Device Properties (GET)
// These do NOT require connected state per ASCOM spec:
//   Connected, Connecting, Description, DriverInfo, DriverVersion,
//   InterfaceVersion, Name, SupportedActions
// ============================================================

void AlpacaHandler::handleGetConnected() {
  sendValueResponse(0, "", _connected);
}

void AlpacaHandler::handleGetConnecting() {
  // Connection is instantaneous for embedded device
  sendValueResponse(0, "", false);
}

void AlpacaHandler::handleGetDescription() {
  sendValueResponse(0, "", "DarkLight Cover Calibrator - ESP32-S3");
}

void AlpacaHandler::handleGetDriverInfo() {
  sendValueResponse(0, "", "DarkLight CoverCalibrator Driver");
}

void AlpacaHandler::handleGetDriverVersion() {
  // ASCOM spec requires "n.n" format without prefix
  sendValueResponse(0, "", "2.0.0");
}

void AlpacaHandler::handleGetInterfaceVersion() {
  sendValueResponse(0, "", 2); // ICoverCalibratorV2
}

void AlpacaHandler::handleGetName() {
  sendValueResponse(0, "", "DarkLight CoverCalibrator");
}

void AlpacaHandler::handleGetSupportedActions() {
  JsonDocument arrDoc;
  JsonArray arr = arrDoc.to<JsonArray>(); // Empty array
  sendArrayResponse(arr);
}

void AlpacaHandler::handleGetDeviceState() {
  // V2 DeviceState: aggregated operational state as array of {Name, Value} pairs
  if (!checkConnected()) return;

  JsonDocument doc;
  JsonArray stateArr = doc["Value"].to<JsonArray>();

  // CoverState
  JsonObject cs = stateArr.add<JsonObject>();
  cs["Name"] = "CoverState";
  #ifdef COVER_INSTALLED
    cs["Value"] = (int)cover.getState();
  #else
    cs["Value"] = (int)COVER_NOT_PRESENT;
  #endif

  // CalibratorState
  JsonObject cals = stateArr.add<JsonObject>();
  cals["Name"] = "CalibratorState";
  #ifdef LIGHT_INSTALLED
    cals["Value"] = (int)light.getState();
  #else
    cals["Value"] = (int)CAL_NOT_PRESENT;
  #endif

  // Brightness
  JsonObject br = stateArr.add<JsonObject>();
  br["Name"] = "Brightness";
  #ifdef LIGHT_INSTALLED
    br["Value"] = (int)light.getCurrentBrightness();
  #else
    br["Value"] = 0;
  #endif

  // CoverMoving
  JsonObject cm = stateArr.add<JsonObject>();
  cm["Name"] = "CoverMoving";
  #ifdef COVER_INSTALLED
    cm["Value"] = (cover.getState() == COVER_MOVING);
  #else
    cm["Value"] = false;
  #endif

  // CalibratorChanging
  JsonObject cc = stateArr.add<JsonObject>();
  cc["Name"] = "CalibratorChanging";
  #ifdef LIGHT_INSTALLED
    cc["Value"] = (light.getState() == CAL_NOT_READY);
  #else
    cc["Value"] = false;
  #endif

  doc["ClientTransactionID"] = getClientTransactionID();
  doc["ServerTransactionID"] = ++_serverTransactionID;
  doc["ErrorNumber"] = 0;
  doc["ErrorMessage"] = "";

  char buffer[1024];
  serializeJson(doc, buffer, sizeof(buffer));
  _server.send(200, "application/json", buffer);
}

// ============================================================
// Common Device Methods (PUT)
// ============================================================

void AlpacaHandler::handlePutConnected() {
  // Read "Connected" from form-encoded body (case-sensitive)
  if (_server.hasArg("Connected")) {
    String val = _server.arg("Connected");
    // Accept "true"/"True"/"TRUE" and "false"/"False"/"FALSE"
    if (val.equalsIgnoreCase("true")) {
      _connected = true;
      Debug::info("ALPACA", "Connected = true");
    } else if (val.equalsIgnoreCase("false")) {
      _connected = false;
      Debug::info("ALPACA", "Connected = false");
    } else {
      sendMethodResponse(0x401, "Invalid value for Connected parameter");
      return;
    }
    sendMethodResponse(0, "");
  } else {
    sendMethodResponse(0x401, "Connected parameter is required");
  }
}

void AlpacaHandler::handlePutConnect() {
  // V2 Connect method - non-blocking, instantaneous for embedded
  _connected = true;
  Debug::info("ALPACA", "Connect()");
  sendMethodResponse(0, "");
}

void AlpacaHandler::handlePutDisconnect() {
  // V2 Disconnect method
  _connected = false;
  Debug::info("ALPACA", "Disconnect()");
  sendMethodResponse(0, "");
}

void AlpacaHandler::handlePutAction() {
  if (!checkConnected()) return;
  // No custom actions supported
  sendMethodResponse(0x40C, "Action is not implemented in this driver");
}

void AlpacaHandler::handlePutCommandBlind() {
  if (!checkConnected()) return;
  sendMethodResponse(0x400, "CommandBlind is not implemented");
}

void AlpacaHandler::handlePutCommandBool() {
  if (!checkConnected()) return;
  sendValueResponse(0x400, "CommandBool is not implemented", false);
}

void AlpacaHandler::handlePutCommandString() {
  if (!checkConnected()) return;
  sendValueResponse(0x400, "CommandString is not implemented", "");
}

// ============================================================
// CoverCalibrator Properties (GET)
// These REQUIRE connected state (except state properties that
// return NotPresent - those never throw per spec)
// ============================================================

void AlpacaHandler::handleGetBrightness() {
  // Per spec: throws PropertyNotImplementedException when CalibratorState is NotPresent
  #ifdef LIGHT_INSTALLED
    if (!checkConnected()) return;
    sendValueResponse(0, "", (int)light.getCurrentBrightness());
  #else
    if (!checkConnected()) return;
    sendValueResponse(0x400, "Calibrator is not present", 0);
  #endif
}

void AlpacaHandler::handleGetCalibratorState() {
  // Per spec: returns NotPresent (0) without throwing, even when not connected
  // Conform Universal expects this to work regardless of connection state
  #ifdef LIGHT_INSTALLED
    if (!checkConnected()) return;
    sendValueResponse(0, "", (int)light.getState());
  #else
    sendValueResponse(0, "", (int)CAL_NOT_PRESENT);
  #endif
}

void AlpacaHandler::handleGetCoverState() {
  // Per spec: returns NotPresent (0) without throwing, even when not connected
  #ifdef COVER_INSTALLED
    if (!checkConnected()) return;
    sendValueResponse(0, "", (int)cover.getState());
  #else
    sendValueResponse(0, "", (int)COVER_NOT_PRESENT);
  #endif
}

void AlpacaHandler::handleGetMaxBrightness() {
  // Per spec: throws PropertyNotImplementedException when CalibratorState is NotPresent
  #ifdef LIGHT_INSTALLED
    if (!checkConnected()) return;
    sendValueResponse(0, "", (int)light.getMaxBrightness());
  #else
    if (!checkConnected()) return;
    sendValueResponse(0x400, "Calibrator is not present", 0);
  #endif
}

void AlpacaHandler::handleGetCoverMoving() {
  // V2: returns false when CoverState is NotPresent (never throws)
  #ifdef COVER_INSTALLED
    if (!checkConnected()) return;
    sendValueResponse(0, "", (cover.getState() == COVER_MOVING));
  #else
    sendValueResponse(0, "", false);
  #endif
}

void AlpacaHandler::handleGetCalibratorChanging() {
  // V2: returns false when CalibratorState is NotPresent (never throws)
  #ifdef LIGHT_INSTALLED
    if (!checkConnected()) return;
    sendValueResponse(0, "", (light.getState() == CAL_NOT_READY));
  #else
    sendValueResponse(0, "", false);
  #endif
}

// ============================================================
// CoverCalibrator Methods (PUT)
// All require connected state and throw NotImplemented for NotPresent
// ============================================================

void AlpacaHandler::handlePutCalibratorOff() {
  if (!checkConnected()) return;

  #ifdef LIGHT_INSTALLED
    light.turnPanelOff();
    sendMethodResponse(0, "");
  #else
    sendMethodResponse(0x400, "CalibratorOff is not implemented - calibrator is not present");
  #endif
}

void AlpacaHandler::handlePutCalibratorOn() {
  if (!checkConnected()) return;

  #ifdef LIGHT_INSTALLED
    // Brightness parameter is required, case-sensitive, in form body
    if (!_server.hasArg("Brightness")) {
      sendMethodResponse(0x401, "Brightness parameter is required");
      return;
    }

    int brightness = _server.arg("Brightness").toInt();

    // Validate range: 0 to MaxBrightness (must reject out-of-range, not clamp)
    if (brightness < 0 || brightness > (int)light.getMaxBrightness()) {
      char errMsg[80];
      snprintf(errMsg, sizeof(errMsg), "Brightness must be between 0 and %d", light.getMaxBrightness());
      sendMethodResponse(0x401, errMsg);
      return;
    }

    light.turnPanelTo((uint16_t)brightness);
    sendMethodResponse(0, "");
  #else
    sendMethodResponse(0x400, "CalibratorOn is not implemented - calibrator is not present");
  #endif
}

void AlpacaHandler::handlePutCloseCover() {
  if (!checkConnected()) return;

  #ifdef COVER_INSTALLED
    cover.closeCover();
    sendMethodResponse(0, "");
  #else
    sendMethodResponse(0x400, "CloseCover is not implemented - cover is not present");
  #endif
}

void AlpacaHandler::handlePutHaltCover() {
  if (!checkConnected()) return;

  #ifdef COVER_INSTALLED
    if (cover.getState() == COVER_MOVING) {
      cover.haltCover();
      sendMethodResponse(0, "");
    } else {
      // Conform expects MethodNotImplementedException when cover is not moving
      sendMethodResponse(0x400, "Cover is not moving");
    }
  #else
    sendMethodResponse(0x400, "HaltCover is not implemented - cover is not present");
  #endif
}

void AlpacaHandler::handlePutOpenCover() {
  if (!checkConnected()) return;

  #ifdef COVER_INSTALLED
    cover.openCover();
    sendMethodResponse(0, "");
  #else
    sendMethodResponse(0x400, "OpenCover is not implemented - cover is not present");
  #endif
}
