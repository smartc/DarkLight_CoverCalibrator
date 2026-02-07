/*
  Name:         DarkLight Cover Calibrator (DLC) - ESP32-S3 Port
  Author:       Nathan Woelfle
  Contributors: Taylor J (Initial Dew Heater Integration)
  Date:         2/7/26

  Version: 2.0.0
  *View GitHub Wiki for version change details
  https://github.com/10thTeeAstronomy/DarkLight_CoverCalibrator/wiki/Firmware-Version-History

  Description: ESP32-S3 port of the DarkLight Cover Calibrator firmware.
               Adds WiFi connectivity, ASCOM Alpaca REST API, and Web UI
               alongside the existing USB serial interface.

               Target: ESP32-S3 Dev Module (USB CDC On Boot: Enabled)

  https://github.com/10thTeeAstronomy/DarkLight_CoverCalibrator

  (c) Copyright Nathan Woelfle 2020-present day. All Rights Reserved.

  This project is protected by International Copyright Law.
  Permission is granted for personal and Academic/Educational use only.
  Software and hardware distributed under Creative Commons Attribution-NonCommercial License

  For more information, please refer to the full terms of the Creative Commons Attribution-NonCommercial 4.0 International License:
  https://creativecommons.org/licenses/by-nc/4.0/
*/

#include "config.h"
#include "Debug.h"
#include "storage_manager.h"

#ifdef COVER_INSTALLED
  #include "cover_controller.h"
#endif

#ifdef LIGHT_INSTALLED
  #include "light_controller.h"
#endif

#ifdef HEATER_INSTALLED
  #include "heater_controller.h"
#endif

#ifdef ENABLE_SERIAL_CONTROL
  #include "serial_handler.h"
#endif

#ifdef ENABLE_MANUAL_CONTROL
  #include "button_handler.h"
#endif

#include <WiFi.h>
#include <ESPmDNS.h>
#include "alpaca_handler.h"
#include "web_ui_handler.h"

// WiFi state
enum DLCWiFiMode : uint8_t { DLC_WIFI_STA, DLC_WIFI_AP };
DLCWiFiMode wifiCurrentMode = DLC_WIFI_STA;
uint32_t wifiConnectStartTime = 0;
bool wifiConnected = false;
uint32_t lastWifiCheck = 0;
const uint32_t WIFI_CHECK_INTERVAL = 10000; // Check WiFi status every 10 seconds

// Forward declarations
void initializeWiFi();
void handleWiFi();
void startAPMode();
void onCoverOpenStart();
void onCoverCloseComplete();

void setup() {
  // Initialize debug logging
  Debug::begin(DBG_INFO);

  // Wait for USB CDC
  delay(1000);

  Debug::info("MAIN", "========================================");
  Debug::infof("MAIN", "DarkLight Cover Calibrator %s", DLC_VERSION);
  Debug::info("MAIN", "ESP32-S3 Port");
  Debug::info("MAIN", "========================================");

  // Initialize storage (NVS)
  #ifdef ENABLE_SAVING_TO_MEMORY
    storage.begin();
  #endif

  // Initialize serial handler
  #ifdef ENABLE_SERIAL_CONTROL
    serialHandler.begin();
  #endif

  // Initialize cover controller
  #ifdef COVER_INSTALLED
    cover.setOnOpenStart(onCoverOpenStart);
    cover.setOnCloseComplete(onCoverCloseComplete);
    cover.begin();
  #endif

  // Initialize light controller
  #ifdef LIGHT_INSTALLED
    light.begin();
  #endif

  // Initialize heater controller
  #ifdef HEATER_INSTALLED
    heater.begin();
  #endif

  // Initialize button handler
  #ifdef ENABLE_MANUAL_CONTROL
    button.begin();
  #endif

  // Initialize WiFi
  initializeWiFi();

  Debug::info("MAIN", "Setup complete");
}

