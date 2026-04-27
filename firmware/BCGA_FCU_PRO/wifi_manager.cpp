// BCGA FCU PRO — wifi_manager.cpp

#include "wifi_manager.h"
#include "config.h"
#include "storage.h"
#include "buzzer.h"
#include "web_server.h"
#include "display.h"

#include <WiFi.h>
#include <DNSServer.h>

// Defined in BCGA_FCU_PRO.ino — resets the inactivity-alarm watchdog.
extern void noteUserActivity();

namespace {
DNSServer dns;
bool      active     = false;
uint32_t  startMs    = 0;
uint32_t  lastActMs  = 0;

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
  displayWifiUp(ip);
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
  displayWifiDown();
}
}  // namespace

void wifiManagerBegin() {
  WiFi.persistent(false);
  WiFi.mode(WIFI_OFF);
  active = false;
  pinMode(PIN_WIFI_BTN, INPUT_PULLUP);
}

void wifiStart()         { doStart(); }
void wifiStop()          { doStop(); }
bool wifiActive()        { return active; }
void wifiNoteActivity()  {
  if (active) lastActMs = millis();
  noteUserActivity();
}

void wifiManagerUpdate() {
  // ── Button state machine ──────────────────────────────────────────────────
  // Hold ≥3s  (release between 3s and 30s) → toggle WiFi. 3 beeps on turn-on.
  // Hold ≥30s → reset WiFi password to default; continuous tone starts at 3s
  //             as "hold in progress" feedback.
  static bool     btnPrev      = HIGH;   // last raw read (active LOW)
  static uint32_t btnDownMs    = 0;      // millis() when button went LOW
  static bool     continuousOn = false;  // whether continuous buzzer is playing

  bool btnNow = digitalRead(PIN_WIFI_BTN);

  // Falling edge: button pressed
  if (btnPrev == HIGH && btnNow == LOW) {
    btnDownMs    = millis();
    continuousOn = false;
    noteUserActivity();   // count any button press as activity
  }

  // While held: start continuous buzzer feedback after 3s
  if (btnNow == LOW && btnDownMs != 0) {
    uint32_t held = (uint32_t)(millis() - btnDownMs);
    if (held >= 3000 && !continuousOn) {
      continuousOn = true;
      ledcWriteTone(PIN_BUZZER, 2700);
      ledcWrite(PIN_BUZZER, 128);
    }
  }

  // Rising edge: button released
  if (btnPrev == LOW && btnNow == HIGH && btnDownMs != 0) {
    uint32_t held = (uint32_t)(millis() - btnDownMs);
    btnDownMs = 0;

    if (continuousOn) {
      ledcWrite(PIN_BUZZER, 0);
      continuousOn = false;
    }

    if (held >= 30000) {
      storageSetWifiPass(WIFI_PASS_DEFAULT);
      buzzerPlay(BUZZ_SAVE_OK);
      if (active) {
        doStop();
        doStart();
      }
      LOG("WiFi password reset to default\n");
    } else if (held >= 3000) {
      if (active) {
        doStop();
      } else {
        doStart();
        buzzerPlayCount(3);
      }
    }
    // held < 3000: ignore
  }

  btnPrev = btnNow;

  // ── WiFi services ─────────────────────────────────────────────────────────
  if (active) {
    dns.processNextRequest();
    webServerHandle();
    if ((uint32_t)(millis() - lastActMs) > WIFI_AUTO_OFF_MS) {
      doStop();
    }
  }
}
