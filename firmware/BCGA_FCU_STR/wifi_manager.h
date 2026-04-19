// BCGA FCU STR — wifi_manager.h
// AP-only mode + DNS captive portal. Auto-disables after WIFI_AUTO_OFF_MS idle.
// Activation on STR: boot trigger gesture only — hold trigger for 5s during the
// first 5s after boot.

#pragma once

#include <Arduino.h>

void wifiManagerBegin();
void wifiManagerUpdate();
bool wifiActive();
void wifiStart();
void wifiStop();
// Call to mark "user did something on the web UI" — resets idle timer.
void wifiNoteActivity();
