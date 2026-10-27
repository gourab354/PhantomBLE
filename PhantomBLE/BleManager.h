#ifndef BLE_MANAGER_H
#define BLE_MANAGER_H

#include <Arduino.h>
#include <NimBLEDevice.h>
#include <NimBLEHIDDevice.h>
#include "HidKeycodes.h"

// Forward declaration
class BleManager;
extern BleManager bleManager;

// Event callback function type for logging
typedef void (*LogCallback)(const String& msg);

class BleManager : public NimBLEServerCallbacks, public NimBLECharacteristicCallbacks {
public:
    String deviceName = "Android TV";
    uint16_t appearance = 0x0140; // 320 Android TV
    bool swiftMode = true;
    uint32_t duration = 30; // seconds
    uint32_t remaining = 0;
    uint32_t sessions = 0;
    
    String phase = "idle"; // "idle", "advertising", "cooldown", "connected"
    bool hidReady = false;
    LogCallback logger = nullptr;

    BleManager() {}

    void setLogger(LogCallback cb) {
        logger = cb;
    }

    void log(const String& msg) {
        if (logger) logger(msg);
        Serial.println("[BLE] " + msg);
    }

    void begin() {
        NimBLEDevice::init(deviceName.c_str());
        NimBLEDevice::setSecurityAuth(true, false, false); // Just Works bonding
        NimBLEDevice::setSecurityIOCap(BLE_HS_IO_NO_INPUT_OUTPUT);

        pServer = NimBLEDevice::createServer();
        pServer->setCallbacks(this);

        pHid = new NimBLEHIDDevice(pServer);
        pInputReport = pHid->inputReport(1);
        pInputReport->setCallbacks(this);

        // Standard Keyboard Report Descriptor
        static const uint8_t hidReportDesc[] = {
            0x05, 0x01,  // USAGE_PAGE (Generic Desktop)
            0x09, 0x06,  // USAGE (Keyboard)
            0xa1, 0x01,  // COLLECTION (Application)
            0x85, 0x01,  //   REPORT_ID (1)
            0x05, 0x07,  //   USAGE_PAGE (Keyboard)
            0x19, 0xe0,  //   USAGE_MINIMUM (LeftControl)
            0x29, 0xe7,  //   USAGE_MAXIMUM (Right GUI)
            0x15, 0x00,  //   LOGICAL_MINIMUM (0)
            0x25, 0x01,  //   LOGICAL_MAXIMUM (1)
            0x75, 0x01,  //   REPORT_SIZE (1)
            0x95, 0x08,  //   REPORT_COUNT (8)
            0x81, 0x02,  //   INPUT (Data,Var,Abs) -- Modifiers
            0x95, 0x01,  //   REPORT_COUNT (1)
            0x75, 0x08,  //   REPORT_SIZE (8)
            0x81, 0x03,  //   INPUT (Cnst,Var,Abs) -- Reserved
            0x95, 0x05,  //   REPORT_COUNT (5)
            0x75, 0x01,  //   REPORT_SIZE (1)
            0x05, 0x08,  //   USAGE_PAGE (LEDs)
            0x19, 0x01,  //   USAGE_MINIMUM (Num Lock)
            0x29, 0x05,  //   USAGE_MAXIMUM (Kana)
            0x91, 0x02,  //   OUTPUT (Data,Var,Abs)
            0x95, 0x01,  //   REPORT_COUNT (1)
            0x75, 0x03,  //   REPORT_SIZE (3)
            0x91, 0x03,  //   OUTPUT (Cnst,Var,Abs)
            0x95, 0x06,  //   REPORT_COUNT (6)
            0x75, 0x08,  //   REPORT_SIZE (8)
            0x15, 0x00,  //   LOGICAL_MINIMUM (0)
            0x25, 0x65,  //   LOGICAL_MAXIMUM (101)
            0x05, 0x07,  //   USAGE_PAGE (Keyboard)
            0x19, 0x00,  //   USAGE_MINIMUM (0)
            0x29, 0x65,  //   USAGE_MAXIMUM (101)
            0x81, 0x00,  //   INPUT (Data,Ary,Abs) -- Keys
            0xc0         // END_COLLECTION
        };

        pHid->manufacturer()->setValue("PhantomBLE");
        pHid->pnp(0x02, 0x045E, 0x0001, 0x0110);
        pHid->hidInfo(0x00, 0x01);
        pHid->reportMap((uint8_t*)hidReportDesc, sizeof(hidReportDesc));
        pHid->startServices();

        NimBLEDevice::setPower(ESP_PWR_LVL_P9); // Maximum +9dBm TX power for strong signal
        log("NimBLE HID initialized with identity: " + deviceName);
        phase = "idle";
    }

