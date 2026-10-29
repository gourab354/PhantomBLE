#ifndef BLE_SCANNER_H
#define BLE_SCANNER_H

#include <Arduino.h>
#include <NimBLEDevice.h>
#include <vector>

#define MAX_SCAN_RESULTS 25

struct ScannedDevice {
    String name;
    String address;
    int rssi;
    uint16_t appearance;
    String services;
};

// Forward declaration — defined in main.cpp
class BleScanner;
extern BleScanner bleScanner;

class BleScanner : public NimBLEAdvertisedDeviceCallbacks {
public:
    std::vector<ScannedDevice> devices;
    bool scanning = false;
    bool scanDone = false;

    void startScan(uint32_t seconds = 10) {
        if (scanning) return;
        devices.clear();
        scanDone = false;
        scanning = true;

        NimBLEScan* pScan = NimBLEDevice::getScan();
        pScan->setAdvertisedDeviceCallbacks(this, false);
        pScan->setActiveScan(true);
        pScan->setInterval(100);
        pScan->setWindow(99);
        pScan->start(seconds, onScanComplete, false);
        Serial.printf("[SCANNER] BLE scan started (%us)...\n", seconds);
    }

    void stopScan() {
        if (!scanning) return;
        NimBLEDevice::getScan()->stop();
        scanning = false;
        scanDone = true;
        Serial.println("[SCANNER] Scan stopped by user.");
    }

    // --- NimBLEAdvertisedDeviceCallbacks ---
    void onResult(NimBLEAdvertisedDevice* adv) override {
        if (devices.size() >= MAX_SCAN_RESULTS) return;

        String addr = String(adv->getAddress().toString().c_str());

        // Update existing device if already discovered (better info on rescan)
        for (auto& d : devices) {
            if (d.address == addr) {
                if (adv->haveName() && d.name.length() == 0)
                    d.name = String(adv->getName().c_str());
                d.rssi = adv->getRSSI();
                if (adv->haveAppearance())
                    d.appearance = adv->getAppearance();
                return;
            }
        }

        ScannedDevice dev;
        dev.name = adv->haveName() ? String(adv->getName().c_str()) : "";
        dev.address = addr;
        dev.rssi = adv->getRSSI();
        dev.appearance = adv->haveAppearance() ? adv->getAppearance() : 0;

        // Collect service UUIDs
        String svc = "";
        if (adv->haveServiceUUID()) {
            for (int i = 0; i < adv->getServiceUUIDCount(); i++) {
                if (i > 0) svc += ",";
                svc += String(adv->getServiceUUID(i).toString().c_str());
            }
        }
        dev.services = svc;
        devices.push_back(dev);

        Serial.printf("[SCANNER] [%d] %s  %s  RSSI:%d\n",
            (int)devices.size() - 1,
            dev.name.length() > 0 ? dev.name.c_str() : "(no name)",
            dev.address.c_str(), dev.rssi);
    }

    static void onScanComplete(NimBLEScanResults results) {
        bleScanner.scanning = false;
        bleScanner.scanDone = true;
        Serial.printf("[SCANNER] Scan complete. %d device(s) found.\n",
            (int)bleScanner.devices.size());
    }
};

#endif // BLE_SCANNER_H

