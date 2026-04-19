// BCGA FCU PRO — firing.cpp
// State machine — never blocks (except firingPulseCycleWithSample for noise cal).
// Timings stored as MILLISECONDS in SlotConfig; converted to microseconds here.
// Active-HIGH MOSFETs. mosfetSwap (D8PA only) swaps logical roles of SOL1/SOL2.

#include "firing.h"
#include "config.h"

namespace {

enum FireState : uint8_t {
  FS_IDLE = 0,
  FS_NOZZLE_PULSE,    // D8PA: SOL2 driving for DN ms
  FS_RETURN_WAIT,     // D8PA: both off for DR ms (spring seals bucking)
  FS_POPPET_PULSE,    // SOL1 driving for DP ms (the actual shot)
  FS_POST_DELAY       // both off — D8PA waits DL; S8PA waits DR
};

FireState  state          = FS_IDLE;
uint32_t   stateEndUs     = 0;
bool       triggerHeld    = false;
bool       cycleArmed     = false;
uint16_t   shotsRemaining = 0;
uint32_t   minIntervalUs  = 0;
uint32_t   lastShotEndUs  = 0;
uint32_t   shotsTotal     = 0;
// Tracks the cycle end of the most recent SEMI shot for semiRofMs blocking.
uint32_t   lastSemiShotEndMs = 0;
bool       lastCycleWasSemi  = false;

// MOSFET test scheduler (web diagnostic) — non-blocking, active-HIGH only.
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

  if (cfg.solenoidCount == 2) {
    // D8PA: pulse nozzle (SOL2) first
    writeMos(pinNozzle(cfg), true);
    state      = FS_NOZZLE_PULSE;
    stateEndUs = now + msToUs(cfg.dn);
  } else {
    // S8PA: only the poppet (SOL1) is wired
    writeMos(pinPoppet(cfg), true);
    state      = FS_POPPET_PULSE;
    stateEndUs = now + msToUs(cfg.dp);
  }
  lastShotEndUs = now;
  shotsTotal++;
}

}  // namespace

void firingBegin(const SlotConfig& cfg) {
  pinMode(PIN_MOS_1, OUTPUT);
  digitalWrite(PIN_MOS_1, LOW);
  pinMode(PIN_MOS_2, OUTPUT);
  digitalWrite(PIN_MOS_2, LOW);
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
    lastCycleWasSemi = (mode == FIRE_SEMI);
    if (mode == FIRE_SEMI)        shotsRemaining = 1;
    else if (mode == FIRE_FULL)   shotsRemaining = 0xFFFF;
    else if (isBurstMode(mode))   shotsRemaining = burstCountOf(mode);
  } else {
    if (mode == FIRE_FULL) shotsRemaining = 0;
  }
}

bool firingIsSemiBlocked(const SlotConfig& cfg) {
  if (cfg.semiRofMs == 0) return false;
  if (lastSemiShotEndMs == 0) return false;
  return (uint32_t)(millis() - lastSemiShotEndMs) < cfg.semiRofMs;
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

    case FS_NOZZLE_PULSE: {
      if ((int32_t)(now - stateEndUs) < 0) return;
      writeMos(pinNozzle(cfg), false);
      state      = FS_RETURN_WAIT;
      stateEndUs = now + msToUs(cfg.dr);
      break;
    }

    case FS_RETURN_WAIT: {
      if ((int32_t)(now - stateEndUs) < 0) return;
      writeMos(pinPoppet(cfg), true);
      state      = FS_POPPET_PULSE;
      stateEndUs = now + msToUs(cfg.dp);
      break;
    }

    case FS_POPPET_PULSE: {
      if ((int32_t)(now - stateEndUs) < 0) return;
      writeMos(pinPoppet(cfg), false);
      state = FS_POST_DELAY;
      // D8PA uses DL; S8PA uses DR as inter-shot rest.
      uint16_t postMs = (cfg.solenoidCount == 2) ? cfg.dl : cfg.dr;
      stateEndUs = now + msToUs(postMs);
      if (shotsRemaining != 0xFFFF && shotsRemaining > 0) shotsRemaining--;
      break;
    }

    case FS_POST_DELAY: {
      if ((int32_t)(now - stateEndUs) < 0) return;
      state = FS_IDLE;
      if (shotsRemaining == 0) {
        cycleArmed = false;
        if (lastCycleWasSemi) lastSemiShotEndMs = millis();
      }
      break;
    }
  }
}

bool firingActive() { return state != FS_IDLE; }

void firingForceStop() {
  digitalWrite(PIN_MOS_1, LOW);
  digitalWrite(PIN_MOS_2, LOW);
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
  if (pin != PIN_MOS_1 && pin != PIN_MOS_2) return;
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
// Fires one complete cycle and reads the ADC pin as fast as possible during
// the entire window. Returns min/max ADC values observed — the spread is the
// EMI noise margin caused by the solenoid switching, used to widen the Hall
// trigger deadband.

NoiseReport firingPulseCycleWithSample(const SlotConfig& cfg, uint8_t adcPin) {
  NoiseReport rep = { 4095, 0, 0, false };
  if (firingActive() || mosTestActive()) return rep;

  const uint8_t pp = pinPoppet(cfg);
  const uint8_t nz = pinNozzle(cfg);

  auto sampleFor = [&](uint32_t durUs) {
    uint32_t t0 = micros();
    while ((uint32_t)(micros() - t0) < durUs) {
      int v = analogRead(adcPin);
      if (v < rep.adcMin) rep.adcMin = v;
      if (v > rep.adcMax) rep.adcMax = v;
      rep.samples++;
      if (rep.samples >= 2000) return;   // safety cap
    }
  };

  if (cfg.solenoidCount == 2) {
    // D8PA: DN nozzle ON
    writeMos(nz, true);
    sampleFor(msToUs(cfg.dn));
    writeMos(nz, false);
    // DR seal wait
    sampleFor(msToUs(cfg.dr));
    // DP poppet ON
    writeMos(pp, true);
    sampleFor(msToUs(cfg.dp));
    writeMos(pp, false);
    // DL post-shot delay
    sampleFor(msToUs(cfg.dl));
  } else {
    // S8PA: DP poppet ON, then DR rest
    writeMos(pp, true);
    sampleFor(msToUs(cfg.dp));
    writeMos(pp, false);
    sampleFor(msToUs(cfg.dr));
  }

  rep.ok = (rep.samples > 0);
  return rep;
}
