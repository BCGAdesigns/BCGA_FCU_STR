// BCGA FCU V2 — storage.h
// Wraps Preferences (NVS) for SlotConfig, language, and password.

#pragma once

#include "types.h"

void          storageBegin();

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

void          storageFactoryReset();
