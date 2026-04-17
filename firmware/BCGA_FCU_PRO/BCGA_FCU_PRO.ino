// BCGA FCU PRO — main sketch
// Target: ESP32-C3 SuperMini, Arduino-ESP32 core.
// Variant: PRO (battery sense, WiFi button, kill latch, dual MOSFET).
//
// Core loop is fully non-blocking: firing state machine, buzzer player,
// WiFi manager, web server, and battery poll all share the same loop().

#include "config.h"
#include "types.h"
#include "selector.h"
#include "buzzer.h"
#include "firing.h"
#include "storage.h"
#include "battery.h"
#include "wifi_manager.h"
#include "web_server.h"

namespace {

SlotConfig    activeCfg;
RuntimeState  rs;

// Trigger debounce / edge tracking
bool      trigCurrent  = false;     // logical: true == pressed
bool      trigLastRaw  = false;
uint32_t  trigDebounceUntilMs = 0;

uint32_t  shotsAtLastPersist = 0;
uint32_t  lastShotPersistMs  = 0;

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
    // invertTrig=1 flips the polarity for odd-wired builds.
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
  FireMode mode = selectorReadMode(activeCfg);

  if (!trigCurrent) {
    // released — for SAFE in WiFi-gesture path, count pulls
    if (mode == FIRE_SAFE) wifiNoteTriggerPull();
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

void serviceBattery() {
  batteryUpdate();
  bool low = batteryLow();
  bool cut = batteryCut();
  if (low && !rs.battLow) {
    rs.battLow = true;
    if (!activeCfg.silentMode) buzzerPlay(BUZZ_LOW_BATT);
    rs.sysState = SYS_LOW_BATT;
  } else if (!low) {
    rs.battLow = false;
  }
  if (cut && !rs.battCut) {
    rs.battCut = true;
    rs.sysState = SYS_CUT;
    firingForceStop();
    buzzerPlay(BUZZ_BATT_CUT);
    // give buzzer time to play before cutting power
    uint32_t deadline = millis() + 1500;
    while (millis() < deadline) { buzzerUpdate(); }
    batteryKillLatch();
  }
}

}  // namespace

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
  batteryBegin();
  wifiManagerBegin();

  rs.activeSlot = lastSlot;
  rs.sysState   = SYS_READY;
  rs.currentMode = selectorReadMode(activeCfg);

  buzzerPlay(BUZZ_BOOT);
  shotsAtLastPersist = 0;
  lastShotPersistMs  = millis();

  LOG("Active slot: %u (%s)\n", lastSlot, activeCfg.name);

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

  // 5) Battery
  serviceBattery();

  // 6) WiFi (handles DNS + HTTP when active)
  wifiManagerUpdate();

  // 7) MOSFET diagnostic test scheduler (web /test → 2s pulse)
  mosTestService();

  // 8) Persist shot counter occasionally
  persistShotsIfNeeded();
}