    void updateAdvertising() {
        pAdvertising = NimBLEDevice::getAdvertising();
        pAdvertising->stop();

        // Set fast advertising interval recommended for Swift Pair (20ms - 40ms)
        pAdvertising->setMinInterval(32); // 20ms (32 * 0.625ms)
        pAdvertising->setMaxInterval(64); // 40ms (64 * 0.625ms)

        NimBLEAdvertisementData advData;
        advData.setFlags(0x06); // General Discoverable + BR/EDR not supported
        advData.setName(deviceName.c_str());
        advData.setAppearance(appearance);

        if (swiftMode) {
            // Microsoft Swift Pair Beacon Data:
            // 0x06, 0x00 (Vendor ID: Microsoft)
            // 0x03 (Beacon Subtype: Swift Pair)
            // 0x00 (Scenario: Swift Pair over BLE)
            // 0x80 (Tx Power / Proximity flag)
            uint8_t msftPayload[] = {0x06, 0x00, 0x03, 0x00, 0x80};
            std::string mfgData((char*)msftPayload, sizeof(msftPayload));
            advData.setManufacturerData(mfgData);
        }

        pAdvertising->setAdvertisementData(advData);

        NimBLEAdvertisementData scanData;
        scanData.setName(deviceName.c_str());
        scanData.setCompleteServices(NimBLEUUID((uint16_t)0x1812)); // HID Service
        pAdvertising->setScanResponseData(scanData);
        pAdvertising->setScanResponse(true);
    }

    void startAdvertising() {
        if (phase == "connected") return;
        updateAdvertising();
        pAdvertising->start();
        phase = "advertising";
        advStartTime = millis();
        remaining = duration;
        log("BLE advertising started for '" + deviceName + "' (Mode: " + String(swiftMode ? "Swift Pair" : "Beacon") + ", " + String(duration) + "s)");
    }

    void stopAdvertising() {
        if (pAdvertising) {
            pAdvertising->stop();
        }
        if (phase == "connected" && pServer) {
            pServer->disconnect(connectedConnId);
        }
        phase = "idle";
        remaining = 0;
        hidReady = false;
        log("BLE stopped / idle");
    }

    void updateConfig(const String& name, uint16_t app, uint32_t dur, bool swift) {
        deviceName = name;
        appearance = app;
        duration = dur;
        swiftMode = swift;
        NimBLEDevice::setDeviceName(deviceName.c_str());

        bool wasAdv = (phase == "advertising");
        if (wasAdv && pAdvertising) {
            pAdvertising->stop();
        }
        updateAdvertising();
        if (wasAdv && pAdvertising) {
            pAdvertising->start();
            advStartTime = millis();
        }
        log("BLE identity updated: " + deviceName + " (Appearance 0x" + String(appearance, HEX) + ", Swift: " + String(swift ? "yes" : "no") + ")");
    }

    void forgetBonds() {
        NimBLEDevice::deleteAllBonds();
        log("All Bluetooth bonds cleared.");
    }

    void tick() {
        if (phase == "advertising") {
            uint32_t elapsed = (millis() - advStartTime) / 1000;
            if (elapsed >= duration) {
                phase = "cooldown";
                cooldownStartTime = millis();
                remaining = 15;
                log("Discovery window closed. Entered cooldown mode.");
            } else {
                remaining = duration - elapsed;
            }
        } else if (phase == "cooldown") {
            uint32_t elapsed = (millis() - cooldownStartTime) / 1000;
            if (elapsed >= 15) {
                stopAdvertising();
            } else {
                remaining = 15 - elapsed;
            }
        }
    }

    // HID Send Key Methods
    void sendReport(uint8_t modifier, uint8_t keycode) {
        if (!hidReady || !pInputReport) return;
        uint8_t report[8] = {modifier, 0, keycode, 0, 0, 0, 0, 0};
        pInputReport->setValue(report, sizeof(report));
        pInputReport->notify();
    }

    void releaseKeys() {
        if (!hidReady || !pInputReport) return;
        uint8_t report[8] = {0, 0, 0, 0, 0, 0, 0, 0};
        pInputReport->setValue(report, sizeof(report));
        pInputReport->notify();
    }

    void tapKey(uint8_t modifier, uint8_t keycode, uint32_t pressDelayMs = 25) {
        sendReport(modifier, keycode);
        delay(pressDelayMs);
        releaseKeys();
        delay(pressDelayMs);
    }

    void sendChar(char c) {
        AsciiKey ak = getAsciiKey(c);
        if (ak.keycode != 0) {
            tapKey(ak.modifier, ak.keycode, 15);
        }
    }

    void sendString(const String& text, uint32_t charDelayMs = 15) {
        for (size_t i = 0; i < text.length(); i++) {
            sendChar(text[i]);
            if (charDelayMs > 0) delay(charDelayMs);
        }
    }

    // NimBLEServerCallbacks
    void onConnect(NimBLEServer* pServer, ble_gap_conn_desc* desc) override {
        connectedConnId = desc->conn_handle;
        phase = "connected";
        remaining = 0;
        sessions++;
        log("PC connected over BLE! Conn handle: " + String(desc->conn_handle));
    }

    void onDisconnect(NimBLEServer* pServer) override {
        phase = "idle";
        hidReady = false;
        remaining = 0;
        log("PC disconnected from BLE.");
    }

    // NimBLECharacteristicCallbacks
    void onSubscribe(NimBLECharacteristic* pCharacteristic, ble_gap_conn_desc* desc, uint16_t subValue) override {
        if (subValue != 0) {
            hidReady = true;
            log("HID notification ready! Keyboard paired and active.");
        } else {
            hidReady = false;
        }
    }

private:
    NimBLEServer* pServer = nullptr;
    NimBLEHIDDevice* pHid = nullptr;
    NimBLECharacteristic* pInputReport = nullptr;
    NimBLEAdvertising* pAdvertising = nullptr;
    uint16_t connectedConnId = 0;
    uint32_t advStartTime = 0;
    uint32_t cooldownStartTime = 0;
};

#endif // BLE_MANAGER_H
