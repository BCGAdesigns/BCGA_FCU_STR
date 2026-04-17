// BCGA FCU V2 — selector.h
// Reads the 3-position selector (digital or Hall) and maps to FireMode.

#pragma once

#include "types.h"

void     selectorBegin(const SlotConfig& cfg);
void     selectorReconfig(const SlotConfig& cfg);
SelectorPos selectorReadPos(const SlotConfig& cfg);
FireMode selectorReadMode(const SlotConfig& cfg);
