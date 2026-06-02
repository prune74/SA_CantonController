/*

   Settings.h

*/

#ifndef __SETTINGS__
#define __SETTINGS__

#include <Arduino.h>
#include <ArduinoJson.h>
#include <SPIFFS.h>
#include <FS.h>
#include "CanMsg.h"
#include "CanConfig.h"
#include "Config.h"
#include "Node.h"

class Settings
{
private:
    static bool isMainReady;
    static bool WIFI_ON;
    static bool DISCOVERY_ON;
    static Node *node;

public:
    // 🔥 Strings complètes (utilisées par Wifi_fl.cpp)
    static String ssid_str;
    static String password_str;

    // 🔥 Buffers élargis (sécurité + compatibilité WPA2)
    static char ssid[64];
    static char password[64];

    Settings() = delete;

    static void setup(Node *);
    static bool begin();
    static void writeFile();
    static void readFile();

    static void sMainReady(bool);
    static bool discoveryOn();
    static void discoveryOn(bool);
    static bool wifiOn();
    static void wifiOn(bool);
};

#endif