void loop() {
  // Handle serial commands
  #ifdef ENABLE_SERIAL_CONTROL
    serialHandler.loop();
  #endif

  // Handle button input
  #ifdef ENABLE_MANUAL_CONTROL
    button.loop();
  #endif

  // Process cover movement
  #ifdef COVER_INSTALLED
    cover.loop();
  #endif

  // Process light stabilization
  #ifdef LIGHT_INSTALLED
    light.loop();
  #endif

  // Process heater control
  #ifdef HEATER_INSTALLED
    #ifdef COVER_INSTALLED
      heater.loop(cover.getState() == COVER_MOVING);
    #else
      heater.loop(false);
    #endif
  #endif

  // Handle WiFi
  handleWiFi();

  // Handle Alpaca server
  if (wifiConnected) {
    getAlpacaHandler().loop();
    getWebUIHandler().loop();
  }
}

// --- WiFi Management ---

void initializeWiFi() {
  String ssid = storage.loadWifiSSID();
  String pass = storage.loadWifiPass();

  if (ssid.length() == 0) {
    Debug::info("WIFI", "No saved credentials, starting AP mode");
    startAPMode();
    return;
  }

  Debug::infof("WIFI", "Connecting to: %s", ssid.c_str());
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid.c_str(), pass.c_str());
  wifiCurrentMode = DLC_WIFI_STA;
  wifiConnectStartTime = millis();
}

void handleWiFi() {
  uint32_t now = millis();

  // Rate-limit WiFi checks
  if (now - lastWifiCheck < WIFI_CHECK_INTERVAL && wifiConnected) return;
  lastWifiCheck = now;

  if (wifiCurrentMode == DLC_WIFI_STA) {
    if (WiFi.status() == WL_CONNECTED) {
      if (!wifiConnected) {
        wifiConnected = true;
        Debug::infof("WIFI", "Connected! IP: %s", WiFi.localIP().toString().c_str());

        // Start mDNS
        if (MDNS.begin(MDNS_HOST)) {
          Debug::infof("WIFI", "mDNS: %s.local", MDNS_HOST);
        }

        // Start Alpaca and Web servers
        getAlpacaHandler().begin();
        getWebUIHandler().begin();
      }
    } else {
      if (wifiConnected) {
        // Lost connection
        wifiConnected = false;
        Debug::warning("WIFI", "Connection lost, reconnecting...");
      }

      // Check timeout for initial connection
      if (!wifiConnected && (now - wifiConnectStartTime >= WIFI_TIMEOUT)) {
        Debug::warning("WIFI", "Connection timeout, starting AP mode");
        startAPMode();
      }
    }
  }
  // In AP mode, servers are always running
}

void startAPMode() {
  WiFi.mode(WIFI_AP);
  WiFi.softAP(AP_SSID, AP_PASS);
  wifiCurrentMode = DLC_WIFI_AP;
  wifiConnected = true; // AP is "connected" for server purposes

  Debug::infof("WIFI", "AP Mode: SSID=%s, IP=%s", AP_SSID, WiFi.softAPIP().toString().c_str());

  // Start mDNS
  if (MDNS.begin(MDNS_HOST)) {
    Debug::infof("WIFI", "mDNS: %s.local", MDNS_HOST);
  }

  // Start Alpaca and Web servers
  getAlpacaHandler().begin();
  getWebUIHandler().begin();
}

// --- Cross-module callbacks ---

void onCoverOpenStart() {
  // Turn off light when cover opens
  #ifdef LIGHT_INSTALLED
    if (light.getState() != CAL_NOT_PRESENT && light.getState() != CAL_OFF) {
      light.turnPanelOff();
    }
  #endif

  // Turn off heater if manually heating
  #ifdef HEATER_INSTALLED
    if (heater.getState() == HEATER_ON) {
      heater.setManualHeat(false);
    }
  #endif
}

void onCoverCloseComplete() {
  // Restore light if autoON
  #ifdef LIGHT_INSTALLED
    light.restorePreviousLight();
  #endif

  // Trigger heat-on-close
  #ifdef HEATER_INSTALLED
    heater.triggerHeatOnClose();
  #endif
}
