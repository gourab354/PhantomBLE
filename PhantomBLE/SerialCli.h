#ifndef SERIAL_CLI_H
#define SERIAL_CLI_H

#include <Arduino.h>
#include "BleManager.h"
#include "PayloadRunner.h"
#include "WifiManager.h"
#include "BleScanner.h"

class SerialCli {
public:
    void begin(BleManager* ble, PayloadRunner* runner, WifiManager* wifi, BleScanner* scanner = nullptr) {
        pBle = ble;
        pRunner = runner;
        pWifi = wifi;
        pScanner = scanner;
        printBanner();
    }

    void handleSerial() {
        while (Serial.available()) {
            char c = Serial.read();
            if (c == '\r') continue;
            if (c == '\n') {
                processLine(inputBuffer);
                inputBuffer = "";
            } else {
                inputBuffer += c;
            }
        }
    }

private:
    BleManager* pBle = nullptr;
    PayloadRunner* pRunner = nullptr;
    WifiManager* pWifi = nullptr;
    BleScanner* pScanner = nullptr;
    String inputBuffer;
    bool inCustomScriptMode = false;
    String storedCustomScript = "";

    void printBanner() {
        Serial.println();
        Serial.println("==================================================");
        Serial.println("   ____  __                __                     ");
        Serial.println("  / __ \\/ /_  ____ _____  / /_____  ____ ___      ");
        Serial.println(" / /_/ / __ \\/ __ `/ __ \\/ __/ __ \\/ __ `__ \\     ");
        Serial.println("/ ____/ / / / /_/ / / / / /_/ /_/ / / / / / /     ");
        Serial.println("/_/   /_/ /_/\\__,_/_/ /_/\\__/\\____/_/ /_/ /_/ BLE ");
        Serial.println("==================================================");
        Serial.println(" Target Hardware: ESP32 DevKit (38-pin Classic)   ");
        Serial.println(" Firmware: v1.2.0 | Baud: 115200                  ");
        Serial.println(" SoftAP IP: 192.168.4.1                           ");
        Serial.println(" Type 'help' or 'menu' for available commands.    ");
        Serial.println("==================================================");
        Serial.println();
    }

    void printMenu() {
        Serial.println("\r\n+-------------------------------------------------------------+");
        Serial.println("| PhantomBLE v1.2.0 / ESP32 38-Pin CLI Menu                   |");
        Serial.println("+-------------------------------------------------------------+");
        Serial.println("  help / menu         Display this interactive command menu");
        Serial.println("  1 / start           Start BLE discovery broadcast");
        Serial.println("  0 / stop            Stop advertising / disconnect active session");
        Serial.println("  2 / status          Display real-time connection status");
        Serial.println("  3 / profiles        List available BLE appearance profiles");
        Serial.println("  name <text>         Set custom BLE name (up to 25 characters)");
        Serial.println("  name tv             Reset identity to Android TV");
        Serial.println("  appearance <hex>    Set raw appearance (e.g. appearance 0x03C1)");
        Serial.println("  duration <sec>      Set discovery timeout (5 - 60 seconds)");
        Serial.println("  mode swift          Enable Microsoft Swift Pair beacon mode");
        Serial.println("  mode beacon         Enable standard BLE beacon mode");
        Serial.println("  wifi                Display current Wi-Fi SSID & Password");
        Serial.println("  wifi set <s> <p>    Update SoftAP SSID and Password (min 8 chars)");
        Serial.println("  4 / hid             List HID payload menu");
        Serial.println("  5 / hello           Execute Notepad + Hello preset");
        Serial.println("  payload begin       Enter interactive custom script mode (finish with END)");
        Serial.println("  payload run         Execute the stored custom payload");
        Serial.println("  abort               Immediately cancel active payload and release keys");
        Serial.println("  forget              Clear stored Bluetooth pairing bonds");
        Serial.println("+-------------------------------------------------------------+");
        Serial.println("| Scanner Commands                                            |");
        Serial.println("+-------------------------------------------------------------+");
        Serial.println("  6 / scan            Scan for nearby BLE devices (10s)");
        Serial.println("  scan <seconds>      Scan with custom duration (3-30s)");
        Serial.println("  7 / devices         List discovered BLE devices");
        Serial.println("  clone <n>           Clone identity of scanned device #n");
        Serial.println("+-------------------------------------------------------------+\r\n");
    }

