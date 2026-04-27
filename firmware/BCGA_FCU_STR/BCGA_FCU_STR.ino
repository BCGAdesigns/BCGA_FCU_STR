// BCGA FCU STR — main sketch
// Target: ESP32-C3 SuperMini, Arduino-ESP32 core.
// Variant: STR (Starter, perfboard) — single solenoid (S8PA), no battery,
// no WiFi button (5-pull trigger gesture in SAFE brings up the AP).
//
// Core loop is fully non-blocking: firing state machine, buzzer player,
// WiFi manager, and web server all share the same loop().

#include "config.h"
#include "types.h"
#include "selector.h"
#include "buzzer.h"
#include "firing.h"
#include "storage.h"
#include "wifi_manager.h"
#include "web_server.h"
#include "display.h"

#include <esp_sleep.h>

// ── Inactivity alarm state ─────────────────────────────────────────────────
// RTC_DATA_ATTR survives deep sleep, so a multi-cycle alarm sequence can resume
// across timer-wake reboots. Reset to 0/false on a normal (cold or GPIO) wake.
RTC_DATA_ATTR uint8_t  alarmCycleCount = 0;
RTC_DATA_ATTR bool     inAlarmCycle    = false;

namespace {

SlotConfig    activeCfg;
RuntimeState  rs;

// Trigger debounce / edge tracking
bool      trigCurrent  = false;     // logical: true == pressed
bool      trigLastRaw  = false;
uint32_t  trigDebounceUntilMs = 0;

uint32_t  shotsAtLastPersist = 0;
uint32_t  lastShotPersistMs  = 0;

// Activity tracking — last user input of any kind (trigger, selector, HTTP
// request). Drives the inactivity-alarm watchdog.
uint32_t  lastActivityMs        = 0;
uint32_t  trigTestEventCount    = 0;   // increments on every logical edge

// Inactivity alarm timing (volatile within a cycle; cycle count is the
// RTC_DATA_ATTR above)
uint32_t  alarmBurstStartMs = 0;
uint32_t  lastAlarmBeepMs   = 0;

// Skip BOOT/READY beeps and the boot WiFi gesture when waking back into the
// middle of an alarm sequence (no user is around).
bool      bootIsAlarmResume = false;

void applySlot(uint8_t idx) {
  storageLoadSlot(idx, activeCfg);
  rs.activeSlot = idx;
  storageSetLastSlot(idx);
  selectorReconfig(activeCfg);
  firingReconfig(activeCfg);
  buzzerPlayCount(idx + 1);
}

bool readTriggerRaw() {
  if (activeCfg.trigMode == SWITCH_DIGITAL) {
    // Default wiring (invertTrig=0): INPUT_PULLUP + switch to GND => LOW when pressed.
    bool low = (digitalRead(PIN_TRIG) == LOW);
    return activeCfg.invertTrig ? !low : low;
  }
  int a = analogRead(PIN_TRIG);
  if (a >= activeCfg.hallTrigHigh) return true;
  if (a <= activeCfg.hallTrigLow)  return false;
  return trigCurrent;   // hysteresis band
}

void serviceTrigger() {
  uint32_t now = millis();
  bool raw = readTriggerRaw();
  if (raw != trigLastRaw) {
    trigLastRaw = raw;
    trigDebounceUntilMs = now + DEBOUNCE_MS;
    return;
  }
  if (now < trigDebounceUntilMs) return;
  if (raw == trigCurrent) return;

  trigCurrent = raw;
  lastActivityMs = now;
  // Trigger input also clears any in-progress alarm cycle.
  if (inAlarmCycle) {
    inAlarmCycle      = false;
    alarmCycleCount   = 0;
    alarmBurstStartMs = 0;
    lastAlarmBeepMs   = 0;
  }
  trigTestEventCount++;
  FireMode mode = selectorReadMode(activeCfg);

  // Semi ROF cap: swallow the press if we're still inside the cooldown.
  if (trigCurrent && mode == FIRE_SEMI && firingIsSemiBlocked(activeCfg)) {
    return;
  }

  firingOnTriggerEdge(trigCurrent, activeCfg, mode);
}

void serviceTriggerHallPoll() {
  // Hall mode polled at ~2 kHz via micros guard
  static uint32_t lastUs = 0;
  uint32_t u = micros();
  if ((uint32_t)(u - lastUs) < HALL_POLL_US) return;
  lastUs = u;
  serviceTrigger();
}

void persistShotsIfNeeded() {
  uint32_t live = firingShotCount();
  if (live == shotsAtLastPersist) return;
  // throttle NVS writes: every 30s if there's been firing
  if ((uint32_t)(millis() - lastShotPersistMs) < 30000) return;
  storageBumpShots(live - shotsAtLastPersist);
  shotsAtLastPersist = live;
  lastShotPersistMs  = millis();
}

// ── Inactivity alarm state machine ─────────────────────────────────────────
enum AlarmSleepCmd : uint8_t { SLEEP_NONE = 0, SLEEP_TIMER, SLEEP_PERMANENT };

AlarmSleepCmd serviceInactivityAlarm() {
  uint32_t now = millis();

  if (!inAlarmCycle) {
    if ((uint32_t)(now - lastActivityMs) < INACTIVITY_TIMEOUT_MS) return SLEEP_NONE;
    inAlarmCycle      = true;
    if (alarmCycleCount == 0) alarmCycleCount = 1;
    alarmBurstStartMs = now;
    lastAlarmBeepMs   = 0;
  }

  uint32_t elapsed = (uint32_t)(now - alarmBurstStartMs);
  if (elapsed < ALARM_BURST_DURATION_MS) {
    if (lastAlarmBeepMs == 0 ||
        (uint32_t)(now - lastAlarmBeepMs) >= ALARM_BEEP_INTERVAL_MS) {
      lastAlarmBeepMs = now;
      buzzerPlay(BUZZ_INACTIVITY_ALERT);
    }
    return SLEEP_NONE;
  }

  if (alarmCycleCount >= ALARM_MAX_CYCLES) {
    inAlarmCycle    = false;
    alarmCycleCount = 0;
    return SLEEP_PERMANENT;
  }
  alarmCycleCount++;
  return SLEEP_TIMER;
}

}  // namespace

