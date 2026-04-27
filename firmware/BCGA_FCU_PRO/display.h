// BCGA FCU PRO — display.h
// Integrated 0.42" OLED status screen (SSD1306 72×40, I²C, on the ESP32-C3
// SuperMini module itself). Three read-only lines: slot + engine type, WiFi
// state, AP IP. No menus, no input. Redraw only on state change.

#pragma once

#include <Arduino.h>
#include <IPAddress.h>

void displayInit();
void displaySetSlot(uint8_t slotIdx, uint8_t solenoidCount);  // 1 = S8PA, 2 = D8PA
void displayWifiUp(const IPAddress& ip);
void displayWifiDown();
void displaySleep();   // Blank + power-save; call before esp_deep_sleep_start().
