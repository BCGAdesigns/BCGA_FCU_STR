// BCGA FCU PRO — firing.h
// Non-blocking firing engine. Drives SOL1 (poppet) and SOL2 (nozzle, D8PA).
// Timings stored as MILLISECONDS in SlotConfig; converted internally.
//
// Cycle:
//   D8PA (solenoidCount=2): SOL2 pulse DN → wait DR → SOL1 pulse DP → wait DL → repeat
//   S8PA (solenoidCount=1): SOL1 pulse DP → wait DR → repeat (DN, DL unused)
//
// Default pin mapping:
//   SOL1 (poppet) = PIN_MOS_1
//   SOL2 (nozzle) = PIN_MOS_2
// mosfetSwap = 1 swaps the logical roles, so users can fix a swapped solder
// job in software (D8PA only — no effect in S8PA).
//
// Hardware assumed active-HIGH for MOSFETs (no invert).

#pragma once

#include "types.h"
#include "config.h"

void firingBegin(const SlotConfig& cfg);
void firingReconfig(const SlotConfig& cfg);
void firingOnTriggerEdge(bool pressed, const SlotConfig& cfg, FireMode mode);
void firingUpdate(const SlotConfig& cfg, FireMode mode);
bool firingActive();
void firingForceStop();         // emergency stop (e.g. battery cut)
uint32_t firingShotCount();     // total shots since boot

// Logical pin helpers (respect mosfetSwap).
inline uint8_t pinPoppet(const SlotConfig& cfg) {
  return cfg.mosfetSwap ? PIN_MOS_2 : PIN_MOS_1;   // SOL1
}
inline uint8_t pinNozzle(const SlotConfig& cfg) {
  return cfg.mosfetSwap ? PIN_MOS_1 : PIN_MOS_2;   // SOL2
}

// Non-blocking MOSFET test (web diagnostic). Pulses the named pin for
// MOS_TEST_DURATION_MS. Refuses if firing is active.
void mosTestSchedule(uint8_t pin);
void mosTestService();
bool mosTestActive();

// Noise calibration: blocking pulse of one full shot cycle while sampling an
// ADC pin at max rate. Returns observed min/max counts. Refuses if active.
struct NoiseReport { int adcMin; int adcMax; uint16_t samples; bool ok; };
NoiseReport firingPulseCycleWithSample(const SlotConfig& cfg, uint8_t adcPin);
