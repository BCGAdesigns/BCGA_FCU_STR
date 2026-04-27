// BCGA FCU STR — storage.cpp

#include "storage.h"
#include "config.h"
#include <Preferences.h>

namespace {
Preferences prefs;
const char* NS = "bcgafcu";
// Bump when defaults or schema change. Mismatch on boot → wipe namespace and
// reinit, so users get the new defaults without manual factory-reset.
const uint8_t STORAGE_INIT_VERSION = 8;
bool firstBootFlag = false;
}

static void buildKey(char* out, const char* prefix, uint8_t idx) {
  snprintf(out, 12, "%s%u", prefix, (unsigned)idx);
}

void storageBegin() {
  prefs.begin(NS, false);
  uint8_t initVer = prefs.getUChar("init", 0);
  if (initVer != STORAGE_INIT_VERSION) {
    firstBootFlag = true;
    prefs.clear();
    for (uint8_t i = 0; i < SLOT_COUNT; i++) {
      SlotConfig c;
      storageDefaultSlot(i, c);
      storageSaveSlot(i, c);
    }
    prefs.putUChar("lang", LANG_BR);
    prefs.putUChar("lastSlot", 0);
    prefs.putString("wifiPwd", WIFI_PASS_DEFAULT);
    prefs.putULong("shots", 0);
    prefs.putUChar("init", STORAGE_INIT_VERSION);
  }
}

bool storageWasFirstBoot() { return firstBootFlag; }

void storageDefaultSlot(uint8_t idx, SlotConfig& out) {
  memset(&out, 0, sizeof(out));
  out.version       = SLOT_CONFIG_VERSION;
  snprintf(out.name, sizeof(out.name), "Slot %u", (unsigned)(idx + 1));
  out.solenoidCount = DEFAULT_SOLENOIDS;
  out.trigMode      = SWITCH_DIGITAL;
  out.selMode       = SWITCH_DIGITAL;
  out.sel3pos       = 0;                   // 2-pos selector by default
  out.selPos1Mode   = FIRE_SEMI;
  out.selPos2Mode   = FIRE_FULL;
  out.selPos3Mode   = FIRE_SAFE;           // only used when sel3pos == 1
  out.dn            = DEFAULT_DN_MS;
  out.dr            = DEFAULT_DR_MS;
  out.dp            = DEFAULT_DP_MS;
  out.db            = DEFAULT_DB_UNITS;
  out.rofLimit      = DEFAULT_ROF_LIMIT;
  out.semiRofMs     = DEFAULT_SEMI_ROF_MS;   // 0 = disabled
  out.hallTrigLow   = 1500;
  out.hallTrigHigh  = 2500;
  out.hallSelLow1   = 1365;                // ~1/3 for 3-pos default; midpoint when 2-pos
  out.hallSelLow2   = 2730;                // ~2/3 (only used when sel3pos == 1)
  out.mosfetSwap    = 0;
  out.invertTrig    = 0;
  out.silentMode    = 0;
}

bool storageLoadSlot(uint8_t idx, SlotConfig& out) {
  if (idx >= SLOT_COUNT) return false;
  char key[12]; buildKey(key, "s", idx);
  size_t sz = prefs.getBytesLength(key);
  if (sz != sizeof(SlotConfig)) {
    storageDefaultSlot(idx, out);
    return false;
  }
  prefs.getBytes(key, &out, sizeof(SlotConfig));
  if (out.version != SLOT_CONFIG_VERSION) {
    storageDefaultSlot(idx, out);
    storageSaveSlot(idx, out);
  }
  return true;
}

bool storageSaveSlot(uint8_t idx, const SlotConfig& cfg) {
  if (idx >= SLOT_COUNT) return false;
  char key[12]; buildKey(key, "s", idx);
  return prefs.putBytes(key, &cfg, sizeof(SlotConfig)) == sizeof(SlotConfig);
}

uint8_t storageGetLastSlot() {
  uint8_t v = prefs.getUChar("lastSlot", 0);
  return v < SLOT_COUNT ? v : 0;
}

void storageSetLastSlot(uint8_t idx) {
  if (idx < SLOT_COUNT) prefs.putUChar("lastSlot", idx);
}

Language storageGetLang() {
  uint8_t v = prefs.getUChar("lang", LANG_BR);
  return v == LANG_EN ? LANG_EN : LANG_BR;
}

void storageSetLang(Language l) { prefs.putUChar("lang", (uint8_t)l); }

void storageGetWifiPass(char* out, size_t outLen) {
  String s = prefs.getString("wifiPwd", WIFI_PASS_DEFAULT);
  strncpy(out, s.c_str(), outLen - 1);
  out[outLen - 1] = '\0';
}

bool storageSetWifiPass(const char* pwd) {
  if (!pwd || strlen(pwd) < WIFI_PASS_MIN_LEN) return false;
  prefs.putString("wifiPwd", pwd);
  return true;
}

uint32_t storageGetTotalShots() { return prefs.getULong("shots", 0); }

void storageBumpShots(uint32_t delta) {
  if (!delta) return;
  uint32_t cur = prefs.getULong("shots", 0);
  prefs.putULong("shots", cur + delta);
}

void storageFactoryReset() {
  prefs.clear();
  storageBegin();
}
