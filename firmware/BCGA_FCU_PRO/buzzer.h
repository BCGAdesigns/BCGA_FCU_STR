// BCGA FCU PRO — buzzer.h
// Non-blocking pattern player on PIN_BUZZER. V2.1 hardware uses an active
// magnetic buzzer (FUET-9650B-3V, 3 kHz oscillator built in) — the GPIO is
// driven on/off with digitalWrite; patterns are count/duration based.

#pragma once

#include <Arduino.h>

enum BuzzPattern : uint8_t {
  BUZZ_NONE = 0,
  BUZZ_BOOT,            // 1: 1 short beep at startup
  BUZZ_READY,           // 2: 1 long beep — ready (first-boot only)
  BUZZ_SLOT,            // 3: N beeps == active slot number
  BUZZ_MODE_CHANGE,     // 4: 1 short click
  BUZZ_SAVE_OK,         // 5: tick-tock pair
  BUZZ_ERROR,           // 6: 1 sustained beep
  BUZZ_LOW_BATT,        // 7: 3 evenly-spaced beeps
  BUZZ_BATT_CUT,        // 8: 3 long beeps + terminal long beep
  BUZZ_WIFI_ON,         // 9: 4 short beeps
  BUZZ_WIFI_OFF,        // 10: short + long descending pair
  BUZZ_TEST,            // 11: 1 s steady tone (web "Test buzzer" button)
  BUZZ_INACTIVITY_ALERT // 12: 1 short beep — fires every 30 s during inactivity alarm
};

void buzzerBegin();
void buzzerPlay(BuzzPattern p);
void buzzerPlayCount(uint8_t n);   // n short chirps (used for slot announce)
void buzzerStop();
void buzzerUpdate();               // call from loop()
bool buzzerBusy();
