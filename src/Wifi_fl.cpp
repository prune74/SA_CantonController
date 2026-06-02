/*
   Wifi_fl.cpp
*/

#include "Wifi_fl.h"
#include <WiFi.h>
#include <ESPmDNS.h>
#include "Config.h"
#include "Settings.h"

#include <Arduino.h>

void Fl_Wifi::start()
{

#ifdef WIFI_AP_MODE

    WiFi.softAP(WIFI_SSID, WIFI_PSW);

    Serial.print("\n");
    Serial.print("\n------------WIFI------------");
    Serial.print("\nConnected to : ");
    Serial.print(WIFI_SSID);
    Serial.print("\nIP address :   ");
    Serial.print(WiFi.softAPIP());
    Serial.print("\n\n");

#else
    // 🔥 Version corrigée : utilisation des String (comme la Master)
    Serial.print("Connecting to: ");
    Serial.println(Settings::ssid_str);

    WiFi.begin(Settings::ssid_str.c_str(), Settings::password_str.c_str());

    while (WiFi.status() != WL_CONNECTED)
    {
        Serial.println(Settings::ssid_str);
        vTaskDelay(pdMS_TO_TICKS(500));
        Serial.print(".");
    }

    Serial.print("\n");
    Serial.print("\n------------WIFI------------");
    Serial.print("\nConnected to : ");
    Serial.print(Settings::ssid_str);
    Serial.print("\nIP address :   ");
    Serial.print(WiFi.localIP());
    Serial.print("\n----------------------------\n\n");

#endif

    //     if (!MDNS.begin(MDNS_NAME))
    //     {
    //         debug.print("Error setting up MDNS responder!\n");
    //         while (1)
    //             vTaskDelay(pdMS_TO_TICKS(1000));
    //     }

    // #ifdef DEBUG
    //     debug.print("MDNS responder started @ http://");
    //     debug.print(MDNS_NAME);
    //     debug.print(".local");
    //     debug.print("\n\n");
    // #endif
}
