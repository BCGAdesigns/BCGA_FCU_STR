// BCGA FCU PRO — types.h
// Shared enums and the SlotConfig struct persisted to NVS.

#pragma once

#include <Arduino.h>

// ============================================================================
// ENUMS
// ============================================================================

enum SwitchMode : uint8_t {
  SWITCH_DIGITAL = 0,       // Mechanical microswitch
  SWITCH_HALL    = 1        // Hall-effect analog sensor
};

// FireMode embeds the burst count directly. Pos-N dropdown picks one of these.
enum FireMode : uint8_t {
  FIRE_SAFE   = 0,
  FIRE_SEMI   = 1,
  FIRE_FULL   = 2,          // continuous while held
  FIRE_BURST2 = 3,
  FIRE_BURST3 = 4,
  FIRE_BURST4 = 5
};

inline bool    isAutoMode(FireMode m)  { return m >= FIRE_FULL && m <= FIRE_BURST4; }
inline bool    isBurstMode(FireMode m) { return m >= FIRE_BURST2 && m <= FIRE_BURST4; }
inline uint8_t burstCountOf(FireMode m){ return isBurstMode(m) ? (uint8_t)(m - FIRE_BURST2 + 2) : 1; }

enum SystemState : uint8_t {
  SYS_BOOT = 0,
  SYS_READY,
  SYS_FIRING,
  SYS_WIFI_ON,
  SYS_LOW_BATT,
  SYS_CUT          // Battery latch killed
};

// Selector position. With sel3pos=0 only POS_1 and POS_2 are reachable.
enum SelectorPos : uint8_t {
  SEL_POS_1 = 0,
  SEL_POS_2 = 1,
  SEL_POS_3 = 2
};

enum Language : uint8_t {
  LANG_BR = 0,
  LANG_EN = 1
};

// ============================================================================
// SLOT CONFIG — persisted to NVS via putBytes/getBytes
// ============================================================================
// Bump CONFIG_VERSION when the layout/semantics change; storage will wipe.
#define SLOT_CONFIG_VERSION 5

struct __attribute__((packed)) SlotConfig {
  uint8_t  version;          // SLOT_CONFIG_VERSION
  char     name[16];         // UTF-8 short name

  uint8_t  solenoidCount;    // 1 = S8PA (single), 2 = D8PA (dual)
  uint8_t  trigMode;         // SwitchMode
  uint8_t  selMode;          // SwitchMode
  uint8_t  sel3pos;          // 1 = enable 3rd selector position (Hall only)

  // Selector → fire mode mapping per position. FireMode 0..5.
  // Pos 3 only used when sel3pos == 1.
  uint8_t  selPos1Mode;
  uint8_t  selPos2Mode;
  uint8_t  selPos3Mode;

  // Timing — milliseconds, clamped FIRE_MIN_MS..FIRE_MAX_MS
  // Cycle order:
  //   D8PA: pulse SOL2 (DN) → wait DR → pulse SOL1 (DP) → wait DL → repeat
  //   S8PA: pulse SOL1 (DP) → wait DR → repeat (DN and DL unused)
  uint16_t dn;               // Nozzle Dwell — SOL2 pulse (D8PA only)
  uint16_t dr;               // D8PA: seal wait between DN and DP. S8PA: inter-shot rest.
  uint16_t dp;               // Shot Poppet — SOL1 pulse (both modes)
  uint16_t dl;               // Post-shot Delay — D8PA only (BB exits barrel)

  uint16_t rofLimit;         // rounds/sec cap, 0 = unlimited
  uint16_t semiRofMs;        // ms to ignore trigger after semi shot (0 = disabled)

  // Hall thresholds (ADC counts 0..4095)
  uint16_t hallTrigLow;      // trig below this = released
  uint16_t hallTrigHigh;     // trig above this = pressed (also "fire point")
  uint16_t hallSelLow1;      // pos 1 upper bound (always used in Hall)
  uint16_t hallSelLow2;      // pos 2 upper bound (only used when sel3pos == 1)

  // Flags
  uint8_t  mosfetSwap;       // 1 = swap PIN_MOS_1 ↔ PIN_MOS_2 logical roles (D8PA wiring fix)
  uint8_t  invertTrig;       // 1 = trigger active-LOW (microswitch default)
  uint8_t  silentMode;       // 1 = skip buzzer confirmations during firing

  uint8_t  _reserved[6];
};

static_assert(sizeof(SlotConfig) <= 64, "SlotConfig should fit in one NVS blob comfortably");

// ============================================================================
// RUNTIME STATE (not persisted)
// ============================================================================

struct RuntimeState {
  SystemState sysState;
  uint8_t     activeSlot;       // 0..SLOT_COUNT-1
  FireMode    currentMode;
  uint16_t    shotsThisBurst;
  uint32_t    lastShotMicros;
  bool        wifiActive;
  uint32_t    wifiStartMs;
  uint16_t    vbatMv;
  uint8_t     cellCount;        // 2 or 3
  bool        battLow;
  bool        battCut;
};
