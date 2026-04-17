// BCGA FCU V2 — web_server.h
// HTTP API exposed when WiFi AP is on. Endpoints:
//   GET  /                — dashboard HTML
//   GET  /load            — global state JSON (lang, lastSlot, fw, batt, shots)
//   GET  /getslot?i=N     — slot config JSON
//   POST /save?i=N        — slot config JSON body
//   POST /test            — body: {"buzz":N} or {"mos":1|2,"ms":...}
//   POST /setpwd          — body: {"pwd":"..."}
//   POST /setlang         — body: {"lang":"pt"|"en"}

#pragma once

void webServerBegin();
void webServerEnd();
void webServerHandle();
