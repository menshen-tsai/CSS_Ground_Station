# CSS Ground Station 
Hardware and Firmware/Software for Hand-On Class
---

## 📌 Overview

This repository contains the firmware, hardware configurations, and software components for the CSS Ground Station built using **STM32 microcontrollers** (STM32L432 series). It serves as a practical guide and codebase for hands-on satellite/ground station workshops.

---

## 📁 Repository Structure

├── Ground_Station_L432_SR105U_FLASH_20261008/       # Firmware with SR105U transceiver & Flash memory support<BR>
├── STM32L432_SENDBTN_ENCODER_FLASH_DECODE_20260929/ # Firmware for send button, rotary encoder, & flash decoding<BR>
└── README.md

---

## 🛠️ Hardware & Tools Required

- **MCU:** STM32L432 MCU board
- **Transceiver:** SR105U Module
- **Peripherals:** Rotary Encoder, Push Buttons, SPI Flash Memory
- **IDE/Toolchain:** STM32CubeIDE / Keil uVision
- **Programmer:** ST-LINK V2 / V3

---

## 🚀 Getting Started

### 1. Prerequisites
Ensure you have installed:
- [STM32CubeIDE](https://www.st.com/en/development-tools/stm32cubeide.html) (or your preferred C/C++ toolchain for STM32)
- ST-LINK Drivers

### 2. Cloning the Repository
```bash
git clone [https://github.com/menshen-tsai/CSS_Ground_Station.git](https://github.com/menshen-tsai/CSS_Ground_Station.git)
cd CSS_Ground_Station
```
### 3. Building & Flashing
1. Open STM32CubeIDE.
2. Go to File > Import... > Existing Projects into Workspace.
3. Browse to one of the project folders (e.g., Ground_Station_L432_SR105U_FLASH_20261008).
4. Build the project (Ctrl + B).
5. Connect your STM32 board via ST-LINK and click Run / Debug.

## 📜 Tech Stack
Primary Language: C (97%+)

Build System: Make / Linker Scripts

Target Platform: STM32L432 Series

## 👤 Author
menshen-tsai - GitHub Profile
