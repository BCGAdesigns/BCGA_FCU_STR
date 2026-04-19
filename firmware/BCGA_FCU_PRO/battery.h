// BCGA FCU V2 — battery.h
// Pro-only battery monitor: ADC sense, cell-count detect, low/cut thresholds,
// hardware latch kill on critical.
// On Starter, all functions are no-ops (compile-time stubbed).

#pragma once

#include "config.h"
#include <Arduino.h>

void     batteryBegin();
void     batteryUpdate();
uint16_t batteryMv();        // 0 if unknown
uint8_t  batteryCells();     // 2 or 3 (0 if unknown)
bool     batteryLow();       // ≤ CELL_WARN_MV/cell
bool     batteryCritical();  // ≤ CELL_CRITICAL_MV/cell (middle tier)
bool     batteryCut();       // ≤ CELL_CUT_MV/cell
void     batteryKillLatch(); // force latch off (Pro only)
