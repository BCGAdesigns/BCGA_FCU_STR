// BCGA FCU STR — firing.cpp
// State machine for S8PA (single solenoid). Never blocks except in
// firingPulseCycleWithSample (used for noise calibration).
// Active-HIGH MOSFET on PIN_MOS_1.

#include "firing.h"
#include "config.h"

namespace {

enum FireState : uint8_t {
  FS_IDLE = 0,
  FS_POPPET_PULSE,    // SOL1 driving for DP ms (the actual shot)
  FS_POST_DELAY       // both off — waits DR ms inter-shot
};

FireState  state          = FS_IDLE;
uint32_t   stateEndUs     = 0;
bool       triggerHeld    = false;
bool       cycleArmed     = false;
uint16_t   shotsRemaining = 0;
uint32_t   minIntervalUs  = 0;
uint32_t   lastShotEndUs  = 0;
uint32_t   shotsTotal     = 0;

// MOSFET test scheduler (web diagnostic).
uint8_t    mosTestPin       = 0;
uint32_t   mosTestEndMs     = 0;

inline void writeMos(uint8_t pin, bool on) { digitalWrite(pin, on ? HIGH : LOW); }

uint16_t clampMs(uint16_t v) {
  if (v < FIRE_MIN_MS) return FIRE_MIN_MS;
  if (v > FIRE_MAX_MS) return FIRE_MAX_MS;
  return v;
}

inline uint32_t msToUs(uint16_t ms) { return (uint32_t)clampMs(ms) * 1000UL; }

void recomputeRof(const SlotConfig& cfg) {
  if (cfg.rofLimit == 0) { minIntervalUs = 0; return; }
  minIntervalUs = 1000000UL / cfg.rofLimit;
}

void startShot(const SlotConfig& cfg) {
  uint32_t now = micros();
  if (minIntervalUs && (uint32_t)(now - lastShotEndUs) < minIntervalUs) return;

  writeMos(pinPoppet(cfg), true);
  state      = FS_POPPET_PULSE;
  stateEndUs = now + msToUs(cfg.dp);
  lastShotEndUs = now;
  shotsTotal++;
}

}  // namespace

void firingBegin(const SlotConfig& cfg) {
  pinMode(PIN_MOS_1, OUTPUT);
  digitalWrite(PIN_MOS_1, LOW);
  state = FS_IDLE;
  recomputeRof(cfg);
}

void firingReconfig(const SlotConfig& cfg) {
  recomputeRof(cfg);
}

void firingOnTriggerEdge(bool pressed, const SlotConfig& cfg, FireMode mode) {
  (void)cfg;
  triggerHeld = pressed;
  if (mode == FIRE_SAFE) { cycleArmed = false; shotsRemaining = 0; return; }
  if (pressed) {
    cycleArmed = true;
    if (mode == FIRE_SEMI)        shotsRemaining = 1;
    else if (mode == FIRE_FULL)   shotsRemaining = 0xFFFF;
    else if (isBurstMode(mode))   shotsRemaining = burstCountOf(mode);
  } else {
    if (mode == FIRE_FULL) shotsRemaining = 0;
  }
}

void firingUpdate(const SlotConfig& cfg, FireMode mode) {
  uint32_t now = micros();

  switch (state) {
    case FS_IDLE: {
      if (mode == FIRE_SAFE) { cycleArmed = false; shotsRemaining = 0; return; }
      if (cycleArmed && shotsRemaining > 0) {
        if (mode == FIRE_FULL && !triggerHeld) {
          cycleArmed = false;
          return;
        }
        startShot(cfg);
      }
      break;
    }

    case FS_POPPET_PULSE: {
      if ((int32_t)(now - stateEndUs) < 0) return;
      writeMos(pinPoppet(cfg), false);
      state = FS_POST_DELAY;
      stateEndUs = now + msToUs(cfg.dr);
      if (shotsRemaining != 0xFFFF && shotsRemaining > 0) shotsRemaining--;
      break;
    }

    case FS_POST_DELAY: {
      if ((int32_t)(now - stateEndUs) < 0) return;
      state = FS_IDLE;
      if (shotsRemaining == 0) cycleArmed = false;
      break;
    }
  }
}

bool firingActive() { return state != FS_IDLE; }

void firingForceStop() {
  digitalWrite(PIN_MOS_1, LOW);
  state = FS_IDLE;
  cycleArmed = false;
  shotsRemaining = 0;
}

uint32_t firingShotCount() { return shotsTotal; }

// ============================================================================
// MOSFET TEST SCHEDULER — non-blocking, active-HIGH only
// ============================================================================

void mosTestSchedule(uint8_t pin) {
  if (firingActive()) return;
  if (pin != PIN_MOS_1) return;
  mosTestPin   = pin;
  mosTestEndMs = millis() + MOS_TEST_DURATION_MS;
  pinMode(pin, OUTPUT);
  writeMos(pin, true);
}

void mosTestService() {
  if (mosTestPin == 0) return;
  if ((int32_t)(millis() - mosTestEndMs) < 0) return;
  writeMos(mosTestPin, false);
  mosTestPin = 0;
}

bool mosTestActive() { return mosTestPin != 0; }

// ============================================================================
// NOISE CALIBRATION — blocking pulse cycle while sampling ADC
// ============================================================================
// S8PA: pulse SOL1 for DP, then wait DR. Reads the ADC pin during both phases.
// Returns min/max ADC values observed — the spread is the EMI noise margin
// caused by the solenoid switching.

NoiseReport firingPulseCycleWithSample(const SlotConfig& cfg, uint8_t adcPin) {
  NoiseReport rep = { 4095, 0, 0, false };
  if (firingActive() || mosTestActive()) return rep;

  const uint8_t pp = pinPoppet(cfg);

  auto sampleFor = [&](uint32_t durUs) {
    uint32_t t0 = micros();
    while ((uint32_t)(micros() - t0) < durUs) {
      int v = analogRead(adcPin);
      if (v < rep.adcMin) rep.adcMin = v;
      if (v > rep.adcMax) rep.adcMax = v;
      rep.samples++;
      if (rep.samples >= 2000) return;
    }
  };

  writeMos(pp, true);
  sampleFor(msToUs(cfg.dp));
  writeMos(pp, false);
  sampleFor(msToUs(cfg.dr));

  rep.ok = (rep.samples > 0);
  return rep;
}
