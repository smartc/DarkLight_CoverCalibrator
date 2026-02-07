# DarkLight Cover/Calibrator (DLC)

The **DarkLight Cover/Calibrator (DLC)** is a DIY project to build a motorized telescope cover, flat panel, or a combined flip-flat system. Designed for versatility and modularity, the DLC supports a variety of setups:

- **Light Panel Only** – Wall-mounted calibration panel (no servo)  
- **Servo(s) Cover Only** – Flip cover for sky flats and darks  
- **Flip-Flat Combo** – Integrated servo and light panel system

The DLC is compatible with both **ASCOM** and **INDI** platforms, enabling seamless automation through:

- The **DLC Windows app** (ASCOM only)
- ASCOM automation software (via the **ICoverCalibrator** driver)
- **ASCOM Alpaca** network discovery and control (ESP32-S3 only)
- The **INDI** platform for Linux/macOS users

Optional **dew heating** reduces the need for additional equipment and cabling.

> ⚠️ While the DLC **does NOT** make the telescope weatherproof or dustproof, it provides hands-off control and dark/light calibration for astrophotography in the field or an observatory.

---

## 📦 Features

- Build as a **standalone cover**, **light panel**, or **combination flip-flat** mechanism
- **Optional physical button** for manual servo and light control
- 7 **servo easing options** (ease-in/out and linear)
- **Adjustable servo speed**
- **Optional dew heater** with auto, manual, and heat-on-close modes
- **ASCOM**, **ASCOM Alpaca**, and **INDI** driver support
- Up to **270-degree movement** range with configurable min/max limits
- **Reverse voltage protection** on 12VDC input
- Save **brightness and position** to persistent storage
- **10-bit PWM** brightness control (0-1023) with configurable max brightness
- **Smooth illumination circuit** (eliminates PWM banding)
- Save **broadband and narrowband calibration values**

---

## 🖥️ Supported Hardware

### Arduino Nano (AVR) — Original
The original firmware in `dlc_firmware/` runs on Arduino Nano and communicates via USB serial. It supports dual servos, dual heater channels, and all serial protocol commands.

### ESP32-S3 — New
The `dlc_firmware_s3/` folder contains a full port to ESP32-S3, adding WiFi connectivity, a web dashboard, and native ASCOM Alpaca support while maintaining full backward compatibility with the USB serial protocol and existing INDI/ASCOM drivers.

**ESP32-S3 additions:**
- **WiFi** with STA mode and AP fallback ("DLC-Setup")
- **ASCOM Alpaca CoverCalibratorV2** REST API with UDP discovery (Conform Universal compliant)
- **Web dashboard** with live status, device controls, and dark theme
- **Web setup page** with servo positioning (nudge +/-1 degree), WiFi, servo, light, and heater configuration
- **OTA firmware updates** via ElegantOTA (`/update`)
- **K1 relay power-gating** for light panel
- **mDNS** discovery (`darklightcc.local`)
- **NVS Preferences** replacing EEPROM for wear-leveled persistent storage

**ESP32-S3 pin assignments:**

| Pin | Function |
|-----|----------|
| IO46 | Button (INPUT_PULLUP) |
| IO10 | Servo PWM |
| IO11 | Heater PWM |
| IO12 | Light panel PWM |
| IO13 | DS18B20 OneWire |
| IO4 | K1 relay (light panel power gate) |
| IO8 | I2C SDA (BME280) |
| IO9 | I2C SCL (BME280) |

---

## 🛠️ Building

### Arduino Nano firmware
```bash
arduino-cli compile --fqbn arduino:avr:nano --libraries dlc_firmware/DLC_Library dlc_firmware
```

### ESP32-S3 firmware
```bash
arduino-cli compile --fqbn esp32:esp32:esp32s3 dlc_firmware_s3
```
Required libraries: ESP32Servo, ArduinoJson, OneWire, DallasTemperature, Adafruit BME280, Adafruit Unified Sensor, ElegantOTA. WiFi, WebServer, ESPmDNS, Preferences, and Wire are included in the ESP32 Arduino Core.

---

> 📢 **Personal and academic use only.**
> Commercial use is **prohibited** without written permission.
> See full license and legal terms below.

---

## ⚠️ Security Notice

Only download DLC software from the official GitHub repository:  
[https://github.com/iwannabswiss/DarkLight_CoverCalibrator](https://github.com/iwannabswiss/DarkLight_CoverCalibrator)

Do **not** trust `.exe`, `.msi`, `.zip`, or `.tar.gz` files from unofficial sources, which may contain malware or unsupported changes.

---

## 📜 License

© 2020–2025 10th Tee Astronomy. All rights reserved.

This project is licensed under the  
[Creative Commons Attribution-NonCommercial 4.0 International License](https://creativecommons.org/licenses/by-nc/4.0/)

- You may **share and adapt** the materials for **personal or academic use**
- **Commercial use is prohibited** without written permission
- Modified versions must credit the original work
- See `LICENSE.md` for full terms

---

## 💼 Commercial Use

If you are interested in using this project for commercial purposes,  
please contact the author for licensing options:

📧 **Contact**: 10thTeeAstronomy@gmail.com

---

## 🧾 Legal Notice

This project is protected by international copyright law and treaties.  
All content is provided **"as is"** with no warranty or guarantee of suitability.

---

Happy imaging! 🌌