/*
  config.h - Pin definitions, compile-time feature flags, state enums, and constants
  DarkLight Cover Calibrator - ESP32-S3 Port

  (c) Copyright Nathan Woelfle 2020-present day. All Rights Reserved.
  Creative Commons Attribution-NonCommercial 4.0 International License
*/

#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

//-----------------------------------------------------------
//----- IF UNSURE HOW TO SETUP, SEE MANUAL FOR DETAILS  -----
//-----------------------------------------------------------

//----- (UA) USER-ADJUSTABLE OPTIONS ------
#define COVER_INSTALLED       // comment out if not utilized
//#define LIGHT_INSTALLED       // comment out if not utilized
// #define HEATER_INSTALLED      // comment out if not utilized
#define ENABLE_SERIAL_CONTROL // comment out if not utilized
#define ENABLE_MANUAL_CONTROL // comment out if not utilized
#define ENABLE_SAVING_TO_MEMORY // comment out if not utilized

//----- (UA) (COVER) -----
#define DEFAULT_TIME_TO_MOVE 5000   // (ms) time to move between open/close (1000-10000, recommend 5000)

//----- (UA) (COVER) SERVO PARAMETERS -----
#define DEFAULT_SERVO_MIN_PULSE 500   // usec min pulse width
#define DEFAULT_SERVO_MAX_PULSE 2500  // usec max pulse width
#define DEFAULT_SERVO_OPEN_ANGLE 85   // position angle servo opens to (within range limits)
#define DEFAULT_SERVO_CLOSE_ANGLE 195 // position angle servo closes to (within range limits)
#define DEFAULT_SERVO_MAX_ANGLE 270   // max degrees of servo travel (180 for standard, 270 for 270-degree)
#define DEFAULT_SERVO_RANGE_MIN 0     // usable range minimum (0 to SERVO_MAX_ANGLE)
#define DEFAULT_SERVO_RANGE_MAX 270   // usable range maximum (0 to SERVO_MAX_ANGLE)

//----- (UA) (COVER) SELECT A MOVEMENT -----
//----- UNCOMMENT ONLY ONE OPTION -----
#define USE_LINEAR
//#define USE_CIRCULAR
//#define USE_CUBIC
//#define USE_EXPO
//#define USE_QUAD
//#define USE_QUART
//#define USE_QUINT
//#define USE_SINE

//----- (UA) (LIGHT) -----
#define DEFAULT_MAX_BRIGHTNESS 255  // max brightness value (1, 5, 17, 51, 85, 255, or 1023 for 10-bit)
#define DEFAULT_STABILIZE_TIME 0    // (ms) delay for light to settle after change
#define LIGHT_PWM_BITS 10           // PWM resolution bits (10 = 0-1023 range)
#define LIGHT_PWM_MAX  1023         // max PWM output value (2^LIGHT_PWM_BITS - 1)

//----- (UA) (HEATER) -----
#define DEFAULT_HEATER_SHUTOFF 3600000  // (ms) max time for manual heating (1 hour)
#define DEFAULT_DELTA_POINT 5.0f        // (degrees) target temp above dew point

//----- (UA) (HEATER) TEMPERATURE & HUMIDITY SENSOR -----
//----- UNCOMMENT ONLY ONE OPTION -----
#define ENABLE_BME280
//#define ENABLE_DHT22

//----- (UA) (BUTTON) -----
#define DEBOUNCE_DELAY 150      // (ms) debounce time

//----- END OF (UA) USER-ADJUSTABLE OPTIONS -----
//-----------------------------------------------
//-------------- DO NOT EDIT BELOW --------------
//-----------------------------------------------

//----- VERSIONING -----
#define DLC_VERSION "v2.1.0"

//----- VALIDATION: easing options -----
#ifdef COVER_INSTALLED
  #if (defined(USE_LINEAR) + defined(USE_CIRCULAR) + defined(USE_CUBIC) + \
      defined(USE_EXPO) + defined(USE_QUAD) + defined(USE_QUART) + \
      defined(USE_QUINT) + defined(USE_SINE)) > 1
    #error "Multiple easing options defined. Please uncomment only one."
  #elif !(defined(USE_LINEAR) || defined(USE_CIRCULAR) || defined(USE_CUBIC) || \
        defined(USE_EXPO) || defined(USE_QUAD) || defined(USE_QUART) || \
        defined(USE_QUINT) || defined(USE_SINE))
    #define USE_LINEAR
    #pragma message("Warning: No easing option defined. Defaulting to USE_LINEAR.")
  #endif
#endif

//----- VALIDATION: temp sensor -----
#ifdef HEATER_INSTALLED
  #if (defined(ENABLE_BME280) + defined(ENABLE_DHT22)) > 1
    #error "Multiple temp sensor options defined. Please uncomment only one."
  #elif !(defined(ENABLE_BME280) + defined(ENABLE_DHT22))
    #error "No temp sensor option defined."
  #endif
#endif

