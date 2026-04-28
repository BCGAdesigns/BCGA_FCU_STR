// BCGA FCU PRO — config.h
// Pin map, hardware constants, defaults.
// Target: ESP32-C3 SuperMini, BCGA FCU PRO board (battery sense, WiFi button,
// dual MOSFET). V2.1 dropped the kill-latch circuit; battery protection now
// uses the firmware lockout mode (firing block + audible alarm).

#pragma once

#include <Arduino.h>

// ============================================================================
// FIRMWARE METADATA
// ============================================================================
#define FW_NAME         "BCGA FCU PRO"
#define FW_VERSION      "2.1.0"
#define FW_VARIANT      "Pro"
#define FW_VARIANT_FULL "BCGA_FCU_PRO"

// ============================================================================
// PIN MAP — ESP32-C3 SuperMini
// ============================================================================
// Inputs
#define PIN_TRIG     0   // Trigger (Switch digital OR Hall analog), ADC1_CH0
#define PIN_SEL      1   // Selector (Switch digital OR Hall analog), ADC1_CH1
#define PIN_WIFI_BTN 3   // WiFi enable momentary button (active LOW, INPUT_PULLUP)
#define PIN_VBAT     4   // Battery voltage divider (ADC1_CH4)

// Outputs
#define PIN_MOS_1    8   // MOSFET 1 — SOL1 / Poppet (also onboard LED, inverted)
#define PIN_MOS_2    7   // MOSFET 2 — SOL2 / Nozzle
#define PIN_BUZZER  10   // 3V3 piezo buzzer (MLT-5020)

// Integrated 0.42" OLED (on the ESP32-C3 SuperMini module itself)
// SSD1306, 72×40 visible px, I²C addr 0x3C, ~30-col offset to glass.
#define PIN_OLED_SDA 5
#define PIN_OLED_SCL 6

// Reserved (do not use)
// GPIO 9     — boot strap / BOOT button (avoid)
// GPIO 18,19 — USB D+/D- (avoid)
// GPIO 20    — UART0 RX (logging)
// GPIO 21    — free on V2.1 (was PIN_LATCH on V2.0; latch circuit removed)

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
// BATTERY
// ============================================================================
// Voltage divider (V2.1): VBAT --[R16=47k]--+--[R17=10k]-- GND, tap at PIN_VBAT.
// Zener clamp D4 (BZT52C3V3S) was removed in V2.1 — the larger top resistor
// keeps Vadc safely under 3.3 V across all 2S/3S operating ranges.
// Vadc = Vbat * R17/(R16+R17) = Vbat * 10/57
#define VBAT_DIV_NUM   10
#define VBAT_DIV_DEN   57
#define ADC_REF_MV    3300
#define ADC_RESOLUTION 4095   // 12-bit
// Thresholds (mV per cell)
#define CELL_NOMINAL_MV   3700
#define CELL_WARN_MV      3500   // LOW — 6 beeps/min
#define CELL_CRITICAL_MV  3200   // CRITICAL — 12 beeps/min
#define CELL_CUT_MV       3000   // CUT — enter lockout mode (firing block + alarm)
// Cell count detection: 2S (>5.5V & <9V) | 3S (>=9V)
#define V_2S_3S_BOUNDARY_MV 9000
#define V_MIN_VALID_MV      5500
// Sampling
#define BATTERY_SAMPLES     16
#define BATTERY_POLL_MS    500

// ============================================================================
// TRIGGER DEBOUNCE / POLL
// ============================================================================
#define DEBOUNCE_MS         1
#define HALL_POLL_US      500   // analog poll period

// ============================================================================
// WIFI
// ============================================================================
#define WIFI_SSID_DEFAULT   FW_VARIANT_FULL   // BCGA_FCU_PRO
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

// Battery lockout (PRO only): once cut threshold is hit, firing is blocked and
// a beep fires every LOCKOUT_BEEP_INTERVAL_MS. After LOCKOUT_MAX_BEEPS beeps
// (~30 min) the FCU enters permanent deep sleep to spare what's left of the pack.
#define LOCKOUT_BEEP_INTERVAL_MS (5UL  *        1000UL)  // beep every 5 s
#define LOCKOUT_MAX_BEEPS        360                     // 360 × 5 s ≈ 30 min
#define WIFI_GESTURE_PULL_COUNT 5
#define WIFI_GESTURE_WINDOW_MS  3000
#define DNS_PORT            53
#define HTTP_PORT           80

// ============================================================================
// DEBUG
// ============================================================================
// Auto-start WiFi AP at boot. Comment out for normal operation (button/gesture
// triggered). With this defined, plugging in power immediately brings up the
// AP — useful for benchtop development. The 10-min idle auto-off still applies.
#define WIFI_AUTOSTART_AT_BOOT

#define LOG_ENABLED 1
#if LOG_ENABLED
  #define LOG(...)    Serial.printf(__VA_ARGS__)
  #define LOGLN(s)    Serial.println(s)
#else
  #define LOG(...)    ((void)0)
  #define LOGLN(s)    ((void)0)
#endif
