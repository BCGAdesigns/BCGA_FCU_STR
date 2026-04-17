

**BCGA FCU Starter** is the open source version of our digital fire control unit. Designed to be simple, replicable and educational — ideal for makers, hobbyists and anyone learning airsoft electronics.

For the commercial product with Bluetooth, multi-module and advanced protections, see the [FCU Pro](https://bcgaairsoft.com/fcu/pro).

### Repository structure

```
BCGA_FCU_STR/
├── MotherBoard/              # Main board (MOBO)
├── DaugtherBoard_BCGA/       # Daughterboard A (DBA) — for the BCGA gearbox
├── Gearbox_DaughterBoard/    # Daughterboard B (DBB) — for V2 standard gearboxes
└── LICENSE                   # GPL v3
```

Each board folder contains: `BOM` (xlsx), `CPL` (xlsx), `GERBER` (zip), `SCH` (pdf) and `STEP` (3D model).

> ⏳ **Coming soon:** `firmware/` — source code for the ESP32-C3 SuperMini.

### Specifications

| Item | Value |
|---|---|
| MCU | ESP32-C3 SuperMini |
| MOSFETs | IRLZ44NPBF (TO-220) |
| Main connector | JST 8-pin (A1501WR-S-8P) |
| Operating voltage | 7.4V — 11.1V (2S/3S LiPo) |
| Fire modes | Semi - Auto - Burst |
| License | GPL v3 |

### How to use

#### Option 1 — Download from the website (easiest)

Go to [bcgaairsoft.com/fcu/str](https://bcgaairsoft.com/fcu/str) and click **Download ZIP**. You get the full package directly — no GitHub account needed.

#### Option 2 — Clone via Git

```bash
git clone https://github.com/BCGAdesings/BCGA_FCU_STR.git
cd BCGA_FCU_STR
```

#### Option 3 — Buy the physical product

BCGA also sells the full kit (PCB + components) through the website, with shipping to Brazil. See [bcgaairsoft.com/fcu/str](https://bcgaairsoft.com/fcu/str).

### Build your own

1. Upload the chosen board's `GERBER` file to a PCB fab (JLCPCB, PCBWay, etc.)
2. Buy the components from the matching `BOM` (most are available on LCSC)
3. Solder the components following the `SCH` (schematic)
4. Flash the firmware to the ESP32-C3 (coming soon to this repo)

> ⚠️ Requires basic SMD soldering skills. For a plug-and-play version, consider the [FCU Pro](https://bcgaairsoft.com/fcu/pro).

### License

[**GPL v3**](LICENSE) — you can manufacture, modify, sell and distribute freely, as long as derivatives keep the same license and credit the original project.

### Contributing

Issues, pull requests and forks are welcome. For technical support or usage questions, please open an issue on GitHub.

---

**BCGA Airsoft** — [bcgaairsoft.com](https://bcgaairsoft.com)
