#include <Arduino.h>
#include "BleManager.h"
#include "PayloadRunner.h"
#include "WifiManager.h"
#include "WebManager.h"
#include "SerialCli.h"

BleManager bleManager;
PayloadRunner payloadRunner;
WifiManager wifiManager;
WebManager webManager;
SerialCli serialCli;

void onBleLog(const String& msg) {
    webManager.addLog(msg);
}

void setup() {
    Serial.begin(115200);
    delay(500);

    Serial.println("\r\n[SYSTEM] Booting PhantomBLE for ESP32 DevKit 38-Pin...");

    // Connect logging callback
    bleManager.setLogger(onBleLog);

    // Initialize Wi-Fi SoftAP
    wifiManager.begin();

    // Initialize BLE & HID
    bleManager.begin();

    // Initialize Payload Runner
    payloadRunner.begin(&bleManager);

    // Initialize Web Dashboard and API
    webManager.begin(&bleManager, &payloadRunner, &wifiManager);

    // Initialize Serial CLI
    serialCli.begin(&bleManager, &payloadRunner, &wifiManager);

    Serial.println("[SYSTEM] System ready. Connect to Wi-Fi SoftAP or use Serial CLI.");
}

void loop() {
    // Service HTTP client requests
    webManager.handleClient();

    // Service BLE state timer ticks
    bleManager.tick();

    // Service Serial CLI inputs
    serialCli.handleSerial();

    // Small yield
    vTaskDelay(1 / portTICK_PERIOD_MS);
}
