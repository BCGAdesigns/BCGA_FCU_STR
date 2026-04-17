// BCGA FCU STR — web_server.cpp

#include "web_server.h"
#include "web_ui.h"
#include "config.h"
#include "types.h"
#include "storage.h"
#include "buzzer.h"
#include "firing.h"
#include "wifi_manager.h"

#include <WebServer.h>
#include <ArduinoJson.h>

namespace {
WebServer server(HTTP_PORT);
bool running = false;

uint16_t clampU(uint16_t v, uint16_t lo, uint16_t hi) {
  if (v < lo) return lo;
  if (v > hi) return hi;
  return v;
}

void sendJson(int code, const JsonDocument& doc) {
  String out;
  serializeJson(doc, out);
  server.sendHeader("Cache-Control", "no-store");
  server.send(code, "application/json", out);
  wifiNoteActivity();
}

void sendErr(int code, const char* msg) {
  StaticJsonDocument<128> d;
  d["ok"] = false;
  d["error"] = msg;
  sendJson(code, d);
}

void handleRoot() {
  server.sendHeader("Cache-Control", "no-store");
  server.send_P(200, "text/html; charset=utf-8", WEB_UI_HTML);
  wifiNoteActivity();
}

void handleLoad() {
  StaticJsonDocument<256> d;
  d["ok"]       = true;
  d["fw"]       = FW_VERSION;
  d["variant"]  = FW_VARIANT;
  d["ssid"]     = WIFI_SSID_DEFAULT;
  d["lang"]     = storageGetLang() == LANG_EN ? "en" : "br";
  d["lastSlot"] = storageGetLastSlot();
  d["slots"]    = SLOT_COUNT;
  d["shots"]    = (uint32_t)storageGetTotalShots() + (uint32_t)firingShotCount();
  sendJson(200, d);
}

void slotToJson(const SlotConfig& c, JsonObject d) {
  d["name"]        = c.name;
  d["trigMode"]    = c.trigMode;
  d["selMode"]     = c.selMode;
  d["sel3pos"]     = c.sel3pos;
  d["selPos1"]     = c.selPos1Mode;
  d["selPos2"]     = c.selPos2Mode;
  d["selPos3"]     = c.selPos3Mode;
  d["dr"]          = c.dr;
  d["dp"]          = c.dp;
  d["rof"]         = c.rofLimit;
  d["hallTrigLow"] = c.hallTrigLow;
  d["hallTrigHigh"]= c.hallTrigHigh;
  d["hallSelLow1"] = c.hallSelLow1;
  d["hallSelLow2"] = c.hallSelLow2;
  d["invertTrig"]  = c.invertTrig;
  d["silent"]      = c.silentMode;
}

void handleGetSlot() {
  if (!server.hasArg("i")) { sendErr(400, "missing i"); return; }
  uint8_t i = (uint8_t)server.arg("i").toInt();
  if (i >= SLOT_COUNT) { sendErr(400, "i out of range"); return; }
  SlotConfig c;
  storageLoadSlot(i, c);
  StaticJsonDocument<512> d;
  d["ok"] = true;
  d["i"]  = i;
  JsonObject s = d.createNestedObject("slot");
  slotToJson(c, s);
  sendJson(200, d);
}

void handleSave() {
  if (!server.hasArg("i"))      { sendErr(400, "missing i"); return; }
  if (!server.hasArg("plain"))  { sendErr(400, "no body"); return; }
  uint8_t i = (uint8_t)server.arg("i").toInt();
  if (i >= SLOT_COUNT)          { sendErr(400, "i out of range"); return; }

  StaticJsonDocument<768> body;
  DeserializationError e = deserializeJson(body, server.arg("plain"));
  if (e) { sendErr(400, "bad json"); return; }

  SlotConfig c;
  storageLoadSlot(i, c);

  if (body.containsKey("name")) {
    const char* nm = body["name"]; if (nm) {
      strncpy(c.name, nm, sizeof(c.name) - 1);
      c.name[sizeof(c.name) - 1] = '\0';
    }
  }
  if (body.containsKey("trigMode"))    c.trigMode    = (uint8_t)constrain((int)body["trigMode"], 0, 1);
  if (body.containsKey("selMode"))     c.selMode     = (uint8_t)constrain((int)body["selMode"], 0, 1);
  if (body.containsKey("sel3pos"))     c.sel3pos     = body["sel3pos"] ? 1 : 0;
  if (body.containsKey("selPos1"))     c.selPos1Mode = (uint8_t)constrain((int)body["selPos1"], 0, 5);
  if (body.containsKey("selPos2"))     c.selPos2Mode = (uint8_t)constrain((int)body["selPos2"], 0, 5);
  if (body.containsKey("selPos3"))     c.selPos3Mode = (uint8_t)constrain((int)body["selPos3"], 0, 5);
  if (body.containsKey("dr"))          c.dr  = clampU((uint16_t)body["dr"],  FIRE_MIN_MS, FIRE_MAX_MS);
  if (body.containsKey("dp"))          c.dp  = clampU((uint16_t)body["dp"],  FIRE_MIN_MS, FIRE_MAX_MS);
  if (body.containsKey("rof"))         c.rofLimit   = (uint16_t)constrain((int)body["rof"], 0, 50);
  if (body.containsKey("hallTrigLow"))  c.hallTrigLow  = clampU((uint16_t)body["hallTrigLow"], 0, 4095);
  if (body.containsKey("hallTrigHigh")) c.hallTrigHigh = clampU((uint16_t)body["hallTrigHigh"], 0, 4095);
  if (body.containsKey("hallSelLow1"))  c.hallSelLow1  = clampU((uint16_t)body["hallSelLow1"], 0, 4095);
  if (body.containsKey("hallSelLow2"))  c.hallSelLow2  = clampU((uint16_t)body["hallSelLow2"], 0, 4095);
  if (body.containsKey("invertTrig"))  c.invertTrig = body["invertTrig"] ? 1 : 0;
  if (body.containsKey("silent"))      c.silentMode = body["silent"]    ? 1 : 0;

  if (!storageSaveSlot(i, c)) { sendErr(500, "nvs write failed"); return; }
  if (body.containsKey("makeActive") && body["makeActive"]) {
    storageSetLastSlot(i);
  }
  buzzerPlay(BUZZ_SAVE_OK);

  StaticJsonDocument<96> r;
  r["ok"] = true;
  sendJson(200, r);
}

void handleTest() {
  if (!server.hasArg("plain")) { sendErr(400, "no body"); return; }
  StaticJsonDocument<128> body;
  if (deserializeJson(body, server.arg("plain"))) { sendErr(400, "bad json"); return; }
  if (body.containsKey("buzz")) {
    int p = body["buzz"];
    if (p == 0) buzzerStop();
    else        buzzerPlay((BuzzPattern)constrain(p, 1, (int)BUZZ_TEST));
  } else if (body.containsKey("mos")) {
    int which = body["mos"];
    if (firingActive()) { sendErr(409, "firing active"); return; }
    // Starter has only SOL1 (poppet) wired. "mos: 1" pulses PIN_MOS_1.
    if (which != 1) { sendErr(400, "bad mos"); return; }
    mosTestSchedule(PIN_MOS_1);
  } else { sendErr(400, "missing buzz/mos"); return; }
  StaticJsonDocument<64> r; r["ok"] = true; sendJson(200, r);
}

void handleHallLive() {
  // GET /halllive?ch=t|s  → { ok, ch, value }
  if (!server.hasArg("ch")) { sendErr(400, "missing ch"); return; }
  String ch = server.arg("ch");
  int v = -1;
  if (ch == "t") v = analogRead(PIN_TRIG);
  else if (ch == "s") v = analogRead(PIN_SEL);
  else { sendErr(400, "bad ch"); return; }
  StaticJsonDocument<96> r;
  r["ok"] = true;
  r["ch"] = ch;
  r["value"] = v;
  sendJson(200, r);
}

void handleNoiseCal() {
  // POST /noisecal  body: { "ch": "t"|"s" }
  // Fires one full shot cycle while sampling the requested ADC pin.
  if (firingActive()) { sendErr(409, "firing active"); return; }
  if (!server.hasArg("plain")) { sendErr(400, "no body"); return; }
  StaticJsonDocument<64> body;
  if (deserializeJson(body, server.arg("plain"))) { sendErr(400, "bad json"); return; }
  const char* ch = body["ch"] | "t";
  uint8_t pin = (ch[0] == 's') ? PIN_SEL : PIN_TRIG;

  SlotConfig c; storageLoadSlot(storageGetLastSlot(), c);
  NoiseReport rep = firingPulseCycleWithSample(c, pin);
  if (!rep.ok) { sendErr(500, "no samples"); return; }

  StaticJsonDocument<128> r;
  r["ok"]      = true;
  r["adcMin"]  = rep.adcMin;
  r["adcMax"]  = rep.adcMax;
  r["samples"] = rep.samples;
  r["margin"]  = (rep.adcMax - rep.adcMin) / 2;
  sendJson(200, r);
}

void handleSetPwd() {
  if (!server.hasArg("plain")) { sendErr(400, "no body"); return; }
  StaticJsonDocument<128> body;
  if (deserializeJson(body, server.arg("plain"))) { sendErr(400, "bad json"); return; }
  const char* pwd = body["pwd"] | "";
  if (!storageSetWifiPass(pwd)) { sendErr(400, "pwd too short"); return; }
  StaticJsonDocument<64> r; r["ok"] = true; sendJson(200, r);
}

void handleSetLang() {
  if (!server.hasArg("plain")) { sendErr(400, "no body"); return; }
  StaticJsonDocument<64> body;
  if (deserializeJson(body, server.arg("plain"))) { sendErr(400, "bad json"); return; }
  const char* l = body["lang"] | "br";
  Language lang = (l[0] == 'e') ? LANG_EN : LANG_BR;
  storageSetLang(lang);
  StaticJsonDocument<64> r; r["ok"] = true; sendJson(200, r);
}

void handleSetSlot() {
  if (!server.hasArg("i")) { sendErr(400, "missing i"); return; }
  uint8_t i = (uint8_t)server.arg("i").toInt();
  if (i >= SLOT_COUNT)     { sendErr(400, "out of range"); return; }
  storageSetLastSlot(i);
  StaticJsonDocument<64> r; r["ok"] = true; sendJson(200, r);
}

void handleResetSlot() {
  if (!server.hasArg("i")) { sendErr(400, "missing i"); return; }
  uint8_t i = (uint8_t)server.arg("i").toInt();
  if (i >= SLOT_COUNT)     { sendErr(400, "out of range"); return; }
  if (firingActive())      { sendErr(409, "firing active"); return; }
  SlotConfig c;
  storageDefaultSlot(i, c);
  if (!storageSaveSlot(i, c)) { sendErr(500, "nvs write failed"); return; }
  buzzerPlay(BUZZ_SAVE_OK);
  StaticJsonDocument<512> r;
  r["ok"] = true;
  r["i"]  = i;
  JsonObject s = r.createNestedObject("slot");
  slotToJson(c, s);
  sendJson(200, r);
}

void handleNotFound() {
  handleRoot();
}

}  // namespace

void webServerBegin() {
  if (running) return;
  server.on("/",         HTTP_GET,  handleRoot);
  server.on("/load",     HTTP_GET,  handleLoad);
  server.on("/getslot",  HTTP_GET,  handleGetSlot);
  server.on("/halllive", HTTP_GET,  handleHallLive);
  server.on("/save",     HTTP_POST, handleSave);
  server.on("/test",     HTTP_POST, handleTest);
  server.on("/noisecal", HTTP_POST, handleNoiseCal);
  server.on("/setpwd",   HTTP_POST, handleSetPwd);
  server.on("/setlang",  HTTP_POST, handleSetLang);
  server.on("/setslot",  HTTP_POST, handleSetSlot);
  server.on("/reset",    HTTP_POST, handleResetSlot);
  server.onNotFound(handleNotFound);
  server.begin();
  running = true;
}

void webServerEnd() {
  if (!running) return;
  server.stop();
  running = false;
}

void webServerHandle() {
  if (!running) return;
  server.handleClient();
}