//----- ESP32-S3 PIN ASSIGNMENTS -----
const uint8_t PIN_BUTTON     = 46;  // Single IO button (INPUT_PULLUP)
const uint8_t PIN_SERVO      = 10;  // Servo PWM
const uint8_t PIN_HEATER     = 11;  // Heater PWM
const uint8_t PIN_LIGHT      = 12;  // Light panel PWM
const uint8_t PIN_DS18B20    = 13;  // DS18B20 OneWire
const uint8_t PIN_RELAY_K1   = 4;   // K1 relay (light panel power gate)
const uint8_t PIN_I2C_SDA    = 8;   // I2C SDA (BME280)
const uint8_t PIN_I2C_SCL    = 9;   // I2C SCL (BME280)
const uint8_t PIN_DHT        = 13;  // DHT22 (shares pin with DS18B20, only one active)

//----- STATE ENUMS -----
enum CoverState : uint8_t {
  COVER_NOT_PRESENT = 0,
  COVER_CLOSED      = 1,
  COVER_MOVING      = 2,
  COVER_OPEN        = 3,
  COVER_UNKNOWN     = 4,
  COVER_ERROR       = 5
};

enum CalibratorState : uint8_t {
  CAL_NOT_PRESENT = 0,
  CAL_OFF         = 1,
  CAL_NOT_READY   = 2,
  CAL_READY       = 3,
  CAL_UNKNOWN     = 4,
  CAL_ERROR       = 5
};

enum HeaterState : uint8_t {
  HEATER_NOT_PRESENT = 0,
  HEATER_OFF         = 1,
  HEATER_AUTO        = 2,
  HEATER_ON          = 3,
  HEATER_UNKNOWN     = 4,
  HEATER_ERROR       = 5,
  HEATER_SET         = 6   // Heat-on-close armed
};

//----- SERIAL PROTOCOL -----
const uint32_t SERIAL_SPEED         = 115200;
const char     SERIAL_START_MARKER  = '<';
const char     SERIAL_END_MARKER    = '>';
const uint8_t  MAX_RECV_CHARS       = 10;
const uint8_t  MAX_SEND_CHARS       = 75;

//----- HEATER CONSTANTS -----
const float DEW_POINT_ALPHA       = 17.27f;   // August-Roche-Magnus constant
const float DEW_POINT_BETA        = 237.7f;   // August-Roche-Magnus constant
const float PWM_MAP_MULTIPLIER    = 100.0f;   // PWM mapping scale factor
const float PWM_MAP_RANGE         = 500.0f;   // PWM mapping range value
const float MAX_HEATER_PWM        = 255.0f;   // Max PWM value for heater
const uint8_t MAX_ERROR_COUNT     = 5;        // Consecutive error threshold

#ifdef ENABLE_BME280
  const uint32_t DEW_INTERVAL = 1000;  // 1 second for BME280
#endif
#ifdef ENABLE_DHT22
  const uint32_t DEW_INTERVAL = 2000;  // 2 seconds for DHT22
#endif

//----- COVER CONSTANTS -----
const uint32_t SERVO_DETACH_TIME = 3000;  // ms after stop before detaching servo

//----- WIFI DEFAULTS -----
const uint16_t ALPACA_PORT       = 11111;
const uint16_t WEB_PORT          = 80;
const uint16_t ALPACA_DISC_PORT  = 32227;
const char*    const AP_SSID     = "DLC-Setup";
const char*    const AP_PASS     = "darklight";
const char*    const MDNS_HOST   = "darklightcc";
const uint32_t WIFI_TIMEOUT      = 15000;  // ms to wait for STA connection

//----- NVS PREFERENCE KEYS -----
// Original firmware values
const char* const KEY_COVER_STATE   = "coverState";
const char* const KEY_PANEL_VALUE   = "panelValue";
const char* const KEY_BROADBAND     = "broadband";
const char* const KEY_NARROWBAND    = "narrowband";

// Servo configuration
const char* const KEY_SERVO_OPEN    = "servoOpen";
const char* const KEY_SERVO_CLOSE   = "servoClose";
const char* const KEY_SERVO_MIN_PW  = "servoMinPW";
const char* const KEY_SERVO_MAX_PW  = "servoMaxPW";
const char* const KEY_MOVE_TIME     = "moveTime";
const char* const KEY_SERVO_RANGE_MIN = "servoRngMin";
const char* const KEY_SERVO_RANGE_MAX = "servoRngMax";

// Light configuration
const char* const KEY_MAX_BRIGHT    = "maxBright";
const char* const KEY_STAB_TIME     = "stabTime";

// Heater configuration
const char* const KEY_HEATER_MODE   = "heaterMode";
const char* const KEY_DELTA_POINT   = "deltaPoint";
const char* const KEY_SHUTOFF_TIME  = "shutoffTime";

// WiFi configuration
const char* const KEY_WIFI_SSID     = "wifiSSID";
const char* const KEY_WIFI_PASS     = "wifiPass";

#endif // CONFIG_H
