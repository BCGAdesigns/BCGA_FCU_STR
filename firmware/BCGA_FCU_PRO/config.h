// BCGA FCU PRO — config.h
// Pin map, hardware constants, defaults.
// Target: ESP32-C3 SuperMini, BCGA FCU PRO board (battery sense, WiFi button,
// kill latch, dual MOSFET).

#pragma once

#include <Arduino.h>

// ============================================================================
// FIRMWARE METADATA
// ============================================================================
#define FW_NAME         "BCGA FCU PRO"
#define FW_VERSION      "2.0.0"
#define FW_VARIANT      "Pro"
#define FW_VARIANT_FULL "BCGA_FCU_PRO"

// ============================================================================
// PIN MAP — ESP32-C3 SuperMini
// ============================================================================
// Inputs
#define PIN_TRIG     0   // Trigger (Switch digital OR Hall analog)
#define PIN_SEL      1   // Selector (Switch digital OR Hall analog)
#define PIN_VBAT     2   // Battery voltage divider (ADC1_CH2)
#define PIN_WIFI_BTN 3   // WiFi enable momentary button (active LOW, INPUT_PULLUP)

// Outputs
#define PIN_MOS_1    8   // MOSFET 1 — SOL1 / Poppet (also onboard LED, inverted)
#define PIN_MOS_2    7   // MOSFET 2 — SOL2 / Nozzle
#define PIN_LATCH    4   // Self-kill latch (HIGH=alive, LOW=cut)
#define PIN_BUZZER  10   // 3V3 piezo buzzer (MLT-5020)

// Reserved (do not use)
// GPIO 5, 6  — reserved for future OLED (I2C SDA/SCL)
// GPIO 9     — boot strap (avoid)
// GPIO 18,19 — USB D+/D- (avoid)
// GPIO 20,21 — UART0 (logging)

// ============================================================================
// LEDC (PWM) CHANNELS
// ============================================================================
#define LEDC_BUZZER_CH    0
#define LEDC_BUZZER_RES   8     // 8-bit duty
#define LEDC_BUZZER_DEFAULT_FREQ 2700

// ============================================================================
// FIRING TIMING — milliseconds
// ============================================================================
#define FIRE_MIN_MS         2   // 2 ms minimum any pulse
#define FIRE_MAX_MS        80   // 80 ms maximum any pulse

// Defaults if NVS empty (milliseconds)
// Cycle order:
//   D8PA: pulse SOL2 (DN) → wait DR → pulse SOL1 (DP) → wait DL → repeat
//   S8PA: pulse SOL1 (DP) → wait DR → repeat
#define DEFAULT_DN_MS      18   // Nozzle Dwell — SOL2 pulse (D8PA only)
#define DEFAULT_DR_MS      26   // D8PA: seal wait. S8PA: inter-shot rest (~20).
#define DEFAULT_DP_MS      25   // Shot Poppet — SOL1 pulse
#define DEFAULT_DL_MS      10   // Post-shot delay (D8PA only)
#define DEFAULT_ROF_LIMIT   0   // 0 = unlimited
#define DEFAULT_SOLENOIDS   2   // 1 = S8PA, 2 = D8PA

// ============================================================================
// SLOTS
// ============================================================================
#define SLOT_COUNT 3            // 3 stored slots; selector picks 1..3

// ============================================================================
// BATTERY
// ============================================================================
// Voltage divider: VBAT --[R1=100k]--+--[R2=33k]-- GND, tap at PIN_VBAT
// Vadc = Vbat * R2/(R1+R2) = Vbat * 0.248
#define VBAT_DIV_NUM   33
#define VBAT_DIV_DEN   133
#define ADC_REF_MV    3300
#define ADC_RESOLUTION 4095   // 12-bit
// Thresholds (mV per cell)
#define CELL_NOMINAL_MV  3700
#define CELL_WARN_MV     3500
#define CELL_CUT_MV      3200
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
