// BCGA FCU PRO — battery.cpp

#include "battery.h"

namespace {
uint32_t lastPollMs = 0;
uint16_t mvCache    = 0;
uint8_t  cells      = 0;
bool     low        = false;
bool     critical   = false;
bool     cut        = false;

uint16_t readMvOnce() {
  uint32_t acc = 0;
  for (uint8_t i = 0; i < BATTERY_SAMPLES; i++) {
    acc += analogRead(PIN_VBAT);
  }
  uint32_t adc = acc / BATTERY_SAMPLES;
  uint32_t vAdcMv = (adc * ADC_REF_MV) / ADC_RESOLUTION;
  // Reverse divider: Vbat = Vadc * (R1+R2)/R2
  uint32_t vBat = (vAdcMv * VBAT_DIV_DEN) / VBAT_DIV_NUM;
  return (uint16_t)vBat;
}

void detectCells(uint16_t mv) {
  if (mv < V_MIN_VALID_MV) { cells = 0; return; }
  cells = (mv >= V_2S_3S_BOUNDARY_MV) ? 3 : 2;
}

void evaluate(uint16_t mv) {
  if (cells == 0) { low = false; critical = false; cut = false; return; }
  uint16_t perCell = mv / cells;
  low      = perCell <= CELL_WARN_MV;       // ≤ 3500 mV/cell
  critical = perCell <= CELL_CRITICAL_MV;   // ≤ 3200 mV/cell
  cut      = perCell <= CELL_CUT_MV;        // ≤ 3000 mV/cell
}
} // namespace

void batteryBegin() {
  pinMode(PIN_VBAT, INPUT);
  analogReadResolution(12);
  analogSetPinAttenuation(PIN_VBAT, ADC_11db);
  pinMode(PIN_LATCH, OUTPUT);
  digitalWrite(PIN_LATCH, HIGH);   // hold self alive
  // Prime
  uint16_t mv = readMvOnce();
  mvCache = mv;
  detectCells(mv);
  evaluate(mv);
  lastPollMs = millis();
}

void batteryUpdate() {
  if ((uint32_t)(millis() - lastPollMs) < BATTERY_POLL_MS) return;
  lastPollMs = millis();
  uint16_t mv = readMvOnce();
  mvCache = mv;
  if (cells == 0) detectCells(mv);
  evaluate(mv);
}

uint16_t batteryMv()       { return mvCache; }
uint8_t  batteryCells()    { return cells; }
bool     batteryLow()      { return low; }
bool     batteryCritical() { return critical; }
bool     batteryCut()      { return cut; }

void batteryKillLatch() {
  digitalWrite(PIN_LATCH, LOW);
}