    void processLine(String line) {
        line.trim();
        if (line.length() == 0) return;

        if (inCustomScriptMode) {
            if (line.equalsIgnoreCase("END")) {
                inCustomScriptMode = false;
                Serial.println("[CLI] Custom script recorded (" + String(storedCustomScript.length()) + " bytes). Type 'payload run' to execute.");
            } else {
                storedCustomScript += line + "\n";
                Serial.println("  > " + line);
            }
            return;
        }

        if (line.equalsIgnoreCase("help") || line.equalsIgnoreCase("menu")) {
            printMenu();
        } else if (line == "1" || line.equalsIgnoreCase("start")) {
            if (pBle) pBle->startAdvertising();
        } else if (line == "0" || line.equalsIgnoreCase("stop")) {
            if (pBle) pBle->stopAdvertising();
        } else if (line == "2" || line.equalsIgnoreCase("status")) {
            if (pBle) {
                Serial.printf("[STATUS] Phase: %s | HID: %s | Name: %s | App: 0x%04X | Swift: %s | Remaining: %us | Sessions: %u\r\n",
                    pBle->phase.c_str(),
                    pBle->hidReady ? "READY" : "NO",
                    pBle->deviceName.c_str(),
                    pBle->appearance,
                    pBle->swiftMode ? "YES" : "NO",
                    pBle->remaining,
                    pBle->sessions
                );
            }
        } else if (line == "3" || line.equalsIgnoreCase("profiles")) {
            Serial.println("\r\nPreset Profiles:");
            Serial.println("  Android TV:   0x0140 (320)");
            Serial.println("  Apple TV:     0x0280 (640)");
            Serial.println("  Keyboard:     0x03C1 (961)");
            Serial.println("  Mouse:        0x03C2 (962)");
            Serial.println("  Gamepad:      0x03C4 (964)");
            Serial.println("  Joystick:     0x03C3 (963)");
            Serial.println("  Digital Pen:  0x03C7 (967)");
            Serial.println("  AirTag:       0x0200 (512)");
            Serial.println("  Heart Rate:   0x0340 (832)\r\n");
        } else if (line.startsWith("name ")) {
            String newName = line.substring(5);
            newName.trim();
            if (newName.equalsIgnoreCase("tv")) newName = "Android TV";
            if (newName.length() > 25) newName = newName.substring(0, 25);
            if (pBle) pBle->updateConfig(newName, pBle->appearance, pBle->duration, pBle->swiftMode);
        } else if (line.startsWith("appearance ")) {
            String appStr = line.substring(11);
            appStr.trim();
            uint16_t app = strtol(appStr.c_str(), NULL, 16);
            if (pBle) pBle->updateConfig(pBle->deviceName, app, pBle->duration, pBle->swiftMode);
        } else if (line.startsWith("duration ")) {
            uint32_t dur = line.substring(9).toInt();
            if (dur < 5) dur = 5;
            if (dur > 300) dur = 300;
            if (pBle) pBle->updateConfig(pBle->deviceName, pBle->appearance, dur, pBle->swiftMode);
        } else if (line.equalsIgnoreCase("mode swift")) {
            if (pBle) pBle->updateConfig(pBle->deviceName, pBle->appearance, pBle->duration, true);
        } else if (line.equalsIgnoreCase("mode beacon")) {
            if (pBle) pBle->updateConfig(pBle->deviceName, pBle->appearance, pBle->duration, false);
        } else if (line.equalsIgnoreCase("wifi")) {
            if (pWifi) {
                Serial.printf("[WIFI] SSID: %s | Pass: %s | IP: 192.168.4.1\r\n", pWifi->ssid.c_str(), pWifi->password.length() ? pWifi->password.c_str() : "<open>");
            }
        } else if (line.startsWith("wifi set ")) {
            int spaceIdx = line.indexOf(' ', 9);
            if (spaceIdx == -1) {
                Serial.println("[CLI] Usage: wifi set <ssid> <password (min 8 chars)>");
            } else {
                String s = line.substring(9, spaceIdx);
                String p = line.substring(spaceIdx + 1);
                s.trim();
                p.trim();
                if (pWifi && pWifi->updateCredentials(s, p)) {
                    Serial.println("[WIFI] SoftAP updated successfully.");
                } else {
                    Serial.println("[CLI] Error updating credentials.");
                }
            }
        } else if (line == "4" || line.equalsIgnoreCase("hid")) {
            Serial.println("HID Payloads:");
            Serial.println("  5 / hello       Open Notepad + Hello From PhantomBLE");
            Serial.println("  payload begin   Enter custom script, finish with END");
            Serial.println("  payload run     Run stored custom payload");
            Serial.println("  abort           Abort payload");
        } else if (line == "5" || line.equalsIgnoreCase("hello")) {
            if (pRunner) pRunner->runPresetNotepad();
        } else if (line.equalsIgnoreCase("payload begin")) {
            inCustomScriptMode = true;
            storedCustomScript = "";
            Serial.println("[CLI] Entering script editor. Enter commands (e.g. STRING, DELAY, ENTER). Type 'END' to save.");
        } else if (line.equalsIgnoreCase("payload run")) {
            if (storedCustomScript.length() == 0) {
                Serial.println("[CLI] No custom script stored. Use 'payload begin' first.");
            } else if (pRunner) {
                pRunner->runScript(storedCustomScript);
            }
        } else if (line.equalsIgnoreCase("abort")) {
            if (pRunner) pRunner->abort();
        } else if (line.equalsIgnoreCase("forget")) {
            if (pBle) pBle->forgetBonds();

        // --- Scanner commands ---
        } else if (line == "6" || line.equalsIgnoreCase("scan")) {
            if (pScanner) {
                if (pBle && pBle->phase != "idle") pBle->stopAdvertising();
                pScanner->startScan(10);
            } else {
                Serial.println("[CLI] Scanner not available.");
            }
        } else if (line.startsWith("scan ")) {
            uint32_t dur = line.substring(5).toInt();
            if (dur < 3) dur = 3;
            if (dur > 30) dur = 30;
            if (pScanner) {
                if (pBle && pBle->phase != "idle") pBle->stopAdvertising();
                pScanner->startScan(dur);
            }
        } else if (line == "7" || line.equalsIgnoreCase("devices")) {
            if (pScanner && pScanner->devices.size() > 0) {
                Serial.printf("\r\nDiscovered BLE Devices (%d):\r\n", (int)pScanner->devices.size());
                for (int i = 0; i < (int)pScanner->devices.size(); i++) {
                    const auto& d = pScanner->devices[i];
                    Serial.printf("  [%d] %-20s %s  RSSI:%d  App:0x%04X  %s\r\n",
                        i,
                        d.name.length() > 0 ? d.name.c_str() : "(unnamed)",
                        d.address.c_str(), d.rssi, d.appearance,
                        d.services.length() > 0 ? d.services.c_str() : "");
                }
                Serial.println();
            } else {
                Serial.println("[CLI] No devices found. Run 'scan' first.");
            }
        } else if (line.startsWith("clone ")) {
            int idx = line.substring(6).toInt();
            if (pScanner && idx >= 0 && idx < (int)pScanner->devices.size()) {
                if (pScanner->scanning) pScanner->stopScan();
                const auto& d = pScanner->devices[idx];
                String name = d.name.length() > 0 ? d.name : "BLE Device";
                uint16_t app = d.appearance > 0 ? d.appearance : 961;
                if (pBle) {
                    pBle->updateConfig(name, app, pBle->duration, pBle->swiftMode);
                    pBle->startAdvertising();
                    Serial.printf("[CLI] Cloned: '%s' (0x%04X). Now advertising.\r\n", name.c_str(), app);
                }
            } else {
                Serial.println("[CLI] Invalid index. Run 'devices' to see available devices.");
            }
        } else {
            Serial.println("[CLI] Unknown command: '" + line + "'. Type 'help' for command list.");
        }
    }
};

#endif // SERIAL_CLI_H
