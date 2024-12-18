# Cybertyper Project: Hardware Overview

This document provides an overview of the hardware components used in the Cybertyper project, along with links to detailed product pages where available.

## 1. **Base Keyboard**
- **Old Cherry G80-3000 Keyboard**
  - A classic mechanical keyboard repurposed for the project.
  - The USB Module which converts the Keystrokes to a USB connection will be removed and the Keyboard PCB will be directly connected to the GPIO-Expanders and the Matrix Logic will be handled by the ESP32.

## 2. **Microcontroller**
- **Adafruit Qualia ESP32-S3**
  - Handles display rendering and keyboard input.
  - [Adafruit Product Page](https://www.adafruit.com/product/5785)

## 3. **Display**
- **Adafruit 4.58" Rectangle Bar RGB TTL TFT Display**
  - Resolution: 320*960 / RGB-666 (TTL)
  - [Adafruit Product Page](https://www.adafruit.com/product/5805)

## 4. **Power Supply**
- **DFRobot 2-Way 18650 Battery Holder**
  - Provides 3.3V and 5V outputs from a 18650 Li-Io battery.
  - [DFRobot Product Page](https://www.dfrobot.com/product-1866.html)

## 5. **Keyboard Interface**
- **MCP23017 GPIO Expanders (x2)**
  - Used to manage the keyboard's matrix input.
  - [Adafruit Product Page](https://www.adafruit.com/product/732)

## 6. **Removable Storage**
- **Standard SD Card Breakout Module**
  - Used for file storage and easy data transfer.
  - [Adafruit Product Page](https://www.adafruit.com/product/254)

## 7. **Other Connectors**
- **STEMMA QT Connectors**
  - Simplify wiring between the MCP23017 GPIO expanders and the main microcontroller.

---

This list will be updated as the project evolves. For further details or to contribute, please refer to the project's repository.