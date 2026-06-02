/*
  
   Wifi_fl.h

*/

#pragma once

#include <Arduino.h>
#include <WiFi.h>
#include <ESPmDNS.h>
#include "Config.h"
#include "Settings.h"

struct Fl_Wifi
{
    static void start();
};
