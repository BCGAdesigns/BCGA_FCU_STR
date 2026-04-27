// BCGA FCU STR — config.h
// Pin map, hardware constants, defaults for Starter (perfboard) variant.
// Target: ESP32-C3 SuperMini. Supports both S8PA (single) and D8PA (dual).

#pragma once

#include <Arduino.h>

// ============================================================================
// FIRMWARE METADATA
// ============================================================================
#define FW_NAME         "BCGA FCU STR"
#define FW_VERSION      "2.1.0"
#define FW_VARIANT      "Starter"
#define FW_VARIANT_FULL "BCGA_FCU_STR"

// ============================================================================
// PIN MAP — ESP32-C3 SuperMini (Starter build)
// ============================================================================
// Inputs
#define PIN_TRIG     0   // Trigger (Switch digital OR Hall analog)
#define PIN_SEL      1   // Selector (Switch digital OR Hall analog)

// Outputs
#define PIN_MOS_1    8   // MOSFET 1 — SOL1 / Poppet
#define PIN_MOS_2    7   // MOSFET 2 — SOL2 / Nozzle (D8PA)
#define PIN_BUZZER  10   // 3V3 piezo buzzer (MLT-5020)

// Integrated 0.42" OLED (on the ESP32-C3 SuperMini module itself)
// SSD1306, 72×40 visible px, I²C addr 0x3C, ~30-col offset to glass.
#define PIN_OLED_SDA 5
#define PIN_OLED_SCL 6

// Reserved (do not use)
// GPIO 2, 3, 4   — free on perfboard but unused in Starter firmware
// GPIO 9         — boot strap (avoid)
// GPIO 18,19     — USB D+/D- (avoid)
// GPIO 20,21     — UART0 (logging)

// ============================================================================
// LEDC (PWM) CHANNELS
// ============================================================================
#define LEDC_BUZZER_CH    0
#define LEDC_BUZZER_RES   8     // 8-bit duty
#define LEDC_BUZZER_DEFAULT_FREQ 2700

// ============================================================================
// FIRING TIMING
// ============================================================================
// DN/DR/DP are in milliseconds, clamped FIRE_MIN_MS..FIRE_MAX_MS.
// DB (Trigger Debounce) is in units of 0.1 ms, clamped DB_MIN_UNITS..DB_MAX_UNITS.
// Cycle order:
//   D8PA: pulse SOL2 (DN) → wait DR → pulse SOL1 (DP) → wait DB → repeat
//   S8PA: pulse SOL1 (DP) → wait DR → repeat
#define FIRE_MIN_MS         2   // 2 ms minimum pulse for DN/DR/DP
#define FIRE_MAX_MS        80   // 80 ms maximum pulse for DN/DR/DP
#define DB_MIN_UNITS       20   //  2.0 ms minimum DB (1 unit = 0.1 ms)
#define DB_MAX_UNITS      800   // 80.0 ms maximum DB

// Defaults if NVS empty
#define DEFAULT_DN_MS      18   // Nozzle Dwell — SOL2 pulse (D8PA only)
#define DEFAULT_DR_MS      26   // D8PA: seal wait. S8PA: inter-shot rest (~20).
#define DEFAULT_DP_MS      80   // Shot Poppet — SOL1 pulse (first-boot starts at max; tune down on chrono)
#define DEFAULT_DB_UNITS  100   // Trigger Debounce — D8PA only (100 × 0.1 ms = 10 ms)
#define DEFAULT_ROF_LIMIT   0   // 0 = unlimited
#define DEFAULT_SOLENOIDS   2   // 1 = S8PA, 2 = D8PA
#define DEFAULT_SEMI_ROF_MS 0   // 0 = disabled; ms to ignore trigger after semi shot

// ============================================================================
// SLOTS
// ============================================================================
#define SLOT_COUNT 3            // 3 stored slots; selector picks 1..3

// ============================================================================
// TRIGGER DEBOUNCE / POLL
// ============================================================================
#define DEBOUNCE_MS         1
#define HALL_POLL_US      500   // analog poll period

// ============================================================================
// WIFI
// ============================================================================
#define WIFI_SSID_DEFAULT   FW_VARIANT_FULL   // BCGA_FCU_STR
#define WIFI_PASS_DEFAULT   "12345678"
#define WIFI_PASS_MIN_LEN   8

// MOSFET diagnostic test duration (single click pulses for this long)
#define MOS_TEST_DURATION_MS 2000
#define WIFI_AUTO_OFF_MS    (10UL * 60UL * 1000UL)   // 10 minutes

// Inactivity alarm timer. After this much idle time the FCU starts a 6-cycle
// alarm sequence (5 min of beeps + 1 h timer-sleep per cycle), then permanent
// deep sleep. Enable DEEP_SLEEP_DEBUG to shrink the idle timeout to 5 min for
// bench testing — leave commented out in production.
//#define DEEP_SLEEP_DEBUG
#ifdef DEEP_SLEEP_DEBUG
  #define INACTIVITY_TIMEOUT_MS (5UL  * 60UL * 1000UL) //  5 minutes (debug)
#else
  #define INACTIVITY_TIMEOUT_MS (60UL * 60UL * 1000UL) // 60 minutes (prod)
#endif

// Inactivity alarm sequence (constants below are NOT shrunk by DEEP_SLEEP_DEBUG
// — adjust here if you want shorter cycles for bench testing too).
#define ALARM_BURST_DURATION_MS  (5UL  * 60UL * 1000UL) // 5 min of beeps per cycle
#define ALARM_BEEP_INTERVAL_MS   (30UL        * 1000UL) // beep every 30 s
#define ALARM_SLEEP_INTERVAL_MS  (60UL * 60UL * 1000UL) // 1 h between cycles
#define ALARM_MAX_CYCLES         6                       // 6 cycles → ~6 h total
// Starter has no WiFi button — gesture (5 pulls in SAFE) brings up the AP.
#define WIFI_GESTURE_PULL_COUNT 5
#define WIFI_GESTURE_WINDOW_MS  3000
#define DNS_PORT            53
#define HTTP_PORT           80

// ============================================================================
// DEBUG
// ============================================================================
// Auto-start WiFi AP at boot. Comment out for normal operation (gesture-only).
#define WIFI_AUTOSTART_AT_BOOT

#define LOG_ENABLED 1
#if LOG_ENABLED
  #define LOG(...)    Serial.printf(__VA_ARGS__)
  #define LOGLN(s)    Serial.println(s)
#else
  #define LOG(...)    ((void)0)
  #define LOGLN(s)    ((void)0)
#endif
