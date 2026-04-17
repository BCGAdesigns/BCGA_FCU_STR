// BCGA FCU STR — firing.h
// Non-blocking firing engine for S8PA (single solenoid).
// Cycle: SOL1 pulse DP → wait DR → repeat.
// Hardware assumed active-HIGH for the MOSFET (no invert).

#pragma once

#include "types.h"
#include "config.h"

void firingBegin(const SlotConfig& cfg);
void firingReconfig(const SlotConfig& cfg);
void firingOnTriggerEdge(bool pressed, const SlotConfig& cfg, FireMode mode);
void firingUpdate(const SlotConfig& cfg, FireMode mode);
bool firingActive();
void firingForceStop();         // emergency stop
uint32_t firingShotCount();     // total shots since boot

inline uint8_t pinPoppet(const SlotConfig& cfg) { (void)cfg; return PIN_MOS_1; }

// Non-blocking MOSFET test (web diagnostic). Pulses the named pin for
// MOS_TEST_DURATION_MS. Refuses if firing is active.
void mosTestSchedule(uint8_t pin);
void mosTestService();
bool mosTestActive();

// Noise calibration: blocking pulse of one full shot cycle while sampling an
// ADC pin at max rate. Returns observed min/max counts. Refuses if active.
struct NoiseReport { int adcMin; int adcMax; uint16_t samples; bool ok; };
NoiseReport firingPulseCycleWithSample(const SlotConfig& cfg, uint8_t adcPin);
