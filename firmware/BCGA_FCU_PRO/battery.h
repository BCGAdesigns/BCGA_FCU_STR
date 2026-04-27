// BCGA FCU PRO — battery.h
// Pro-only battery monitor: ADC sense, cell-count detect, low/cut thresholds,
// firmware lockout mode on critical (firing block + audible alarm).
// V2.1 dropped the hardware kill-latch — physical disconnect is now required
// to fully cut power; lockout only stops firing and pulses the buzzer.

#pragma once

#include "config.h"
#include <Arduino.h>

void     batteryBegin();
void     batteryUpdate();
uint16_t batteryMv();          // 0 if unknown
uint8_t  batteryCells();       // 2 or 3 (0 if unknown)
bool     batteryLow();         // ≤ CELL_WARN_MV/cell
bool     batteryCritical();    // ≤ CELL_CRITICAL_MV/cell (middle tier)
bool     batteryCut();         // ≤ CELL_CUT_MV/cell
void     batteryEnterLockout();// latch firmware lockout state (until reboot)
bool     batteryInLockout();   // true once lockout has been entered
