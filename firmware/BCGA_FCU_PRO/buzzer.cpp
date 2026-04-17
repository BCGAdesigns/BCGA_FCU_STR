// BCGA FCU V2 — buzzer.cpp

#include "buzzer.h"
#include "config.h"

namespace {

struct Step {
  uint16_t freqHz;     // 0 = silence
  uint16_t durMs;
};

constexpr uint8_t MAX_STEPS = 12;

struct PatternData {
  Step    steps[MAX_STEPS];
  uint8_t len;
};

const PatternData PATTERNS[] = {
  // BUZZ_NONE
  { {{0,0}}, 0 },
  // BUZZ_BOOT
  { {{1500,40},{2000,40},{2700,80}}, 3 },
  // BUZZ_READY
  { {{2700,80}}, 1 },
  // BUZZ_SLOT — overridden by buzzerPlayCount()
  { {{2700,60},{0,80}}, 2 },
  // BUZZ_MODE_CHANGE
  { {{2400,40},{0,40},{2400,40}}, 3 },
  // BUZZ_SAVE_OK
  { {{2200,60},{2700,90}}, 2 },
  // BUZZ_ERROR
  { {{600,300}}, 1 },
  // BUZZ_LOW_BATT
  { {{2200,80},{0,80},{1900,80},{0,80},{1600,120}}, 5 },
  // BUZZ_BATT_CUT
  { {{1200,250},{900,250},{600,400}}, 3 },
  // BUZZ_WIFI_ON
  { {{1800,60},{2200,60},{2700,60},{3200,90}}, 4 },
  // BUZZ_WIFI_OFF
  { {{3200,60},{2700,60},{2200,60},{1800,90}}, 4 },
  // BUZZ_TEST
  { {{2700,1000}}, 1 }
};

PatternData runtimeBuf;            // used for SLOT/PlayCount
const PatternData* current = nullptr;
uint8_t  curIdx = 0;
uint32_t stepEndMs = 0;
bool     toneOn = false;

void startStep(uint8_t idx) {
  if (!current || idx >= current->len) {
    current = nullptr;
    ledcWrite(PIN_BUZZER, 0);
    toneOn = false;
    return;
  }
  curIdx = idx;
  uint16_t f = current->steps[idx].freqHz;
  if (f == 0) {
    ledcWrite(PIN_BUZZER, 0);
    toneOn = false;
  } else {
    ledcWriteTone(PIN_BUZZER, f);
    ledcWrite(PIN_BUZZER, 128);   // 50% duty
    toneOn = true;
  }
  stepEndMs = millis() + current->steps[idx].durMs;
}

}  // namespace

void buzzerBegin() {
  // ESP32 Arduino Core 3.x: pin-based LEDC API
  ledcAttach(PIN_BUZZER, LEDC_BUZZER_DEFAULT_FREQ, LEDC_BUZZER_RES);
  ledcWrite(PIN_BUZZER, 0);
  current = nullptr;
  toneOn = false;
}

void buzzerPlay(BuzzPattern p) {
  if (p == BUZZ_NONE || p >= sizeof(PATTERNS)/sizeof(PATTERNS[0])) {
    buzzerStop();
    return;
  }
  current = &PATTERNS[p];
  startStep(0);
}

void buzzerPlayCount(uint8_t n) {
  if (n == 0) { buzzerStop(); return; }
  if (n > 5) n = 5;
  runtimeBuf.len = 0;
  for (uint8_t i = 0; i < n && (runtimeBuf.len + 2) <= MAX_STEPS; i++) {
    runtimeBuf.steps[runtimeBuf.len++] = {2700, 70};
    runtimeBuf.steps[runtimeBuf.len++] = {0, 90};
  }
  current = &runtimeBuf;
  startStep(0);
}

void buzzerStop() {
  current = nullptr;
  ledcWrite(PIN_BUZZER, 0);
  toneOn = false;
}

void buzzerUpdate() {
  if (!current) return;
  if ((int32_t)(millis() - stepEndMs) < 0) return;
  startStep(curIdx + 1);
}

bool buzzerBusy() { return current != nullptr; }
