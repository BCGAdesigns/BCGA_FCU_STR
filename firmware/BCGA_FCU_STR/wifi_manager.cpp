// BCGA FCU STR — wifi_manager.cpp
// Starter has no WiFi button — AP is brought up by the 5-pull trigger gesture
// (in SAFE) and torn down after WIFI_AUTO_OFF_MS of inactivity.

#include "wifi_manager.h"
#include "config.h"
#include "storage.h"
#include "buzzer.h"
#include "web_server.h"

#include <WiFi.h>
#include <DNSServer.h>

namespace {
DNSServer dns;
bool      active     = false;
uint32_t  startMs    = 0;
uint32_t  lastActMs  = 0;

// gesture
uint8_t   pullCount  = 0;
uint32_t  pullWindow = 0;

void doStart() {
  if (active) return;
  char pwd[33]; storageGetWifiPass(pwd, sizeof(pwd));
  WiFi.mode(WIFI_AP);
  bool ok = WiFi.softAP(WIFI_SSID_DEFAULT, pwd);
  if (!ok) { LOGLN("AP start failed"); return; }
  IPAddress ip = WiFi.softAPIP();
  dns.setErrorReplyCode(DNSReplyCode::NoError);
  dns.start(DNS_PORT, "*", ip);
  webServerBegin();
  active     = true;
  startMs    = millis();
  lastActMs  = startMs;
  buzzerPlay(BUZZ_WIFI_ON);
  LOG("WiFi AP up: %s @ %s\n", WIFI_SSID_DEFAULT, ip.toString().c_str());
}

void doStop() {
  if (!active) return;
  webServerEnd();
  dns.stop();
  WiFi.softAPdisconnect(true);
  WiFi.mode(WIFI_OFF);
  active = false;
  buzzerPlay(BUZZ_WIFI_OFF);
  LOGLN("WiFi off");
}
}  // namespace

void wifiManagerBegin() {
  WiFi.persistent(false);
  WiFi.mode(WIFI_OFF);
  active = false;
}

void wifiStart()         { doStart(); }
void wifiStop()          { doStop(); }
bool wifiActive()        { return active; }
void wifiNoteActivity()  { if (active) lastActMs = millis(); }

void wifiNoteTriggerPull() {
  uint32_t now = millis();
  if ((uint32_t)(now - pullWindow) > WIFI_GESTURE_WINDOW_MS) {
    pullWindow = now;
    pullCount = 1;
  } else {
    pullCount++;
  }
  if (pullCount >= WIFI_GESTURE_PULL_COUNT) {
    pullCount = 0;
    if (active) doStop(); else doStart();
  }
}

void wifiManagerUpdate() {
  if (active) {
    dns.processNextRequest();
    webServerHandle();
    if ((uint32_t)(millis() - lastActMs) > WIFI_AUTO_OFF_MS) {
      doStop();
    }
  }
}
