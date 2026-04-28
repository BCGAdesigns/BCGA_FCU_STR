// BCGA FCU STR — buzzer.cpp
// Active magnetic buzzer (FUET-9650B-3V): the oscillator is built into the
// part, so the GPIO just toggles it on/off via digitalWrite. Patterns are
// distinguished by beep count and duration — frequency is fixed (~3 kHz).

#include "buzzer.h"
#include "config.h"

namespace {

struct Step {
  bool     on;          // true = buzzer on, false = silence
  uint16_t durMs;       // step duration
};

constexpr uint8_t MAX_STEPS = 12;

struct PatternData {
  Step    steps[MAX_STEPS];
  uint8_t len;
};

const PatternData PATTERNS[] = {
  // BUZZ_NONE
  { {{false, 0}}, 0 },
  // BUZZ_BOOT — 1 short beep
  { {{true, 80}}, 1 },
  // BUZZ_READY — 1 long beep
  { {{true, 200}}, 1 },
  // BUZZ_SLOT — overridden at runtime by buzzerPlayCount()
  { {{true, 70}, {false, 90}}, 2 },
  // BUZZ_MODE_CHANGE — 1 short click
  { {{true, 40}}, 1 },
  // BUZZ_SAVE_OK — short + long pair (tick-tock)
  { {{true, 60}, {false, 80}, {true, 100}}, 3 },
  // BUZZ_ERROR — 1 sustained beep
  { {{true, 500}}, 1 },
  // BUZZ_LOW_BATT — 3 evenly-spaced beeps
  { {{true, 80}, {false, 80}, {true, 80}, {false, 80}, {true, 80}}, 5 },
  // BUZZ_BATT_CUT — 3 long beeps + 1 longer terminal beep (alarm-y)
  { {{true, 250}, {false, 150}, {true, 250}, {false, 150}, {true, 250}, {false, 150}, {true, 400}}, 7 },
  // BUZZ_WIFI_ON — 4 short beeps with a slightly longer last one
  { {{true, 60}, {false, 60}, {true, 60}, {false, 60}, {true, 60}, {false, 60}, {true, 90}}, 7 },
  // BUZZ_WIFI_OFF — short + long descending feel (slower than WIFI_ON)
  { {{true, 100}, {false, 100}, {true, 200}}, 3 },
  // BUZZ_TEST — 1 s steady tone (web "Test buzzer" button)
  { {{true, 1000}}, 1 },
  // BUZZ_INACTIVITY_ALERT — 1 short beep, fires every 30 s during alarm
  { {{true, 100}}, 1 }
};

PatternData runtimeBuf;            // used for SLOT/PlayCount
const PatternData* current = nullptr;
uint8_t  curIdx = 0;
uint32_t stepEndMs = 0;
bool     toneOn = false;

void startStep(uint8_t idx) {
  if (!current || idx >= current->len) {
    current = nullptr;
    digitalWrite(PIN_BUZZER, LOW);
    toneOn = false;
    return;
  }
  curIdx = idx;
  bool on = current->steps[idx].on;
  digitalWrite(PIN_BUZZER, on ? HIGH : LOW);
  toneOn = on;
  stepEndMs = millis() + current->steps[idx].durMs;
}

}  // namespace

void buzzerBegin() {
  pinMode(PIN_BUZZER, OUTPUT);
  digitalWrite(PIN_BUZZER, LOW);
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
    runtimeBuf.steps[runtimeBuf.len++] = {true, 70};
    runtimeBuf.steps[runtimeBuf.len++] = {false, 90};
  }
  current = &runtimeBuf;
  startStep(0);
}

void buzzerStop() {
  current = nullptr;
  digitalWrite(PIN_BUZZER, LOW);
  toneOn = false;
}

void buzzerUpdate() {
  if (!current) return;
  if ((int32_t)(millis() - stepEndMs) < 0) return;
  startStep(curIdx + 1);
}

bool buzzerBusy() { return current != nullptr; }
