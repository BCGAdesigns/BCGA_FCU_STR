// BCGA FCU STR — config.h
// Pin map, hardware constants, defaults for Starter (perfboard) variant.
// Target: ESP32-C3 SuperMini. S8PA only (single solenoid, poppet).

#pragma once

#include <Arduino.h>

// ============================================================================
// FIRMWARE METADATA
// ============================================================================
#define FW_NAME         "BCGA FCU STR"
#define FW_VERSION      "2.0.0"
#define FW_VARIANT      "Starter"
#define FW_VARIANT_FULL "BCGA_FCU_STR"

// ============================================================================
// PIN MAP — ESP32-C3 SuperMini (Starter build)
// ============================================================================
// Inputs
#define PIN_TRIG     0   // Trigger (Switch digital OR Hall analog)
#define PIN_SEL      1   // Selector (Switch digital OR Hall analog)

// Outputs
#define PIN_MOS_1    8   // MOSFET 1 — Poppet solenoid (only one on Starter)
#define PIN_BUZZER  10   // 3V3 piezo buzzer (MLT-5020)

// Reserved (do not use)
// GPIO 2, 3, 4, 7 — free on perfboard but unused in Starter firmware
// GPIO 5, 6       — reserved for future OLED (I2C SDA/SCL)
// GPIO 9          — boot strap (avoid)
// GPIO 18,19      — USB D+/D- (avoid)
// GPIO 20,21      — UART0 (logging)

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
// S8PA cycle: pulse SOL1 (DP) → wait DR → repeat
#define DEFAULT_DR_MS      20   // Inter-shot rest
#define DEFAULT_DP_MS      25   // Shot Poppet — SOL1 pulse
#define DEFAULT_ROF_LIMIT   0   // 0 = unlimited

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
