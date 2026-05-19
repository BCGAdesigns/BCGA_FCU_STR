// BCGA FCU V2 — storage.h
// Wraps Preferences (NVS) for SlotConfig, language, and password.

#pragma once

#include "types.h"

void          storageBegin();
bool          storageWasFirstBoot();    // true only on the boot that initialized NVS

bool          storageLoadSlot(uint8_t idx, SlotConfig& out);
bool          storageSaveSlot(uint8_t idx, const SlotConfig& cfg);
void          storageDefaultSlot(uint8_t idx, SlotConfig& out);

uint8_t       storageGetLastSlot();
void          storageSetLastSlot(uint8_t idx);

Language      storageGetLang();
void          storageSetLang(Language l);

void          storageGetWifiPass(char* out, size_t outLen);
bool          storageSetWifiPass(const char* pwd);    // false if too short

uint32_t      storageGetTotalShots();
void          storageBumpShots(uint32_t delta);

// Config LOCK — single device-wide password that gates slot save/switch/reset.
// Empty password = lock disabled. cfgLocked is the persisted boot state; the
// runtime "session" unlock is held by web_server.cpp.
bool          storageHasCfgPwd();
void          storageGetCfgPwd(char* out, size_t outLen);
bool          storageSetCfgPwd(const char* pwd);      // empty string clears
bool          storageGetCfgLocked();
void          storageSetCfgLocked(bool locked);

void          storageFactoryReset();