// Exported: any user-facing input source calls this to reset the inactivity
// watchdog (and any in-progress alarm cycle).
void noteUserActivity() {
  lastActivityMs = millis();
  if (inAlarmCycle) {
    inAlarmCycle      = false;
    alarmCycleCount   = 0;
    alarmBurstStartMs = 0;
    lastAlarmBeepMs   = 0;
  }
}

// Exported for web_server.cpp — trigger test endpoint (/trigstate).
bool     webServerGetTrigPressed() { return trigCurrent; }
uint32_t webServerGetTrigEvents()  { return trigTestEventCount; }

void setup() {
  // Detect alarm-resume wake (1-hour timer expiry mid-alarm-cycle).
  esp_sleep_wakeup_cause_t wakeCause = esp_sleep_get_wakeup_cause();
  bootIsAlarmResume = (wakeCause == ESP_SLEEP_WAKEUP_TIMER) &&
                      inAlarmCycle &&
                      (alarmCycleCount < ALARM_MAX_CYCLES);
  if (!bootIsAlarmResume) {
    inAlarmCycle    = false;
    alarmCycleCount = 0;
  }

  Serial.begin(115200);
  delay(50);
  LOG("\n%s %s (%s)%s\n", FW_NAME, FW_VERSION, FW_VARIANT,
      bootIsAlarmResume ? " [alarm resume]" : "");

  // I/O
  pinMode(PIN_TRIG, INPUT_PULLUP);

  storageBegin();
  uint8_t lastSlot = storageGetLastSlot();
  storageLoadSlot(lastSlot, activeCfg);

  displayInit();
  displaySetSlot(lastSlot, activeCfg.solenoidCount);

  buzzerBegin();
  firingBegin(activeCfg);
  selectorBegin(activeCfg);
  wifiManagerBegin();

  rs.activeSlot = lastSlot;
  rs.sysState   = SYS_READY;
  rs.currentMode = selectorReadMode(activeCfg);

  if (!bootIsAlarmResume) {
    buzzerPlay(BUZZ_BOOT);
    if (storageWasFirstBoot()) {
      // First-ever boot (NVS just initialized) — follow BUZZ_BOOT with BUZZ_READY
      // so the user can recognize a fresh unit from a normal power-up.
      uint32_t t = millis();
      while (buzzerBusy() && (uint32_t)(millis() - t) < 800) { buzzerUpdate(); }
      delay(200);
      buzzerPlay(BUZZ_READY);
    }
  }
  shotsAtLastPersist = 0;
  lastShotPersistMs  = millis();

  LOG("Active slot: %u (%s)\n", lastSlot, activeCfg.name);

  // ── Boot WiFi gesture: hold trigger for 5s within first 5s ───────────────
  // Skipped on alarm resume — no user is present.
  if (!bootIsAlarmResume) {
    const uint32_t BOOT_WINDOW_MS = 5000;
    const uint32_t BOOT_HOLD_MS   = 5000;
    uint32_t windowStart = millis();
    bool triggerHeldAtBoot = false;
    uint32_t trigHoldStart = 0;

    while ((uint32_t)(millis() - windowStart) < BOOT_WINDOW_MS) {
      bool trigPressed = (digitalRead(PIN_TRIG) == LOW);

      if (trigPressed && !triggerHeldAtBoot) {
        triggerHeldAtBoot = true;
        trigHoldStart = millis();
      } else if (!trigPressed) {
        triggerHeldAtBoot = false;
        trigHoldStart = 0;
      }

      if (triggerHeldAtBoot &&
          (uint32_t)(millis() - trigHoldStart) >= BOOT_HOLD_MS) {
        wifiStart();
        buzzerPlayCount(3);
        uint32_t buzzerWait = millis();
        while (buzzerBusy() && (uint32_t)(millis() - buzzerWait) < 1500) {
          buzzerUpdate();
        }
        break;
      }
      buzzerUpdate();
    }
  }

  if (!bootIsAlarmResume) {
    lastActivityMs = millis();
  } else {
    alarmBurstStartMs = millis();
    lastAlarmBeepMs   = 0;
  }

#ifdef WIFI_AUTOSTART_AT_BOOT
  if (!bootIsAlarmResume) {
    LOGLN("WIFI_AUTOSTART_AT_BOOT defined -> starting AP now");
    wifiStart();
  }
#endif
}

