# BCGA FCU — User Manual (EN)

Open-source firmware for the ESP32-C3 SuperMini. Two variants: **STR** (Starter, perfboard) and **PRO** (commercial product). The web UI and firing logic are nearly identical — differences are listed at the end.

This manual is practical: each section leads with **what to tweak to get what result**.

> 🇧🇷 Versão em português: [MANUAL_PT.md](./MANUAL_PT.md)

---

## Table of contents

1. [Understand the FCU in 1 minute](#1-understand-the-fcu-in-1-minute)
2. [The 4 timings — tuning guide](#2-the-4-timings--tuning-guide)
3. [Recommended tuning workflow](#3-recommended-tuning-workflow)
4. [Firing type: S8PA vs D8PA](#4-firing-type-s8pa-vs-d8pa)
5. [Selector — 2 or 3 positions](#5-selector--2-or-3-positions)
6. [Hall trigger — step-by-step calibration](#6-hall-trigger--step-by-step-calibration)
7. [Hall selector — step-by-step calibration](#7-hall-selector--step-by-step-calibration)
8. [First use and web panel access](#8-first-use-and-web-panel-access)
9. [Slots](#9-slots)
10. [Useful flags (invert trigger, swap MOS, silent)](#10-useful-flags)
11. [Limit rate of fire (ROF limit and Semi ROF)](#11-limit-rate-of-fire)
12. [Diagnostics and WiFi](#12-diagnostics-and-wifi)
13. [Deep sleep and debug mode](#13-deep-sleep-and-debug-mode)
14. [STR vs PRO differences](#14-str-vs-pro-differences)

---

## 1. Understand the FCU in 1 minute

The FCU drives **1 or 2 solenoids** that replace the mechanical trigger of an HPA gearbox. For each shot it sends electrically calibrated pulses to the solenoids, in the correct order, with the correct waits between them. Tuning = adjusting those pulses for your specific setup (pressure, bucking, BB mass).

- **S8PA** (1 solenoid): F2/Pulsar-style gearboxes. Only the poppet is driven.
- **D8PA** (2 solenoids): Jack/Backdraft-style gearboxes. Nozzle and poppet driven separately.

---

## 2. The 4 timings — tuning guide

All in **milliseconds**, allowed range **2–80 ms** per field. The page shows the theoretical ROF live.

### DN — Nozzle Dwell (D8PA only)

Length of the **nozzle (SOL2)** pulse. This is how long the nozzle stays open for the BB to drop into the chamber.

| If DN increases | If DN decreases |
|---|---|
| ✅ easier feeding (heavy BBs, low pressure, cold shot) | ✅ saves gas, prevents double-feed |
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

Length of the **poppet (SOL1)** pulse. This is how long gas can flow through the nozzle pushing the BB.

| If DP increases | If DP decreases |
|---|---|
| ✅ **higher FPS** (more gas behind the BB) | ✅ saves gas (more shots per bottle/mag) |
| ❌ gas waste after BB has left | ✅ higher ROF (valve closes fast) |
| ❌ may delay poppet recycle | ❌ **FPS below target**, weak shot |

**Rule of thumb:** with a chrono, tune DP up to your target FPS. Anything more just wastes gas.

### DL — Post-shot Delay (D8PA only)

Wait after the poppet closes. Time for the **BB to clear the barrel** before the next cycle.

| If DL increases | If DL decreases |
|---|---|
| ✅ **better accuracy** (BB has left before the next nozzle opens) | ✅ **higher ROF** |
| ✅ bucking recovers undisturbed | ❌ BB still in the barrel when the next nozzle pulses → **flyers / inconsistent FPS** |
| ❌ lower ROF | ❌ reduced bucking life |

**Rule of thumb:** full-auto at a 20 m target. If flyers appear, bump DL by 2 ms at a time.

### Defaults (generic D8PA, Jack-style, 110 psi, 0.28 g BB)

```
DN = 18 ms    DR = 26 ms    DP = 25 ms    DL = 10 ms
```

Safe starting point. Tune from there.

---

## 3. Recommended tuning workflow

Follow this order — each step depends on the previous one being stable.

1. **Feeding (DN)** — slow SEMI. Lower DN until you get empty shots. Add 2 ms back.
2. **Sealing (DR)** — fast SEMI. If chrono wobbles, raise DR 2 ms at a time.
3. **FPS (DP)** — with chrono, raise DP to hit target FPS. Don't go higher.
4. **Accuracy (DL, D8PA only)** — full-auto on a target. If flyers, raise DL.
5. **Cadence (ROF limit)** — optional. Caps maximum ROF independently of timings.
6. **Anti-spam (Semi ROF)** — optional. Minimum time between SEMI pulls.

> ⚠️ Always verify with a chrono. FPS drifts with bottle temperature / gas level.

---

## 4. Firing type: S8PA vs D8PA

Chosen per slot in the first section of the panel.

- **S8PA** — only the poppet is pulsed. Cycle: `DP → DR → repeat`. Use with F2, Pulsar, or any 1-solenoid gearbox. The DN, DL, MOS swap fields and the SOL 2 test button are hidden.
- **D8PA** — separate nozzle + poppet. Cycle: `DN → DR → DP → DL → repeat`. Use with Jack, Backdraft, and any 2-solenoid system.

---

## 5. Selector — 2 or 3 positions

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

## 6. Hall trigger — step-by-step calibration

Use Hall for a **wear-free trigger** with an **adjustable fire point**.

**Expected hardware:** linear Hall sensor (DRV5055 or similar) powered from 3.3 V, output wired to **PIN_TRIG** (GPIO 0 on both variants). Magnet on the trigger lever.

### 6.1 Mode selection

**Input** section → "Trigger type" → **Hall** → **Save**.

### 6.2 Quick calibration (single-point)

Covers most cases — 30 seconds.

1. In the **Sensitivity** section, pull the trigger **to the exact fire point you want** and hold.
2. Click **Capture point**.
3. The FCU reads the ADC and builds a symmetric hysteresis band around that value.
4. Save the slot.

Fires before reaching the point → capture at a deeper pull.
Won't fire even at full pull → capture at a shallower pull.

### 6.3 Full calibration (maximum precision)

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

## 7. Hall selector — step-by-step calibration

A Hall selector enables **3 physical positions** (classic SAFE/SEMI/FULL) or just more reliable 2-position reading.

**Expected hardware:** linear Hall sensor on the gearbox body, magnet on the selector plate. Output wired to **PIN_SEL** (GPIO 1).

### 7.1 Mode selection

**Input** section → "Selector type" → **Hall** → check "3 positions" if applicable → **Save**.

### 7.2 Calibration

**Selector calibration** section:

1. **Noise calibration** (optional but recommended) — same principle as the trigger.
2. Rotate selector to **Pos 1** → **Capture Pos 1**.
3. Rotate to **Pos 2** → **Capture Pos 2**.
4. (if 3-pos) Rotate to **Pos 3** → **Capture Pos 3**.
5. FCU computes thresholds between each pair of samples.
6. **Save slot**.

### 7.3 Assign modes

Under **Selector → Pos 1/2/3 Mode**, choose a firing mode per position. These fields apply regardless of input type (digital or Hall).

---

## 8. First use and web panel access

### First-boot

On first power-up (or after flashing a build that bumped the schema), NVS is re-initialized with defaults. Two long beeps (BOOT + READY) confirm first-boot; subsequent power-ups only play BOOT.

**Initial defaults:**

- 3 slots named `Slot 1`, `Slot 2`, `Slot 3`
- D8PA, 2-pos, Pos1=SEMI, Pos2=FULL
- Digital trigger (microswitch, active-LOW)
- Timings `DN=18 / DR=26 / DP=25 / DL=10`
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

## 9. Slots

3 independent slots. Each one holds **everything**: firing type (S8PA/D8PA), timings, selector, Hall calibrations, flags. Switch via the buttons at the top of the page.

The FCU remembers the last slot used and returns to it after reboot.

**Typical use:**
- Slot 1 = your main setup
- Slot 2 = aggressive tune (higher ROF, tighter DR)
- Slot 3 = backup / test

---

## 10. Useful flags

- **Invert trigger** — check if your microswitch is active-HIGH (rare).
- **Swap MOS** (D8PA only) — swaps SOL1↔SOL2 **in software**. Use if you soldered them backwards and don't want to desolder.
- **Silent mode** — mutes beeps only *during firing*. Mode-change beep still plays.

---

## 11. Limit rate of fire

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

## 12. Diagnostics and WiFi

### MOS test

- **Test MOS 1** — pulses MOSFET 1 for 2 s.
- **Test MOS 2** — pulses MOSFET 2 for 2 s (honors `Swap MOS`).

Use with a multimeter, LED, or solenoid to verify wiring.

> Refuses if the FCU is actively firing.

### Buzzer test

Plays each beep type. Useful to learn the sounds before a game.

### Change WiFi password

**WiFi** section → new password (min **8 characters**) → **Save**. Persists in NVS. Only resets on factory reset.

### Factory reset

Two ways:
1. Flash a build that bumps `STORAGE_INIT_VERSION` (NVS is wiped on next boot automatically).
2. If your build exposes **Advanced → Factory reset** on the panel.

---

## 13. Deep sleep and debug mode

After **60 min of trigger inactivity** the FCU enters deep-sleep (<10 µA draw). The next trigger pull **wakes the MCU via full reboot** — the second pull actually fires.

### Debug mode (5 min)

For bench testing, edit `firmware/BCGA_FCU_{STR,PRO}/config.h` and uncomment:

```c
#define DEEP_SLEEP_DEBUG
```

Timeout drops from 60 min to 5 min. **Re-comment before shipping to production.**

---

## 14. STR vs PRO differences

| Feature | STR | PRO |
|---|:---:|:---:|
| S8PA (1 solenoid) | ✅ | ✅ |
| D8PA (2 solenoids) | ✅ | ✅ |
| All modes (SAFE/SEMI/FULL/BURST2-4) | ✅ | ✅ |
| 3-pos Hall selector | ✅ | ✅ |
| Full Hall calibration | ✅ | ✅ |
| Web panel + AP gesture opening | ✅ | ✅ |
| 60 min deep sleep | ✅ | ✅ |
| **Battery voltage read** | ❌ | ✅ |
| **Kill latch** (LiPo deep-discharge protection) | ❌ | ✅ |
| **Dedicated WiFi button** | ❌ | ✅ |
| **Onboard buzzer** | ❌ | ✅ |

---

## License

GPL v3 — see [LICENSE](../LICENSE).
