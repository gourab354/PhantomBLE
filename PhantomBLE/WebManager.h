#ifndef WEB_MANAGER_H
#define WEB_MANAGER_H

#include <Arduino.h>
#include <WebServer.h>
#include <ArduinoJson.h>
#include <vector>
#include "webpage.h"
#include "scanner_page.h"
#include "BleManager.h"
#include "PayloadRunner.h"
#include "WifiManager.h"
#include "BleScanner.h"

class WebManager {
public:
    WebServer server{80};
    std::vector<String> logs;

    void addLog(const String& msg) {
        if (logs.size() >= 40) {
            logs.erase(logs.begin());
        }
        logs.push_back(msg);
    }

    void begin(BleManager* ble, PayloadRunner* runner, WifiManager* wifi, BleScanner* scanner = nullptr) {
        pBle = ble;
        pRunner = runner;
        pWifi = wifi;
        pScanner = scanner;

        addLog("Boot complete. PhantomBLE firmware ready.");
        addLog("ESP32 DevKit 38-pin target initialized.");

        // --- Original routes (unchanged) ---
        server.on("/", HTTP_GET, [this]() {
            server.sendHeader("Content-Encoding", "gzip");
            server.send_P(200, "text/html", (const char*)WEBPAGE_GZ, WEBPAGE_GZ_LEN);
        });

        server.on("/api/status", HTTP_GET, [this]() {
            handleStatus();
        });

        server.on("/api/control", HTTP_POST, [this]() {
            handleControl();
        });

        // --- Scanner routes (new) ---
        server.on("/scanner", HTTP_GET, [this]() {
            server.send(200, "text/html", SCANNER_PAGE);
        });

        server.on("/api/scan", HTTP_POST, [this]() {
            handleScanControl();
        });

        server.on("/api/scan/results", HTTP_GET, [this]() {
            handleScanResults();
        });

        server.on("/api/scan/clone", HTTP_POST, [this]() {
            handleScanClone();
        });

        server.onNotFound([this]() {
            server.sendHeader("Location", "/");
            server.send(302, "text/plain", "");
        });

        server.begin();
        Serial.println("[WEB] HTTP Server listening on port 80");
    }

    void handleClient() {
        server.handleClient();
    }

private:
    BleManager* pBle = nullptr;
    PayloadRunner* pRunner = nullptr;
    WifiManager* pWifi = nullptr;
    BleScanner* pScanner = nullptr;

    // --- Original handlers (unchanged) ---
    void handleStatus() {
        JsonDocument doc;
        doc["ready"] = true;
        doc["phase"] = pBle ? pBle->phase : "idle";
        doc["hidReady"] = pBle ? pBle->hidReady : false;
        doc["name"] = pBle ? pBle->deviceName : "Android TV";
        doc["appearance"] = pBle ? pBle->appearance : 320;
        doc["swift"] = pBle ? pBle->swiftMode : true;
        doc["duration"] = pBle ? pBle->duration : 30;
        doc["remaining"] = pBle ? pBle->remaining : 0;
        doc["sessions"] = pBle ? pBle->sessions : 0;

        doc["payloadActive"] = pRunner ? pRunner->payloadActive : false;
        doc["payloadPhase"] = pRunner ? pRunner->payloadPhase : "idle";
        doc["payloadProgress"] = pRunner ? pRunner->payloadProgress : 0;
        doc["payloadTotal"] = pRunner ? pRunner->payloadTotal : 0;

        doc["ssid"] = pWifi ? pWifi->ssid : "";
        doc["password"] = pWifi ? pWifi->password : "";
        doc["version"] = "1.2.0";

        JsonArray logsArr = doc["logs"].to<JsonArray>();
        for (const auto& logMsg : logs) {
            logsArr.add(logMsg);
        }

        JsonArray profiles = doc["profiles"].to<JsonArray>();
        const struct { const char* label; int value; } presetList[] = {
            {"Android TV", 320},
            {"Apple TV", 640},
            {"Keyboard", 961},
            {"Mouse", 962},
            {"Gamepad", 964},
            {"Joystick", 963},
            {"Digital Pen", 967},
            {"AirTag", 512},
            {"JioTag", 512},
            {"Heart Rate", 832}
        };
        for (const auto& p : presetList) {
            JsonObject obj = profiles.add<JsonObject>();
            obj["label"] = p.label;
            obj["value"] = p.value;
        }

        String jsonResponse;
        serializeJson(doc, jsonResponse);
        server.send(200, "application/json", jsonResponse);
    }

