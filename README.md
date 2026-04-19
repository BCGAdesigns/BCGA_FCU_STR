# BCGA FCU — Open-source HPA Fire Control Unit

[![License: GPL v3](https://img.shields.io/badge/License-GPL%20v3-blue.svg)](LICENSE)
[![Language: C++](https://img.shields.io/badge/language-C%2B%2B-orange.svg)](firmware/)
[![Platform: ESP32-C3](https://img.shields.io/badge/platform-ESP32--C3-green.svg)](https://www.espressif.com/en/products/socs/esp32-c3)
[![Status: Active](https://img.shields.io/badge/status-active-brightgreen.svg)](https://github.com/BCGAdesigns/BCGA_FCU_STR)

> Open-source Fire Control Unit for airsoft HPA replicas. WiFi-native configuration, dual-solenoid support, 4 independent cycle timings, and Hall trigger with automatic EMI noise calibration — unique on the market.

> 🇧🇷 Leia em português: [LEIA-ME.md](LEIA-ME.md)

---

## What is the BCGA FCU?

The **BCGA FCU** is a digital Fire Control Unit for airsoft HPA (High Pressure Air) replicas, built around the ESP32-C3 SuperMini. It drives the solenoid(s) that replace the mechanical trigger of an HPA gearbox and lets you tune each phase of the firing cycle from any browser over WiFi — no app required.

Two hardware variants share the same firmware and web UI:

- **STR** (Starter) — perfboard / THT build, easily sourced components, ideal for makers and hobbyists learning HPA electronics.
- **PRO** — commercial SMD PCB with battery monitoring, LiPo kill latch, dedicated WiFi button and onboard buzzer.

Both variants support **S8PA** (1-solenoid engines — PolarStar JACK / F1, Wolverine INFERNO, GATE PULSAR S) and **D8PA** (2-solenoid engines — PolarStar F2 / Fusion Engine, GATE PULSAR D) per slot. You pick the engine type per profile — the UI adapts automatically.

This project is designed, manufactured and maintained by **[BCGA Airsoft](https://bcgaairsoft.com)** in Brazil. It is **open-source under GPL v3** — you can audit, modify, manufacture and sell, as long as derivatives keep the same license and credit the original project.

---

## Why BCGA FCU?

Compared to commercial FCUs (PolarStar REV3, Wolverine BLINC, GATE TITAN II, Gorilla FCU), the BCGA FCU has concrete advantages — and a few honest limitations. This section is factual; the full side-by-side tables are in [`firmware/MANUAL_EN.md`](firmware/MANUAL_EN.md) section 16.

### Where the BCGA FCU wins

1. **Native WiFi vs Bluetooth.** Configure from any browser on any device — iOS, Android, PC, Linux. No app install, no pairing, no vendor lock-in. TITAN II (BLE 5.2) and BLINC require proprietary apps.

2. **4 independent timings (DN/DR/DP/DB).** Each phase of the D8PA cycle has its own parameter. Feeding (DN), sealing (DR), FPS (DP) and post-shot debounce (DB) tune independently without trade-offs. Commercial FCUs top out at 3 user-facing timings (REV3 in dual-solenoid mode: dP/dN/dr) or a single dwell on single-solenoid setups — none expose a dedicated post-shot debounce.

3. **Automatic Hall trigger EMI noise calibration.** The only FCU on the market with a routine that fires the solenoids dry and measures the EMI kick on the ADC, widening the Hall deadband automatically. Eliminates ghost fires without sacrificing trigger sensitivity.

4. **3 complete independent slots.** Each slot stores everything — engine type, all 4 timings, selector config, individual Hall calibrations, flags. Switching slots = full game-profile switch.

5. **Open-source, GPL v3.** All code is public. Audit, modify, compile and flash without depending on proprietary firmware or a vendor app. TITAN II, BLINC and REV3 are fully closed.

6. **Hall trigger with 2-point calibration + automatic hysteresis.** Captures the exact fire point and computes a hysteresis band from real measurements — not fixed values. No mechanical potentiometer.

7. **3-position Hall selector.** SAFE/SEMI/FULL via a wear-free Hall sensor. Each position freely configurable to any mode (including BURST 2/3/4).

8. **Drastically lower BOM cost.** The STR can be built from easily sourced THT parts for a fraction of any commercial Bluetooth FCU.

9. **Live theoretical ROF in the UI.** The web panel shows maximum achievable ROF as you move the sliders — no chrono needed for a first-pass estimate.

10. **Per-slot S8PA/D8PA switching in software.** Each of the 3 slots independently stores the engine type — flip a slot between S8PA and D8PA directly from the web UI, no hardware change. Commercial FCUs that cover both architectures (TITAN II with PULSAR S/D, Gorilla) do it through separate dedicated cable harnesses per engine, not per-slot software switching.

### Honest limitations

- **No binary trigger.** Not implemented. Available on Wolverine BLINC, GATE TITAN II and Gorilla FCU.
- **No tournament lock with password.** Workaround: set Semi ROF high and ROF limit low before an event. Available on TITAN II (Expert) and Gorilla.
- **Kill latch and onboard buzzer are PRO-only.** The STR variant has no battery voltage read and no LiPo deep-discharge cut-off — use with caution on 2S/3S packs without external protection.
- **First pull after deep sleep wakes via reboot.** After 60 min of idle, the next pull wakes the MCU through a full reboot — the **second** pull actually fires.

---

## Hardware specifications

| Item | Value |
|---|---|
| MCU | ESP32-C3 SuperMini |
| MOSFETs | IRLZ44NPBF (TO-220, THT) |
| Main connector | JST 8-pin (A1501WR-S-8P) — native to the D8PA ecosystem |
| Operating voltage | 7.4 V — 11.1 V (2S/3S LiPo) |
| Engine types | S8PA (1 solenoid) / D8PA (2 solenoids) |
| Fire modes | SAFE / SEMI / FULL / BURST 2 / BURST 3 / BURST 4 |
| Configuration | Web UI over WiFi AP (192.168.4.1) |
| Deep sleep | 60 min idle → <10 µA |
| License | GPL v3 |

---

## Repository structure

```
BCGA_FCU_STR/
├── MotherBoard/              # Main board (MOBO) — BOM, CPL, GERBER, SCH, STEP
├── DaugtherBoard_BCGA/       # Daughterboard A — BCGA gearbox (S8PA or D8PA)
├── Gearbox_DaughterBoard/    # Daughterboard B — V2 standard (AEG conversion)
├── firmware/
│   ├── BCGA_FCU_STR/         # Arduino sketch — Starter variant
│   ├── BCGA_FCU_PRO/         # Arduino sketch — Pro variant
│   ├── MANUAL_EN.md / .pdf   # English manual (this README links here)
│   └── MANUAL_PT.md / .pdf   # Portuguese manual
├── README.md                 # English readme (this file)
├── LEIA-ME.md                # Portuguese readme
└── LICENSE                   # GPL v3
```

Each board folder contains: `BOM` (xlsx), `CPL` (xlsx), `GERBER` (zip), `SCH` (pdf) and `STEP` (3D model).

---

## How to use

1. Get the hardware — either buy a pre-assembled board from [bcgaairsoft.com/fcu/str](https://bcgaairsoft.com/fcu/str) / [fcu/pro](https://bcgaairsoft.com/fcu/pro), or clone this repo and build your own (see next section).
2. Flash the matching firmware from `firmware/BCGA_FCU_STR/` or `firmware/BCGA_FCU_PRO/` via the Arduino IDE. Target board: **ESP32-C3 Dev Module**, with **USB CDC On Boot = Enabled**.
3. On first boot, the FCU brings up a WiFi AP.
   - Default SSID: `BCGA_FCU_STR` or `BCGA_FCU_PRO`
   - Default password: `12345678`
4. Connect your phone/laptop to the AP. A captive page opens at `http://192.168.4.1`.
5. Configure the 3 slots: engine type (S8PA/D8PA), timings (DN/DR/DP/DB), selector, flags. The full tuning workflow is in [`firmware/MANUAL_EN.md`](firmware/MANUAL_EN.md).

To re-open the AP later:
- **STR** — hold the trigger for 5 s during the first 5 s after boot; OR pull the trigger 5 times in SAFE within 3 s.
- **PRO** — press the dedicated WiFi button; or use the same gestures as STR.

The AP shuts off automatically after 10 min of web inactivity.

---

## Build your own

1. Upload the chosen board's `GERBER` zip to a PCB fab (JLCPCB, PCBWay, etc.).
2. Order the components from the matching `BOM`. Most parts are available on [LCSC](https://www.lcsc.com/).
3. Solder following the `SCH` (schematic).
4. Flash the firmware from `firmware/BCGA_FCU_{STR,PRO}/` via the Arduino IDE.
5. Calibrate the Hall trigger and selector from the web UI (sections 7 and 8 of the manual).

> ⚠️ **STR** requires basic THT soldering skills (TO-220 MOSFETs, through-hole headers). **PRO** requires basic SMD soldering skills. For a plug-and-play board, buy the pre-assembled version at [bcgaairsoft.com](https://bcgaairsoft.com).

---

## License

[**GPL v3**](LICENSE). You can manufacture, modify, sell and distribute freely, as long as derivatives keep the same license and credit the original project. Closed-source forks are not permitted.

---

## Links

- **Product pages:** [bcgaairsoft.com/fcu/str](https://bcgaairsoft.com/fcu/str) · [bcgaairsoft.com/fcu/pro](https://bcgaairsoft.com/fcu/pro)
- **Main site:** [bcgaairsoft.com](https://bcgaairsoft.com)
- **User manual (English):** [firmware/MANUAL_EN.md](firmware/MANUAL_EN.md) · [PDF](firmware/MANUAL_EN.pdf)
- **User manual (Portuguese):** [firmware/MANUAL_PT.md](firmware/MANUAL_PT.md) · [PDF](firmware/MANUAL_PT.pdf)
- **Issues and support:** [GitHub Issues](https://github.com/BCGAdesigns/BCGA_FCU_STR/issues)

---

**BCGA Airsoft** — [bcgaairsoft.com](https://bcgaairsoft.com)
