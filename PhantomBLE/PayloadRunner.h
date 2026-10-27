#ifndef PAYLOAD_RUNNER_H
#define PAYLOAD_RUNNER_H

#include <Arduino.h>
#include "BleManager.h"

class PayloadRunner {
public:
    bool payloadActive = false;
    String payloadPhase = "idle"; // "idle", "running", "done", "aborted"
    int payloadProgress = 0;
    int payloadTotal = 0;

    void begin(BleManager* ble) {
        pBle = ble;
    }

    void abort() {
        if (payloadActive) {
            abortRequested = true;
            if (pBle) {
                pBle->releaseKeys();
                pBle->log("Payload aborted by user.");
            }
        }
    }

    void runScript(const String& script) {
        if (payloadActive) return;
        currentScript = script;
        abortRequested = false;
        payloadActive = true;
        payloadPhase = "running";
        payloadProgress = 0;

        // Parse lines
        lines.clear();
        int start = 0;
        while (start < (int)currentScript.length()) {
            int end = currentScript.indexOf('\n', start);
            if (end == -1) end = currentScript.length();
            String line = currentScript.substring(start, end);
            line.trim();
            if (line.length() > 0 && !line.startsWith("REM")) {
                lines.push_back(line);
            }
            start = end + 1;
        }

        payloadTotal = lines.size();
        if (payloadTotal == 0) {
            payloadActive = false;
            payloadPhase = "idle";
            return;
        }

        // Spawn FreeRTOS task to execute script without blocking web server
        xTaskCreatePinnedToCore(
            taskRunner,
            "payloadTask",
            4096,
            this,
            1,
            &taskHandle,
            1
        );
    }

    void runPresetNotepad() {
        String script = 
            "GUI r\n"
            "DELAY 700\n"
            "STRING notepad.exe\n"
            "DELAY 300\n"
            "ENTER\n"
            "DELAY 1000\n"
            "STRING Hello From PhantomBLE\n";
        runScript(script);
    }

private:
    BleManager* pBle = nullptr;
    String currentScript;
    std::vector<String> lines;
    bool abortRequested = false;
    TaskHandle_t taskHandle = nullptr;

    static void taskRunner(void* param) {
        PayloadRunner* self = (PayloadRunner*)param;
        self->execute();
        vTaskDelete(NULL);
    }

    void execute() {
        if (!pBle || !pBle->hidReady) {
            payloadActive = false;
            payloadPhase = "idle";
            return;
        }

        pBle->log("Starting payload execution (" + String(payloadTotal) + " steps)");

        for (size_t i = 0; i < lines.size(); i++) {
            if (abortRequested || !pBle->hidReady) {
                payloadPhase = "aborted";
                break;
            }

            payloadProgress = i + 1;
            String line = lines[i];

            if (line.startsWith("STRING ")) {
                String str = line.substring(7);
                pBle->sendString(str, 12);
            } else if (line.startsWith("DELAY ")) {
                int ms = line.substring(6).toInt();
                int waited = 0;
                while (waited < ms && !abortRequested) {
                    delay(50);
                    waited += 50;
                }
            } else if (line.equalsIgnoreCase("ENTER")) {
                pBle->tapKey(0, KEY_ENTER, 30);
            } else if (line.equalsIgnoreCase("TAB")) {
                pBle->tapKey(0, KEY_TAB, 30);
            } else if (line.equalsIgnoreCase("ESC")) {
                pBle->tapKey(0, KEY_ESC, 30);
            } else if (line.equalsIgnoreCase("BACKSPACE")) {
                pBle->tapKey(0, KEY_BACKSPACE, 30);
            } else if (line.equalsIgnoreCase("SPACE")) {
                pBle->tapKey(0, KEY_SPACE, 30);
            } else if (line.equalsIgnoreCase("LEFT")) {
                pBle->tapKey(0, KEY_LEFT, 30);
            } else if (line.equalsIgnoreCase("RIGHT")) {
                pBle->tapKey(0, KEY_RIGHT, 30);
            } else if (line.equalsIgnoreCase("UP")) {
                pBle->tapKey(0, KEY_UP, 30);
            } else if (line.equalsIgnoreCase("DOWN")) {
                pBle->tapKey(0, KEY_DOWN, 30);
            } else if (line.startsWith("GUI ")) {
                char k = line.charAt(4);
                AsciiKey ak = getAsciiKey(k);
                pBle->tapKey(KEY_MOD_LMETA | ak.modifier, ak.keycode, 40);
            } else if (line.startsWith("CTRL ")) {
                char k = line.charAt(5);
                AsciiKey ak = getAsciiKey(k);
                pBle->tapKey(KEY_MOD_LCTRL | ak.modifier, ak.keycode, 40);
            } else if (line.startsWith("ALT ")) {
                char k = line.charAt(4);
                AsciiKey ak = getAsciiKey(k);
                pBle->tapKey(KEY_MOD_LALT | ak.modifier, ak.keycode, 40);
            } else if (line.equalsIgnoreCase("CTRL SHIFT ENTER")) {
                pBle->tapKey(KEY_MOD_LCTRL | KEY_MOD_LSHIFT, KEY_ENTER, 40);
            } else if (line.equalsIgnoreCase("CTRL SHIFT ESC")) {
                pBle->tapKey(KEY_MOD_LCTRL | KEY_MOD_LSHIFT, KEY_ESC, 40);
            }

            delay(30);
        }

        pBle->releaseKeys();
        if (!abortRequested && payloadPhase != "aborted") {
            payloadPhase = "done";
            pBle->log("Payload completed successfully.");
        }
        payloadActive = false;
    }
};

#endif // PAYLOAD_RUNNER_H
