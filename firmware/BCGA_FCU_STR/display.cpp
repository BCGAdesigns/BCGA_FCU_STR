// BCGA FCU STR — display.cpp

#include "display.h"
#include "config.h"

#include <U8g2lib.h>
#include <Wire.h>

namespace {

// ER-OLED0.42 style panel — 72×40 visible, SSD1306, I²C @ 0x3C. Requires
// U8g2 ≥ 2.34 for this constructor. On older pinned versions, swap to
// U8G2_SSD1306_128X64_NONAME_F_HW_I2C and add a manual 30-column offset to
// every drawStr call.
U8G2_SSD1306_72X40_ER_F_HW_I2C oled(U8G2_R0, /*reset*/ U8X8_PIN_NONE);

bool      ready        = false;

uint8_t   lastSlotIdx  = 0xFF;
uint8_t   lastSolCount = 0xFF;
bool      lastWifiOn   = false;
IPAddress lastIp;

char slotLine[16] = "";
char wifiLine[16] = "";
char ipLine[16]   = "";

void render() {
  if (!ready) return;
  oled.clearBuffer();
  oled.setFont(u8g2_font_6x10_tr);
  oled.drawStr(0,  9, slotLine);
  oled.drawStr(0, 21, wifiLine);
  if (ipLine[0]) oled.drawStr(0, 33, ipLine);
  oled.sendBuffer();
}

void formatSlotLine(uint8_t idx, uint8_t solCount) {
  const char* type = (solCount == 2) ? "D8PA" : "S8PA";
  snprintf(slotLine, sizeof(slotLine), "Slot %u %s", (unsigned)(idx + 1), type);
}

}  // namespace

void displayInit() {
  Wire.begin(PIN_OLED_SDA, PIN_OLED_SCL);
  Wire.setClock(400000);
  ready = oled.begin();
  if (!ready) return;
  oled.setContrast(255);
  // Neutral boot frame — callers immediately replace both lines.
  strcpy(slotLine, "Slot 1 ----");
  strcpy(wifiLine, "WiFi OFF");
  ipLine[0] = '\0';
  render();
}

void displaySetSlot(uint8_t slotIdx, uint8_t solCount) {
  if (!ready) return;
  if (slotIdx == lastSlotIdx && solCount == lastSolCount) return;
  lastSlotIdx  = slotIdx;
  lastSolCount = solCount;
  formatSlotLine(slotIdx, solCount);
  render();
}

void displayWifiUp(const IPAddress& ip) {
  if (!ready) return;
  if (lastWifiOn && lastIp == ip) return;
  lastWifiOn = true;
  lastIp     = ip;
  strcpy(wifiLine, "WiFi ON");
  snprintf(ipLine, sizeof(ipLine), "%u.%u.%u.%u",
           ip[0], ip[1], ip[2], ip[3]);
  render();
}

void displayWifiDown() {
  if (!ready) return;
  if (!lastWifiOn) return;
  lastWifiOn = false;
  strcpy(wifiLine, "WiFi OFF");
  ipLine[0] = '\0';
  render();
}

void displaySleep() {
  if (!ready) return;
  oled.clearBuffer();
  oled.sendBuffer();
  oled.setPowerSave(1);
}
