// BCGA FCU PRO — wifi_manager.h
// AP-only mode + DNS captive portal. Auto-disables after WIFI_AUTO_OFF_MS idle.
// Activation on PRO:
//   • WIFI_BTN hold ≥3s → toggle WiFi on/off (3 beeps on turn-on).
//   • WIFI_BTN hold ≥30s → reset WiFi password to WIFI_PASS_DEFAULT.
//   • Boot gesture: hold trigger for 5s during first 5s of boot → WiFi on.

#pragma once

#include <Arduino.h>

void wifiManagerBegin();
void wifiManagerUpdate();
bool wifiActive();
void wifiStart();
void wifiStop();
// Call to mark "user did something on the web UI" — resets idle timer.
void wifiNoteActivity();
