#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <Arduino.h>
#include <WiFi.h>
#include <Preferences.h>

class WifiManager {
public:
    String ssid;
    String password;
    Preferences prefs;

    void begin() {
        prefs.begin("phantom_wifi", false);
        
        uint8_t mac[6];
        WiFi.macAddress(mac);
        char defaultSsid[32];
        snprintf(defaultSsid, sizeof(defaultSsid), "PhantomBLE-%02X%02X", mac[4], mac[5]);

        ssid = prefs.getString("ssid", defaultSsid);
        password = prefs.getString("password", "");

        IPAddress apIP(192, 168, 4, 1);
        IPAddress netMsk(255, 255, 255, 0);

        WiFi.mode(WIFI_AP);
        WiFi.softAPConfig(apIP, apIP, netMsk);
        
        if (password.length() >= 8) {
            WiFi.softAP(ssid.c_str(), password.c_str());
        } else {
            WiFi.softAP(ssid.c_str());
        }

        Serial.printf("[WIFI] SoftAP started: %s (IP: 192.168.4.1)\n", ssid.c_str());
    }

    bool updateCredentials(const String& newSsid, const String& newPass) {
        if (newSsid.length() == 0 || newSsid.length() > 31) return false;
        if (newPass.length() > 0 && newPass.length() < 8) return false;

        ssid = newSsid;
        password = newPass;

        prefs.putString("ssid", ssid);
        prefs.putString("password", password);

        // Restart SoftAP
        WiFi.softAPdisconnect(true);
        delay(100);
        IPAddress apIP(192, 168, 4, 1);
        IPAddress netMsk(255, 255, 255, 0);
        WiFi.softAPConfig(apIP, apIP, netMsk);
        if (password.length() >= 8) {
            WiFi.softAP(ssid.c_str(), password.c_str());
        } else {
            WiFi.softAP(ssid.c_str());
        }
        return true;
    }
};

#endif // WIFI_MANAGER_H
