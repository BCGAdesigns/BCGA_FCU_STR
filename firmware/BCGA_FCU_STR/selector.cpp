// BCGA FCU V2 — selector.cpp
// Reads selector and maps to FireMode. Default is 2-position; 3-position
// requires sel3pos == 1 AND selMode == HALL (digital can only express 2).

#include "selector.h"
#include "config.h"

void selectorBegin(const SlotConfig& cfg) {
  if (cfg.selMode == SWITCH_DIGITAL) {
    pinMode(PIN_SEL, INPUT_PULLUP);
  } else {
    pinMode(PIN_SEL, INPUT);
    analogReadResolution(12);
  }
}

void selectorReconfig(const SlotConfig& cfg) {
  selectorBegin(cfg);
}

SelectorPos selectorReadPos(const SlotConfig& cfg) {
  if (cfg.selMode == SWITCH_DIGITAL) {
    // Digital is 2-position only: LOW = pos 2, HIGH = pos 1
    return digitalRead(PIN_SEL) == LOW ? SEL_POS_2 : SEL_POS_1;
  }
  // Hall analog
  uint16_t a = analogRead(PIN_SEL);
  if (cfg.sel3pos) {
    if (a < cfg.hallSelLow1) return SEL_POS_1;
    if (a < cfg.hallSelLow2) return SEL_POS_2;
    return SEL_POS_3;
  }
  // 2-position Hall: hallSelLow1 is the midpoint
  return (a < cfg.hallSelLow1) ? SEL_POS_1 : SEL_POS_2;
}

FireMode selectorReadMode(const SlotConfig& cfg) {
  SelectorPos p = selectorReadPos(cfg);
  uint8_t m;
  switch (p) {
    case SEL_POS_1: m = cfg.selPos1Mode; break;
    case SEL_POS_2: m = cfg.selPos2Mode; break;
    default:        m = cfg.selPos3Mode; break;
  }
  if (m > FIRE_BURST4) m = FIRE_SAFE;
  return (FireMode)m;
}