    void handleControl() {
        if (!server.hasArg("plain")) {
            server.send(400, "text/plain", "Missing body");
            return;
        }

        JsonDocument doc;
        DeserializationError err = deserializeJson(doc, server.arg("plain"));
        if (err) {
            server.send(400, "text/plain", "Invalid JSON");
            return;
        }

        const char* action = doc["action"];
        if (!action) {
            server.send(400, "text/plain", "Missing action");
            return;
        }

        if (strcmp(action, "start") == 0) {
            if (pBle) pBle->startAdvertising();
            server.send(200, "text/plain", "Discovery started");
        } else if (strcmp(action, "stop") == 0) {
            if (pBle) pBle->stopAdvertising();
            server.send(200, "text/plain", "Discovery stopped");
        } else if (strcmp(action, "configure") == 0) {
            const char* name = doc["name"] | "Android TV";
            uint16_t appearance = doc["appearance"] | 320;
            uint32_t duration = doc["duration"] | 30;
            bool swift = doc["swift"] | true;
            if (pBle) pBle->updateConfig(name, appearance, duration, swift);
            server.send(200, "text/plain", "Config updated");
        } else if (strcmp(action, "wifi_config") == 0) {
            const char* ssid = doc["ssid"] | "";
            const char* pass = doc["password"] | "";
            if (pWifi && pWifi->updateCredentials(ssid, pass)) {
                addLog("Wi-Fi credentials updated: " + String(ssid));
                server.send(200, "text/plain", "Wi-Fi updated");
            } else {
                server.send(400, "text/plain", "Invalid SSID or password");
            }
        } else if (strcmp(action, "run") == 0) {
            const char* preset = doc["preset"] | "notepad";
            if (strcmp(preset, "notepad") == 0) {
                if (pRunner) pRunner->runPresetNotepad();
                server.send(200, "text/plain", "Running Notepad + Hello");
            } else {
                const char* script = doc["script"] | "";
                if (strlen(script) > 0 && pRunner) {
                    pRunner->runScript(script);
                    server.send(200, "text/plain", "Running custom script");
                } else {
                    server.send(400, "text/plain", "Script empty");
                }
            }
        } else if (strcmp(action, "abort") == 0) {
            if (pRunner) pRunner->abort();
            server.send(200, "text/plain", "Payload aborted");
        } else if (strcmp(action, "forget") == 0) {
            if (pBle) pBle->forgetBonds();
            server.send(200, "text/plain", "Bonds cleared");
        } else {
            server.send(400, "text/plain", "Unknown action");
        }
    }

    // --- Scanner handlers (new) ---

    void handleScanControl() {
        if (!server.hasArg("plain")) {
            server.send(400, "application/json", "{\"ok\":false,\"error\":\"Missing body\"}");
            return;
        }
        JsonDocument doc;
        DeserializationError err = deserializeJson(doc, server.arg("plain"));
        if (err) {
            server.send(400, "application/json", "{\"ok\":false,\"error\":\"Invalid JSON\"}");
            return;
        }

        const char* action = doc["action"];
        if (!action) {
            server.send(400, "application/json", "{\"ok\":false,\"error\":\"Missing action\"}");
            return;
        }

        if (strcmp(action, "start") == 0) {
            uint32_t dur = doc["duration"] | 10;
            if (dur < 3) dur = 3;
            if (dur > 30) dur = 30;
            // Must stop advertising before scanning (shared radio)
            if (pBle && pBle->phase != "idle") pBle->stopAdvertising();
            if (pScanner) pScanner->startScan(dur);
            addLog("BLE scan started (" + String(dur) + "s)");
            server.send(200, "application/json", "{\"ok\":true,\"message\":\"Scan started\"}");
        } else if (strcmp(action, "stop") == 0) {
            if (pScanner) pScanner->stopScan();
            server.send(200, "application/json", "{\"ok\":true,\"message\":\"Scan stopped\"}");
        } else {
            server.send(400, "application/json", "{\"ok\":false,\"error\":\"Unknown action\"}");
        }
    }

    void handleScanResults() {
        JsonDocument doc;
        doc["scanning"] = pScanner ? pScanner->scanning : false;
        doc["done"] = pScanner ? pScanner->scanDone : false;

        JsonArray devArr = doc["devices"].to<JsonArray>();
        if (pScanner) {
            for (const auto& d : pScanner->devices) {
                JsonObject obj = devArr.add<JsonObject>();
                obj["name"] = d.name;
                obj["address"] = d.address;
                obj["rssi"] = d.rssi;
                obj["appearance"] = d.appearance;
                obj["services"] = d.services;
            }
        }

        String response;
        serializeJson(doc, response);
        server.send(200, "application/json", response);
    }

    void handleScanClone() {
        if (!server.hasArg("plain")) {
            server.send(400, "application/json", "{\"ok\":false,\"error\":\"Missing body\"}");
            return;
        }
        JsonDocument doc;
        DeserializationError err = deserializeJson(doc, server.arg("plain"));
        if (err) {
            server.send(400, "application/json", "{\"ok\":false,\"error\":\"Invalid JSON\"}");
            return;
        }

        int index = doc["index"] | -1;
        if (!pScanner || index < 0 || index >= (int)pScanner->devices.size()) {
            server.send(400, "application/json", "{\"ok\":false,\"error\":\"Invalid device index\"}");
            return;
        }

        // Stop scanner if still running
        if (pScanner->scanning) pScanner->stopScan();

        const ScannedDevice& target = pScanner->devices[index];
        String cloneName = target.name.length() > 0 ? target.name : "BLE Device";
        uint16_t cloneAppearance = target.appearance > 0 ? target.appearance : 961;

        if (pBle) {
            // Clone the target's identity and start advertising
            pBle->updateConfig(cloneName, cloneAppearance, pBle->duration, pBle->swiftMode);
            pBle->startAdvertising();
            addLog("Cloned: " + cloneName + " (0x" + String(cloneAppearance, HEX) + ") -> Advertising");
        }

        JsonDocument resp;
        resp["ok"] = true;
        resp["name"] = cloneName;
        resp["appearance"] = cloneAppearance;
        resp["address"] = target.address;
        String response;
        serializeJson(resp, response);
        server.send(200, "application/json", response);
    }
};

#endif // WEB_MANAGER_H