void loop() {
  // 1) Trigger input
  if (activeCfg.trigMode == SWITCH_HALL) {
    serviceTriggerHallPoll();
  } else {
    serviceTrigger();
  }

  // 2) Selector — track changes for buzzer feedback. Mode change counts as
  // user activity for the inactivity watchdog.
  static FireMode lastMode = FIRE_SAFE;
  FireMode mode = selectorReadMode(activeCfg);
  if (mode != lastMode) {
    lastMode = mode;
    rs.currentMode = mode;
    if (!activeCfg.silentMode) buzzerPlay(BUZZ_MODE_CHANGE);
    noteUserActivity();
  }

  // 3) Firing engine
  firingUpdate(activeCfg, mode);

  // 4) Buzzer player
  buzzerUpdate();

  // 5) WiFi (handles DNS + HTTP when active)
  wifiManagerUpdate();

  // 6) MOSFET diagnostic test scheduler (web /test → 2s pulse)
  mosTestService();

  // 7) Persist shot counter occasionally
  persistShotsIfNeeded();

  // 8) Inactivity alarm watchdog. After INACTIVITY_TIMEOUT_MS of no user
  // input, the FCU starts a 6-cycle alarm: 5 min of beeps every 30 s, then
  // 1 h of timer-deep-sleep, repeat. After the last cycle we deep-sleep
  // permanently (only a trigger pull wakes the MCU, via full reboot).
  AlarmSleepCmd sleepCmd = serviceInactivityAlarm();
  if (sleepCmd != SLEEP_NONE) {
    if (wifiActive()) wifiStop();
    buzzerStop();
    displaySleep();
    esp_deep_sleep_enable_gpio_wakeup(1ULL << PIN_TRIG, ESP_GPIO_WAKEUP_GPIO_LOW);
    if (sleepCmd == SLEEP_TIMER) {
      esp_sleep_enable_timer_wakeup((uint64_t)ALARM_SLEEP_INTERVAL_MS * 1000ULL);
    }
    esp_deep_sleep_start();
    // Never returns.
  }
}
