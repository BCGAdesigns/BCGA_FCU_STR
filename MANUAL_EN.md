# BCGA FCU — User Manual (EN)

Open-source firmware for the ESP32-C3 SuperMini. Two variants: **STR** (Starter, perfboard) and **PRO** (commercial product). The web UI and firing logic are nearly identical — differences are listed at the end.

This manual is practical: each section leads with **what to tweak to get what result**.

> 🇧🇷 Versão em português: [MANUAL_PT.md](./MANUAL_PT.md)

---

## Table of contents

1. [Understand the FCU in 1 minute](#1-understand-the-fcu-in-1-minute)
2. [Quick start — reach target FPS first](#2-quick-start--reach-target-fps-first)
3. [The 4 timings — tuning guide](#3-the-4-timings--tuning-guide)
4. [Recommended tuning workflow](#4-recommended-tuning-workflow)
5. [Firing type: S8PA vs D8PA](#5-firing-type-s8pa-vs-d8pa)
6. [Selector — 2 or 3 positions](#6-selector--2-or-3-positions)
7. [Hall trigger — step-by-step calibration](#7-hall-trigger--step-by-step-calibration)
8. [Hall selector — step-by-step calibration](#8-hall-selector--step-by-step-calibration)
9. [First use and web panel access](#9-first-use-and-web-panel-access)
10. [Slots](#10-slots)
11. [Useful flags (invert trigger, swap MOS, silent)](#11-useful-flags)
12. [Limit rate of fire (ROF limit and Semi ROF)](#12-limit-rate-of-fire)
13. [Diagnostics and WiFi](#13-diagnostics-and-wifi)
14. [Deep sleep, inactivity alarm and debug mode](#14-deep-sleep-inactivity-alarm-and-debug-mode)
15. [STR vs PRO differences](#15-str-vs-pro-differences)
16. [BCGA FCU vs commercial FCUs](#16-bcga-fcu-vs-commercial-fcus)

---

## 1. Understand the FCU in 1 minute

The FCU drives **1 or 2 solenoids** that replace the mechanical trigger of an HPA engine. For each shot it sends electrically calibrated pulses to the solenoids, in the correct order, with the correct waits between them. Tuning = adjusting those pulses for your specific setup (pressure, bucking, BB mass).

- **S8PA** (1 solenoid): PolarStar JACK / F1, Wolverine INFERNO, GATE PULSAR S, or any other 1-solenoid HPA engine. Only the poppet is driven.
- **D8PA** (2 solenoids): PolarStar F2 / Fusion Engine, GATE PULSAR D, or any other 2-solenoid HPA engine. Nozzle and poppet driven separately.

---

## 2. Quick start — reach target FPS first

New to the BCGA FCU? Before anything else, hit your target FPS. The rest of the tuning only makes sense once the chrono is where you want it. The 4 timings (DN/DR/DP/DB) are fully explained in section 3 — for this first pass you only touch **DP** (the poppet dwell slider) and the regulator.

1. **Set the regulator to 100 psi.** Default starting pressure for most D8PA setups.
2. **Keep the default timings** (`DN=18 / DR=26 / DP=25 / DB=100`). They ship loaded.
3. **Fire over a chrono.** Is the FPS (or joule) on target?
   - **Yes** → skip straight to section 4 and tune feeding, sealing and accuracy.
   - **No, too low** → step 4.
4. **Push DP to the slider maximum** (80 ms). Chrono again.
   - **FPS jumped to target** → lower DP step-by-step until FPS starts to drop, then add 1–2 ms back. That's your minimum efficient DP. Go to section 4.
   - **FPS still below target** → step 5.
5. **Raise the regulator to 110 psi.** Chrono again.
   - **On target** → lower DP until you find the minimum that holds target FPS. Go to section 4.
   - **Still low** → raise to **120 psi** and chrono again.
6. Once target FPS is stable, **go to section 4** and tune DN (feeding), DR (sealing) and DB (accuracy) in that order.

> **Principle:** FPS is driven mostly by **air pressure × DP**. The other 3 timings shape how the cycle *behaves* (feeding, sealing, accuracy, ROF) — they don't add FPS. Nail FPS first with DP/pressure, tune everything else after.

---

## 3. The 4 timings — tuning guide

The FCU exposes **4 independent timings** that map directly to each physical phase of the firing cycle: **DN**, **DR**, **DP**, **DB**. The page shows the theoretical ROF live as you move the sliders.

> **Note on units:** DN, DR and DP are in **milliseconds** (range 2–80 ms). **DB** (Trigger Debounce) uses **units** — 1 unit = 0.1 ms, range 20–800 units (= 2–80 ms). This aligns DB with the firmware's internal 0.1 ms timing resolution.

### DN — Nozzle Dwell (D8PA only)

Length of the **nozzle (SOL2)** pulse. This is how long the nozzle stays open for the BB to drop into the chamber.

| If DN increases | If DN decreases |
|---|---|
| ✅ easier feeding (heavy BBs, low pressure, cold shot) | ✅ saves air, prevents double-feed |
| ❌ **risk of double-feed** (2 BBs drop) | ❌ empty shots / inconsistent feeding if the return spring is slow |
| ❌ more nozzle wear | ❌ may not seal before DP fires |

**Rule of thumb:** decrease DN until you start getting empty shots, then add 1–2 ms margin.

### DR — Return / Rest

- **In D8PA**: wait between the nozzle pulse and the poppet pulse. Time for the nozzle to return and the **bucking to seal on the BB**.
- **In S8PA**: inter-shot rest.

| If DR increases | If DR decreases |
|---|---|
| ✅ consistent sealing → **stable FPS** | ✅ **higher ROF** |
| ✅ less shot-to-shot spread | ❌ poppet fires before seal → weak shot / low and variable FPS |
| ❌ lower ROF | ❌ BBs dropping past an open nozzle = double / bad pickup |

**Rule of thumb:** raise DR until the chrono stops oscillating under fast SEMI.

### DP — Poppet Dwell (the shot)

Length of the **poppet (SOL1)** pulse. This is how long air can flow through the nozzle pushing the BB.

| If DP increases | If DP decreases |
|---|---|
| ✅ **higher FPS** (more air behind the BB) | ✅ saves air (more shots per bottle/mag) |
| ❌ air waste after BB has left | ✅ higher ROF (valve closes fast) |
| ❌ may delay poppet recycle | ❌ **FPS below target**, weak shot |

**Rule of thumb:** with a chrono, tune DP up to your target FPS. Anything more just wastes air.

### DB — Trigger Debounce (D8PA only)

Wait after the poppet closes, before the trigger can re-arm the next cycle. Physically, this is the time needed for the **BB to clear the barrel** and the bucking to recover.

**Unit:** 1 unit = 0.1 ms. **Range:** 20–800 units (2–80 ms). **Default:** 100 units (10 ms).

| Units | Equivalent ms |
|---:|---:|
| 20  | 2.0  |
| 50  | 5.0  |
| 100 | 10.0 |
| 200 | 20.0 |
| 800 | 80.0 |

| If DB increases | If DB decreases |
|---|---|
| ✅ **better accuracy** (BB has left before the next nozzle opens) | ✅ **higher ROF** |
| ✅ bucking recovers undisturbed | ❌ BB still in the barrel when the next nozzle pulses → **flyers / inconsistent FPS** |
| ❌ lower ROF | ❌ reduced bucking life |

**Rule of thumb:** full-auto on a 20 m target. If flyers appear, bump DB by 20 units (2 ms) at a time.

### Defaults (generic D8PA, 110 psi, 0.28 g BB)

```
DN = 18 ms    DR = 26 ms    DP = 25 ms    DB = 100 units (10 ms)
```

Safe starting point. Tune from there.

---

## 4. Recommended tuning workflow

Follow this order — each step depends on the previous one being stable. Assumes you already hit your target FPS per section 2.

1. **Feeding (DN)** — slow SEMI. Lower DN until you get empty shots. Add 2 ms back.
2. **Sealing (DR)** — fast SEMI. If chrono wobbles, raise DR 2 ms at a time.
3. **FPS (DP)** — with chrono, raise DP to hit target FPS. Don't go higher.
4. **Accuracy (DB, D8PA only)** — full-auto on a target. If flyers, raise DB by 20 units.
5. **Cadence (ROF limit)** — optional. Caps maximum ROF independently of timings.
6. **Anti-spam (Semi ROF)** — optional. Minimum time between SEMI pulls.

> ⚠️ Always verify with a chrono. FPS drifts with bottle temperature / air level.

---

## 5. Firing type: S8PA vs D8PA

Chosen per slot in the first section of the panel.

- **S8PA** — only the poppet is pulsed. Cycle: `DP → DR → repeat`. Use with PolarStar JACK / F1, Wolverine INFERNO, GATE PULSAR S, or any 1-solenoid HPA engine. The DN, DB, MOS swap fields and the SOL 2 test button are hidden.
- **D8PA** — separate nozzle + poppet. Cycle: `DN → DR → DP → DB → repeat`. Use with PolarStar F2 / Fusion Engine, GATE PULSAR D, or any 2-solenoid system.

---

## 6. Selector — 2 or 3 positions

Enabling "3 positions" **requires a Hall selector** (a microswitch can only distinguish 2 states). Enabling it reveals Pos 3 in the UI.

Each position is assigned to a mode **independently**:

| Mode | Behavior |
|---|---|
| **SAFE** | Trigger pull does nothing. Mode-change beeps still play. |
| **SEMI** | One shot per pull. Honors Semi ROF if configured. |
| **FULL** | Fires while the trigger is held. |
| **BURST 2 / 3 / 4** | Fires 2/3/4 rounds per pull, even if trigger is released. |

Common examples:
- **2-position**: Pos 1 = SAFE, Pos 2 = SEMI *(or Pos 1 = SEMI, Pos 2 = FULL)*
- **3-position**: Pos 1 = SAFE, Pos 2 = SEMI, Pos 3 = FULL

---

## 7. Hall trigger — step-by-step calibration

Use Hall for a **wear-free trigger** with an **adjustable fire point**.

**Expected hardware:** linear Hall sensor (DRV5055 or similar) powered from 3.3 V, output wired to **PIN_TRIG** (GPIO 0 on both variants). Magnet on the trigger lever.

### 7.1 Mode selection

**Input** section → "Trigger type" → **Hall** → **Save**.

### 7.2 Quick calibration (single-point)

Covers most cases — 30 seconds.

1. In the **Sensitivity** section, pull the trigger **to the exact fire point you want** and hold.
2. Click **Capture point**.
3. The FCU reads the ADC and builds a symmetric hysteresis band around that value.
4. Save the slot.

Fires before reaching the point → capture at a deeper pull.
Won't fire even at full pull → capture at a shallower pull.

### 7.3 Full calibration (maximum precision)

In the **Trigger calibration** section:

1. **Noise calibration**
   - FCU fires **a full cycle without a BB** sampling the ADC during the solenoid pulses.
   - Measures EMI noise and **widens the deadband automatically**.
   - Prevents ghost fires caused by the MOSFETs' own switching kick.
   - ⚠️ **Run with the gearbox assembled and bottle connected** — noise depends on physical setup.

2. **Capture Released**
   - Trigger fully released → **Capture Released**.

3. **Capture Pressed**
   - Trigger fully pulled → **Capture Pressed**.

4. FCU computes a central threshold and a hysteresis band based on both points + measured noise.

5. **Save slot**.

### Hall trigger tuning

| Symptom | Likely cause | Fix |
|---|---|---|
| Ghost fires | Solenoid EMI bigger than deadband | Re-run noise calibration |
| Fires before fire point | Threshold too low | Recapture at a deeper pull point |
| Won't fire even at full pull | Threshold too high or magnet too far | Recapture, or move magnet closer |
| Flickers at threshold | Hysteresis band too narrow | Use full calibration instead of single-point |

---

## 8. Hall selector — step-by-step calibration

A Hall selector enables **3 physical positions** (classic SAFE/SEMI/FULL) or just more reliable 2-position reading.

**Expected hardware:** linear Hall sensor on the gearbox body, magnet on the selector plate. Output wired to **PIN_SEL** (GPIO 1).

### 8.1 Mode selection

**Input** section → "Selector type" → **Hall** → check "3 positions" if applicable → **Save**.

### 8.2 Calibration

**Selector calibration** section:

1. **Noise calibration** (optional but recommended) — same principle as the trigger.
2. Rotate selector to **Pos 1** → **Capture Pos 1**.
3. Rotate to **Pos 2** → **Capture Pos 2**.
4. (if 3-pos) Rotate to **Pos 3** → **Capture Pos 3**.
5. FCU computes thresholds between each pair of samples.
6. **Save slot**.

### 8.3 Assign modes

Under **Selector → Pos 1/2/3 Mode**, choose a firing mode per position. These fields apply regardless of input type (digital or Hall).

---

## 9. First use and web panel access

### First-boot

On first power-up (or after flashing a build that bumped the schema), NVS is re-initialized with defaults. Two long beeps (BOOT + READY) confirm first-boot; subsequent power-ups only play BOOT.

**Initial defaults:**

- 3 slots named `Slot 1`, `Slot 2`, `Slot 3`
- D8PA, 2-pos, Pos1=SEMI, Pos2=FULL
- Digital trigger (microswitch, active-LOW)
- Timings `DN=18 ms / DR=26 ms / DP=25 ms / DB=100 units (10 ms)`
- SSID = `BCGA_FCU_STR` or `BCGA_FCU_PRO`
- WiFi password = `12345678`

### Open the panel

Connect your phone/laptop to the AP and open any URL — the captive DNS redirects to the panel. Fixed address: `http://192.168.4.1`.

**Turn the AP on:**

| Variant | Gestures |
|---|---|
| **STR** | Hold the trigger for 5 s during the first 5 s after boot; OR 5 trigger pulls in SAFE within 3 s |
| **PRO** | Dedicated WiFi button; OR the same gestures above |

AP shuts off automatically after **10 min of web inactivity** (3 beeps).

---

## 10. Slots

3 independent slots. Each one holds **everything**: firing type (S8PA/D8PA), timings, selector, Hall calibrations, flags. Switch via the buttons at the top of the page.

The FCU remembers the last slot used and returns to it after reboot.

**Typical use:**
- Slot 1 = your main setup
- Slot 2 = aggressive tune (higher ROF, tighter DR)
- Slot 3 = backup / test

---

## 11. Useful flags

- **Invert trigger** — check if your microswitch is active-HIGH (rare).
- **Swap MOS** (D8PA only) — swaps SOL1↔SOL2 **in software**. Use if you soldered them backwards and don't want to desolder.
- **Silent mode** — mutes beeps only *during firing*. Mode-change beep still plays.

---

## 12. Limit rate of fire

### ROF limit (rounds/sec)

Global ceiling. If your timings allow 25 rps but you want 15 rps, set `15`. A delay is inserted automatically between shots.

- `0` = unlimited (ROF determined purely by the timings)
- Too low = FCU ignores your FULL pull until the limit allows the next shot

### Semi ROF (ms)

Cooldown **between SEMI pulls**. Prevents trigger spam (fast finger).

- `0` = disabled
- `150 ms` = caps at ~6 SEMI pulls/sec
- Does not affect FULL/BURST

---

## 13. Diagnostics and WiFi

### MOS test

- **Test MOS 1** — pulses MOSFET 1 for 2 s.
- **Test MOS 2** — pulses MOSFET 2 for 2 s (honors `Swap MOS`).

Use with a multimeter, LED, or solenoid to verify wiring.

> Refuses if the FCU is actively firing.

### Buzzer test

Plays each beep type. Useful to learn the sounds before a game. On STR the piezo is not populated by default; the code still runs but stays silent unless you add one to PIN_BUZZER.

### Change WiFi password

**WiFi** section → new password (min **8 characters**) → **Save**. Persists in NVS. Only resets on factory reset.

### Factory reset

Two ways:
1. Flash a build that bumps `STORAGE_INIT_VERSION` (NVS is wiped on next boot automatically).
2. If your build exposes **Advanced → Factory reset** on the panel.

---

## 14. Deep sleep, inactivity alarm and debug mode

### Inactivity alarm (NEW)

After **60 minutes without any user activity** (trigger pull, WiFi button press, selector change, or web UI interaction), the FCU enters an alarm sequence designed to grab your attention before sleeping:

- **6 alarm cycles**, each consisting of:
  - 5 minutes of short beeps every 30 seconds
  - Then 1 hour of deep sleep
  - Then wake and start the next cycle
- After all 6 cycles complete (~6 hours total), the FCU enters **permanent deep sleep**
- **Any activity resets the counter** and returns to normal operation

This protects against forgetting the FCU powered on, but **does not preserve the battery indefinitely** — see below.

### Deep sleep behavior

When the FCU enters deep sleep:
- ESP32-C3 draws **<10 µA**
- The voltage regulators on the PRO board (MP2315 buck, MT3608 boost, AMS1117 LDO) **continue drawing ~12-18 mA** as long as the battery is connected
- Total board draw in deep sleep: ~12-18 mA (regulators only)

A 1000 mAh battery will fully discharge in **2-3 days** if left connected, even if the ESP32 is sleeping.

> ⚠️ **The only way to truly preserve the battery between sessions is to physically disconnect it.**

### Wake-up

After deep sleep, the next trigger pull **wakes the MCU via full reboot** — the second pull is the one that actually fires.

### Debug mode (5 min)

For bench testing, edit `firmware/BCGA_FCU_{STR,PRO}/config.h` and uncomment:

```c
#define DEEP_SLEEP_DEBUG
```

Inactivity timeout drops from 60 min to 5 min. **Re-comment before shipping to production.**

### Battery lockout (PRO only)

The PRO variant monitors battery voltage and protects against deep discharge through **lockout mode** (not automatic cut-off — see below).

**Thresholds (per cell):**
- **Low warning**: 3.5 V/cell — single beep every 10 seconds
- **Critical**: 3.2 V/cell — single beep every 5 seconds
- **Cut (lockout)**: 3.0 V/cell — firing blocked, beep every 5 seconds

When lockout is triggered:
1. Firing is permanently disabled until reboot (battery reconnect)
2. Beep alert sounds every 5 seconds
3. After ~30 minutes of beeping (360 cycles), FCU enters permanent deep sleep
4. **Battery is not cut off** — you must physically disconnect to prevent further discharge

> ⚠️ **The PRO variant V2.1 does NOT have automatic battery cut-off.** Earlier versions (V2.0 and earlier) had a kill latch circuit that physically disconnected the battery — this was removed in V2.1 to simplify the circuit and eliminate a bootstrap reliability issue. The lockout mode + audible alarm is the new protection mechanism, but it relies on the user disconnecting the battery.

---

## 15. STR vs PRO differences

| Feature | STR | PRO |
|---|:---:|:---:|
| S8PA (1 solenoid) | ✅ | ✅ |
| D8PA (2 solenoids) | ✅ | ✅ |
| All modes (SAFE/SEMI/FULL/BURST2-4) | ✅ | ✅ |
| 3-pos Hall selector | ✅ | ✅ |
| Full Hall calibration | ✅ | ✅ |
| Web panel + AP gesture opening | ✅ | ✅ |
| 60 min deep sleep | ✅ | ✅ |
| Inactivity alarm (6 cycles → permanent sleep) | ✅ | ✅ |
| **Battery voltage read** | ❌ | ✅ |
| **Battery lockout** (firing block at critical V) | ❌ | ✅ |
| **Kill latch** (LiPo deep-discharge protection) | ❌ | ❌ (V2.1: lockout) |
| **Dedicated WiFi button** | ❌ | ✅ |
| **Onboard buzzer** | ❌ | ✅ |

---

## 16. BCGA FCU vs commercial FCUs

This section compares the BCGA FCU to the main commercial FCUs in the airsoft HPA market (PolarStar REV3, Wolverine BLINC, GATE TITAN II, Gorilla FCU). It is factual and includes both advantages and gaps.

### 16.1 Where the BCGA FCU wins

1. **Native WiFi vs Bluetooth.** Configure from any browser on any device — iOS, Android, PC, Linux, anything that opens a web page. No app install, no pairing, no vendor lock-in. TITAN II (BLE 5.2), BLINC and Gorilla all require proprietary apps.

2. **4 independent timings (DN/DR/DP/DB).** Each phase of the D8PA cycle has its own parameter. Feeding (DN), sealing (DR), FPS (DP) and post-shot debounce (DB) are tuned independently without trade-offs. Commercial FCUs top out at 3 user-facing timings (REV3 in dual-solenoid mode: dP/dN/dr) or a single dwell on single-solenoid setups. No commercial FCU exposes a dedicated post-shot debounce.

3. **Automatic Hall trigger EMI noise calibration.** The only FCU on the market with a routine that fires the solenoids dry and measures the EMI kick on the ADC, widening the Hall deadband automatically. Eliminates ghost fires without sacrificing trigger sensitivity.

4. **3 complete independent slots.** Each slot stores **everything** — engine type, all 4 timings, selector config, individual Hall calibrations, flags. Switching slots = full game-profile switch.

5. **Open-source, GPL v3.** All code is public. Audit, modify, compile and flash without depending on proprietary firmware or a vendor app. TITAN II, BLINC, REV3 and Gorilla are fully closed.

6. **Hall trigger with 2-point calibration + automatic hysteresis.** Captures the exact fire point and computes a hysteresis band from real measurements — not fixed values. No mechanical potentiometer.

7. **3-position Hall selector.** SAFE/SEMI/FULL via a wear-free Hall sensor. Each position freely configurable to any mode (including BURST 2/3/4).

8. **Drastically lower BOM cost.** The STR can be built from easily sourced THT parts for a fraction of any commercial Bluetooth FCU.

9. **Live theoretical ROF in the UI.** The web panel shows maximum achievable ROF as you move the sliders — no chrono needed for a first-pass estimate.

10. **Per-slot S8PA/D8PA switching in software.** Each of the 3 slots independently stores the engine type — flip a slot between S8PA and D8PA directly from the web UI, no hardware change. Commercial FCUs that cover both architectures (TITAN II with PULSAR S/D, Gorilla) do it through separate dedicated cable harnesses per engine, not per-slot software switching.

### 16.2 Honest limitations

Buyers need to know these before choosing the BCGA FCU:

1. **No binary trigger.** Not implemented. Available on Wolverine BLINC, GATE TITAN II and Gorilla FCU.
2. **No tournament lock with password.** Workaround: set Semi ROF high and ROF limit low before an event. Available on TITAN II (Expert) and Gorilla.
3. **No automatic battery cut-off.** The PRO variant enters lockout mode (firing disabled, continuous alarm) at critical battery voltage, but does not physically cut the battery — user must disconnect manually. The STR variant has no battery voltage read at all. Use with caution on 2S/3S packs without external protection.
4. **Battery drains while plugged in.** The voltage regulators draw ~12-18mA continuously. A 1000mAh pack discharges fully in 2-3 days if left connected. Disconnect between sessions.
5. **First pull after deep sleep wakes via reboot.** After 60 min of idle the FCU deep-sleeps. The next pull wakes the MCU through a full reboot — the **second** pull is the one that actually fires. Different from FCUs that sleep via MOSFET gate-hold.

### 16.3 Side-by-side comparison

| Dimension | BCGA FCU STR/PRO | PolarStar REV3 | Wolverine BLINC | GATE TITAN II | Gorilla FCU |
|---|---|---|---|---|---|
| MCU | ESP32-C3 | Proprietary | Proprietary | ARM + BLE 5.2 | Proprietary + BLE |
| License | **GPL v3 (open-source)** | Proprietary | Proprietary | Proprietary | Proprietary |
| Single-solenoid | ✅ (S8PA) | ✅ (FCF1) | ✅ | ✅ (PULSAR S) | ✅ |
| Dual-solenoid | ✅ (D8PA) | ✅ (FCFE) | ❌ (single only) | ✅ (PULSAR D) | ✅ |
| Independent timings | **4 (DN/DR/DP/DB)** | 3 (dual) / 1 (single) | 1 + autotune | Auto cycle-sync or manual | Separate SEMI/AUTO dwells |
| Interface | **Web UI over WiFi** | LCD + joystick | BLE app | BLE 5.2 app | BLE app |
| App required | **No** | No | ✅ required | ✅ required | ✅ required |
| Hall noise calibration | **✅ unique on the market** | ❌ | ❌ | ❌ | ❌ |
| Configuration slots | **3 complete** | 1 set | 1 profile | Per-engine profiles | 1 set |
| Binary trigger | ❌ | Hack (burst=01) | ✅ | ✅ | ✅ |
| Tournament lock | ❌ | — | ❌ | ✅ (Expert) | ✅ |
| Approximate cost | **~R$50–100 BOM** | ~US$80 FCU | ~US$160 | ~US$300–440 combo | ~US$200 |
| Deep sleep | ✅ 60 min + alarm | — | ✅ | ✅ | — |
| Battery cut-off | ❌ (PRO: lockout) | — | — | — | — |

### 16.4 Who the BCGA FCU is for

- **DIY HPA builders** who want full control over the firing cycle with 4 independent timings.
- **Field installers** without a specific Bluetooth app on their phone — any browser works.
- **Budget-conscious makers** building an S8PA/D8PA engine from scratch.
- **Open-source advocates** who won't ship a closed-firmware gun.
- **Anyone who wants 3 distinct game profiles** in a single FCU (skirmish, DMR, CQB).

The BCGA FCU is **not** the right choice if you need binary trigger or password-locked tournament mode out of the box — pick TITAN II, BLINC or Gorilla for those.

---

## License

GPL v3 — see [LICENSE](../LICENSE).
