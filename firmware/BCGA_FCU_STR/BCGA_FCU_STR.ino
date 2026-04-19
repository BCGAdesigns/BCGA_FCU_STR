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

#include <esp_sleep.h>

namespace {

SlotConfig    activeCfg;
RuntimeState  rs;

// Trigger debounce / edge tracking
bool      trigCurrent  = false;     // logical: true == pressed
bool      trigLastRaw  = false;
uint32_t  trigDebounceUntilMs = 0;

uint32_t  shotsAtLastPersist = 0;
uint32_t  lastShotPersistMs  = 0;

// Activity tracking for deep-sleep watchdog and /trigstate endpoint
uint32_t  lastTriggerActivityMs = 0;
uint32_t  trigTestEventCount    = 0;   // increments on every logical edge

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
  lastTriggerActivityMs = now;
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

}  // namespace

// Exported for web_server.cpp — trigger test endpoint (/trigstate).
bool     webServerGetTrigPressed() { return trigCurrent; }
uint32_t webServerGetTrigEvents()  { return trigTestEventCount; }

void setup() {
  Serial.begin(115200);
  delay(50);
  LOG("\n%s %s (%s)\n", FW_NAME, FW_VERSION, FW_VARIANT);

  // I/O
  pinMode(PIN_TRIG, INPUT_PULLUP);

  storageBegin();
  uint8_t lastSlot = storageGetLastSlot();
  storageLoadSlot(lastSlot, activeCfg);

  buzzerBegin();
  firingBegin(activeCfg);
  selectorBegin(activeCfg);
  wifiManagerBegin();

  rs.activeSlot = lastSlot;
  rs.sysState   = SYS_READY;
  rs.currentMode = selectorReadMode(activeCfg);

  buzzerPlay(BUZZ_BOOT);
  if (storageWasFirstBoot()) {
    // First-ever boot (NVS just initialized) — follow BUZZ_BOOT with BUZZ_READY
    // so the user can recognize a fresh unit from a normal power-up.
    uint32_t t = millis();
    while (buzzerBusy() && (uint32_t)(millis() - t) < 800) { buzzerUpdate(); }
    delay(200);
    buzzerPlay(BUZZ_READY);
  }
  shotsAtLastPersist = 0;
  lastShotPersistMs  = millis();

  LOG("Active slot: %u (%s)\n", lastSlot, activeCfg.name);

  // ── Boot WiFi gesture: hold trigger for 5s within first 5s ───────────────
  {
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

  lastTriggerActivityMs = millis();

#ifdef WIFI_AUTOSTART_AT_BOOT
  LOGLN("WIFI_AUTOSTART_AT_BOOT defined -> starting AP now");
  wifiStart();
#endif
}

void loop() {
  // 1) Trigger input
  if (activeCfg.trigMode == SWITCH_HALL) {
    serviceTriggerHallPoll();
  } else {
    serviceTrigger();
  }

  // 2) Selector — track changes for buzzer feedback
  static FireMode lastMode = FIRE_SAFE;
  FireMode mode = selectorReadMode(activeCfg);
  if (mode != lastMode) {
    lastMode = mode;
    rs.currentMode = mode;
    if (!activeCfg.silentMode) buzzerPlay(BUZZ_MODE_CHANGE);
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

  // 8) Deep-sleep watchdog — go to sleep if trigger has been idle for 60 min.
  // Waking source: PIN_TRIG LOW. The first trigger pull wakes the MCU
  // (full reboot); the second actually fires. ESP32-C3 uses the GPIO
  // deep-sleep wakeup path (ext0/ext1 don't exist on RISC-V parts).
  if ((uint32_t)(millis() - lastTriggerActivityMs) >= DEEP_SLEEP_TIMEOUT_MS) {
    if (wifiActive()) wifiStop();
    buzzerStop();
    esp_deep_sleep_enable_gpio_wakeup(1ULL << PIN_TRIG, ESP_GPIO_WAKEUP_GPIO_LOW);
    esp_deep_sleep_start();
    // Never returns.
  }
}
