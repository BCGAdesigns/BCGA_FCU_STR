// BCGA FCU V2 — buzzer.h
// Non-blocking pattern player on PIN_BUZZER (LEDC PWM).

#pragma once

#include <Arduino.h>

enum BuzzPattern : uint8_t {
  BUZZ_NONE = 0,
  BUZZ_BOOT,            // 1: short rising chirp at startup
  BUZZ_READY,           // 2: short single beep — ready
  BUZZ_SLOT,            // 3: N chirps == active slot number
  BUZZ_MODE_CHANGE,     // 4: 2 quick beeps
  BUZZ_SAVE_OK,         // 5: short ascending pair
  BUZZ_ERROR,           // 6: long low buzz
  BUZZ_LOW_BATT,        // 7: triple descending warning
  BUZZ_BATT_CUT,        // 8: long sad tone before kill
  BUZZ_WIFI_ON,         // 9: rising arpeggio
  BUZZ_WIFI_OFF,        // 10: falling arpeggio
  BUZZ_TEST             // 11: 1s steady tone (web "Test buzzer" button)
};

void buzzerBegin();
void buzzerPlay(BuzzPattern p);
void buzzerPlayCount(uint8_t n);   // n short chirps (used for slot announce)
void buzzerStop();
void buzzerUpdate();               // call from loop()
bool buzzerBusy();
