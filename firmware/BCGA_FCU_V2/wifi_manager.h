// BCGA FCU V2 — wifi_manager.h
// AP-only mode + DNS captive portal. Auto-disables after WIFI_AUTO_OFF_MS idle.
// Activation: explicit start (Pro = WiFi button), or "trigger gesture" on Starter
// (5 trigger pulls within 3s when in SAFE).

#pragma once

#include <Arduino.h>

void wifiManagerBegin();
void wifiManagerUpdate();
bool wifiActive();
void wifiStart();
void wifiStop();
// Call when trigger goes pressed→released (used for the 5-pull gesture).
void wifiNoteTriggerPull();
// Call to mark "user did something on the web UI" — resets idle timer.
void wifiNoteActivity();
