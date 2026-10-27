# PhantomBLE
**Advanced BLE HID Emulation & Swift Pair Injection for ESP32 DevKit (38-Pin)**

[![Version](https://img.shields.io/badge/version-1.2.0-blue.svg)](https://github.com)
[![Hardware](https://img.shields.io/badge/Target-ESP32--DevKit%20(38--pin)-green.svg)](https://www.espressif.com)
[![Firmware](https://img.shields.io/badge/Release-Pre--built%20Binaries-brightgreen.svg)](firmware/esp32_devkit_38pin/)
[![Architecture](https://img.shields.io/badge/Arch-Xtensa%20Dual--Core%2032--bit-red.svg)](https://github.com)
[![License](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)

---

## Overview

**PhantomBLE** is a complete, full-featured wireless BLE emulation and HID keystroke injection platform engineered specifically for the standard **ESP32 DevKit 38-Pin (ESP32-WROOM-32 / ESP32-D0WDQ6)** board.

PhantomBLE enables your classic ESP32 microcontroller to broadcast multi-device BLE identities—including Android TV, Apple TV, Wireless Keyboards, Mice, Gamepads, AirTags, JioTags, and Heart Rate Sensors—while simultaneously hosting a standalone Wi-Fi SoftAP access point (`192.168.4.1`) with a modern, responsive web dashboard.

When paired over Bluetooth (with automatic Microsoft Swift Pair popup support on Windows 10/11), PhantomBLE operates as an encrypted BLE Human Interface Device (HID) capable of executing programmable keystroke payloads and DuckyScript macros.

---

## Key Features

- **ESP32 DevKit 38-Pin Ready:** Built and optimized specifically for classic ESP32 boards (Xtensa dual-core 240MHz, 4MB Flash).
- **Multi-Device BLE Disguise:** Emulate 10+ preset Bluetooth identities with matching appearances:
  - Android TV (`0x0140` / 320) & Apple TV (`0x0280` / 640)
  - Wireless Keyboard (`0x03C1` / 961)
  - Mouse (`0x03C2` / 962)
  - Gamepad (`0x03C4` / 964) & Joystick (`0x03C3` / 963)
  - Digital Pen / Digitizer (`0x03C7` / 967)
  - AirTag & JioTag (`0x0200` / 512)
  - Heart Rate Sensor (`0x0340` / 832)
  - Custom Identity (Custom device name up to 25 chars & raw appearance hex)
- **Microsoft Swift Pair:** Broadcasts Windows Swift Pair beacons (Vendor ID `0x0006`, Sub-scenario `0x03`) to trigger instant pairing toasts on Windows 10 & 11 PCs.
- **Normal Beacon Mode:** Connectable BLE advertising without Microsoft vendor tags.
- **Zero-Install Web Dashboard:** Built-in Wi-Fi SoftAP (`192.168.4.1`) serves an ultra-sleek, responsive mobile/desktop management dashboard (served with Gzip compression directly from Flash).
- **Built-in Keystroke Payloads:**
  - **Notepad + Hello:** Opens Windows Run (`Win + R`), launches Notepad, and automatically types `Hello From PhantomBLE`.
  - **Custom PowerShell:** Types and executes any PowerShell commands entered into the web console.
  - **Custom Script Runner:** Full DuckyScript interpreter (up to 512 bytes): supports `STRING`, `DELAY`, `ENTER`, `TAB`, `ESC`, `BACKSPACE`, `SPACE`, `LEFT`, `RIGHT`, `UP`, `DOWN`, `GUI <key>`, `CTRL <key>`, `ALT <key>`, `CTRL SHIFT ENTER`, `CTRL SHIFT ESC`, and `REM`.
- **Serial Console Control (CLI):** Full interactive command-line interface at **115200 baud** over USB.
- **Non-Volatile Storage (NVS):** SoftAP SSID, password, and custom device configurations persist across reboots.

---

## Hardware Compatibility

| Hardware | Support Status | Notes |
|---|---|---|
| **ESP32 DevKit 38-Pin** (ESP32-WROOM-32) | **Fully Supported (Primary Target)** | Xtensa Dual-Core 240MHz, 4MB Flash, BLE 4.2 |
| **ESP32 DevKit 30-Pin** / NodeMCU-32S | **Fully Supported** | Identical ESP32 core chip |
| **ESP32-C5** | Legacy Reference | Pre-compiled binaries available in `firmware/uart/` |

---

## Repository Structure

```text
├── firmware/
│   └── esp32_devkit_38pin/        # Pre-built binaries for ESP32 DevKit 38-pin
│       ├── bootloader.bin          # Bootloader (offset: 0x1000)
│       ├── partitions.bin          # Partition table (offset: 0x8000)
│       └── firmware.bin            # Main application (offset: 0x10000)
├── src/                            # Complete C++ source code
│   ├── BleManager.h                # NimBLE HID & Swift Pair advertising engine
│   ├── HidKeycodes.h               # USB HID keycodes & ASCII mapper
│   ├── PayloadRunner.h             # DuckyScript interpreter & payload engine
│   ├── WifiManager.h               # SoftAP & NVS preferences manager
│   ├── WebManager.h                # HTTP WebServer & JSON REST API
│   ├── SerialCli.h                 # Interactive Serial CLI (115200 baud)
│   ├── webpage.h                   # Gzipped dashboard UI stored in PROGMEM
│   └── main.cpp                    # Application entry point
├── PhantomBLE/                     # Arduino IDE ready project folder
│   └── PhantomBLE.ino              # Arduino sketch file
├── platformio.ini                  # PlatformIO project configuration
├── flash.bat                       # One-click Windows batch flasher
├── flash.ps1                       # One-click PowerShell flasher
└── README.md
```

---

## Flashing Instructions for ESP32 DevKit 38-Pin

### Method 1: One-Click Flashing (Recommended)

1. Connect your **ESP32 DevKit 38-pin** to your PC using a micro-USB cable.
2. Verify your COM port in **Device Manager** (e.g. `COM5`).
3. Run the automated script:
   - Double-click `flash.bat` (defaults to `COM5`), or open terminal:
     ```cmd
     flash.bat COM5
     ```
   - Or in PowerShell:
     ```powershell
     .\flash.ps1 -Port COM5
     ```

> [!TIP]
> If `esptool` reports: `Failed to connect to Espressif device: Wrong boot mode detected (0x13)!`:
> 1. Press and **HOLD** down the **BOOT** button on your ESP32 board.
> 2. Press and release the **EN / RST** button once.
> 3. Release the **BOOT** button.
> 4. Run `flash.bat` again.

---

### Method 2: Manual `esptool` Command

If you prefer flashing manually via `python -m esptool`:

```bash
python -m esptool --chip esp32 -p COM5 -b 460800 --before default_reset --after hard_reset write_flash -z --flash_mode dio --flash_freq 40m --flash_size 4MB 0x1000 firmware/esp32_devkit_38pin/bootloader.bin 0x8000 firmware/esp32_devkit_38pin/partitions.bin 0x10000 firmware/esp32_devkit_38pin/firmware.bin
```

---

### Method 3: Compile & Flash with PlatformIO

To compile from source and flash directly:

```bash
python -m platformio run --target upload --upload-port COM5
```

To monitor serial output:
```bash
python -m platformio device monitor -b 115200
```

---

### Method 4: Compile & Flash with Arduino IDE

1. Open **Arduino IDE**.
2. Open the file `PhantomBLE/PhantomBLE.ino`.
3. In **Tools -> Board**, select **ESP32 Dev Module**.
4. In **Tools -> Partition Scheme**, select **Huge APP (3MB No OTA/1MB SPIFFS)**.
5. In **Tools -> Manage Libraries...**, install:
   - `NimBLE-Arduino` (by h2zero)
   - `ArduinoJson` (by Benoît Blanchon)
6. Select your COM port and click **Upload**.

---

## Getting Started & Usage

1. **Power Up:** Once flashed, press the **EN / RST** button to reboot the ESP32.
2. **Connect to Wi-Fi SoftAP:**
   - On your phone or laptop, search for Wi-Fi networks.
   - Connect to **`PhantomBLE-XXXX`** (where `XXXX` corresponds to the last 4 digits of your ESP32 MAC address).
   - By default, the network is open (no password). You can set an 8+ character password inside Settings.
3. **Open the Web UI:**
   - In any browser, navigate to:
     ```text
     http://192.168.4.1
     ```
4. **Trigger BLE Discovery:**
   - On the **Connect** tab, tap **Start discovery**.
   - On Windows 10/11 machines, a **Swift Pair** notification will pop up: *"New Android TV found. Select to connect."*
   - Click to pair. Once connected, the dashboard shows **HID ready · encrypted keyboard active**.
5. **Run Payloads:**
   - Switch to the **Payloads** tab.
   - Select **Notepad + Hello**, **Custom PS**, or upload a custom DuckyScript file.
   - Tap **Run** to execute the keystroke sequence!

---

## Serial Console Commands (115200 Baud)

You can also operate PhantomBLE through any serial monitor (PuTTY, Arduino Serial Monitor, etc.) at **115200 baud**:

```text
  help / menu         Display this interactive command menu
  1 / start           Start BLE discovery broadcast
  0 / stop            Stop advertising / disconnect active session
  2 / status          Display real-time connection status
  3 / profiles        List available BLE appearance profiles
  name <text>         Set custom BLE name (up to 25 characters)
  name tv             Reset identity to Android TV
  appearance <hex>    Set raw appearance (e.g. appearance 0x03C1)
  duration <sec>      Set discovery timeout (5 - 60 seconds)
  mode swift          Enable Microsoft Swift Pair beacon mode
  mode beacon         Enable standard BLE beacon mode
  wifi                Display current Wi-Fi SSID & Password
  wifi set <s> <p>    Update SoftAP SSID and Password (min 8 chars)
  4 / hid             List HID payload menu
  5 / hello           Execute Notepad + Hello preset
  payload begin       Enter interactive custom script mode (finish with END)
  payload run         Execute the stored custom payload
  abort               Immediately cancel active payload and release keys
  forget              Clear stored Bluetooth pairing bonds
```

---

## Payload Script Syntax (DuckyScript Style)

Custom scripts support standard ASCII keystroke commands (up to 512 bytes):

```text
STRING Hello World           # Types literal text
DELAY 500                    # Delay in milliseconds
ENTER                        # Press and release Enter
TAB / ESC / BACKSPACE        # Standard control keys
LEFT / RIGHT / UP / DOWN     # Cursor navigation keys
GUI r                        # Windows Key + R (Run dialog)
CTRL SHIFT ENTER             # Run as Administrator
REM This is a comment        # Ignored by runner
```

---

## Disclaimer

> [!WARNING]
> **FOR EDUCATIONAL, RESEARCH, AND AUTHORIZED SECURITY TESTING PURPOSES ONLY.**
> 
> PhantomBLE is designed to demonstrate Bluetooth Low Energy discovery mechanisms, Microsoft Swift Pair protocol behaviors, and BLE HID security architectures.
> 
> Never deploy this tool on systems without explicit, written authorization from the system owner.
