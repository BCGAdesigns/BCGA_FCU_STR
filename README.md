# BCGA FCU

Open source Fire Control Unit (FCU) for airsoft HPA replicas, based on the ESP32-C3 SuperMini. Designed and maintained by **BCGA Airsoft**.

Supports both the **STR** (Starter, perfboard) and **PRO** (commercial) variants, plus **S8PA** (1 solenoid) and **D8PA** (2 solenoid) gearboxes — configurable per slot via the web UI.

> 🌐 **Info, manual de montagem, suporte e produto físico / Build guide, support and physical product:**
> [bcgaairsoft.com/fcu/str](https://bcgaairsoft.com/fcu/str) · [bcgaairsoft.com/fcu/pro](https://bcgaairsoft.com/fcu/pro)

---

## 🇧🇷 Português

A **BCGA FCU Starter** é a versão open source da nossa unidade de controle de disparo digital. Simples, replicável e didática — ideal para makers, hobbyistas e quem quer aprender eletrônica de airsoft. Para o produto comercial com proteções avançadas, veja a [FCU Pro](https://bcgaairsoft.com/fcu/pro).

### 📖 Manual de uso

- **PT-BR:** [`firmware/MANUAL_PT.md`](firmware/MANUAL_PT.md) · [`firmware/MANUAL_PT.pdf`](firmware/MANUAL_PT.pdf)
- Cobre: tuning dos timings DN/DR/DP/DL, calibração do gatilho Hall, seletor 2/3-pos, modos de tiro, diagnóstico, diferenças STR vs PRO.

### Estrutura do repositório

```
BCGA_FCU_STR/
├── MotherBoard/              # Placa principal (MOBO)
├── DaugtherBoard_BCGA/       # Daughterboard A — gearbox BCGA
├── Gearbox_DaughterBoard/    # Daughterboard B — V2 standard
├── firmware/
│   ├── BCGA_FCU_STR/         # Sketch Arduino — variante Starter
│   ├── BCGA_FCU_PRO/         # Sketch Arduino — variante Pro
│   ├── MANUAL_PT.md / .pdf   # Manual em português
│   └── MANUAL_EN.md / .pdf   # English manual
└── LICENSE                   # GPL v3
```

Cada pasta de placa contém: `BOM` (xlsx), `CPL` (xlsx), `GERBER` (zip), `SCH` (pdf) e `STEP` (modelo 3D).

### Especificações

| Item | Valor |
|---|---|
| MCU | ESP32-C3 SuperMini |
| MOSFETs | IRLZ44NPBF (TO-220) |
| Conector principal | JST 8 pinos (A1501WR-S-8P) |
| Tensão de operação | 7.4 V — 11.1 V (LiPo 2S/3S) |
| Tipos de gearbox | S8PA (1 sol) / D8PA (2 sol) |
| Modos de tiro | Safe / Semi / Full / Burst 2–4 |
| Configuração | Web UI (AP WiFi 192.168.4.1) |
| Licença | GPL v3 |

### Como usar

1. Baixa pelo site em [bcgaairsoft.com/fcu/str](https://bcgaairsoft.com/fcu/str) (opção mais fácil) ou `git clone` deste repositório.
2. Monta a placa seguindo `SCH` + `BOM` + `GERBER`.
3. Flasha o firmware da pasta `firmware/BCGA_FCU_STR/` (ou `_PRO/`) via Arduino IDE (ESP32-C3 Dev Module, USB CDC On Boot = Enabled).
4. Na primeira ligada, a FCU sobe um AP WiFi — conecta o celular em `BCGA_FCU_STR` (senha `12345678`) e abre qualquer URL.
5. Configura os 3 slots pelo painel web. Detalhes completos em [`firmware/MANUAL_PT.md`](firmware/MANUAL_PT.md).

---

## 🇬🇧 English

The **BCGA FCU** is the open source version of our digital fire control unit. Simple, replicable and educational — ideal for makers, hobbyists and anyone learning airsoft electronics. For the commercial product with advanced protections, see the [FCU Pro](https://bcgaairsoft.com/fcu/pro).

### 📖 User manual

- **English:** [`firmware/MANUAL_EN.md`](firmware/MANUAL_EN.md) · [`firmware/MANUAL_EN.pdf`](firmware/MANUAL_EN.pdf)
- Covers: DN/DR/DP/DL timing tuning, Hall trigger calibration, 2/3-pos selector, fire modes, diagnostics, STR vs PRO differences.

### Repository structure

```
BCGA_FCU_STR/
├── MotherBoard/              # Main board (MOBO)
├── DaugtherBoard_BCGA/       # Daughterboard A — for the BCGA gearbox
├── Gearbox_DaughterBoard/    # Daughterboard B — for V2 standard
├── firmware/
│   ├── BCGA_FCU_STR/         # Arduino sketch — Starter variant
│   ├── BCGA_FCU_PRO/         # Arduino sketch — Pro variant
│   ├── MANUAL_PT.md / .pdf   # Portuguese manual
│   └── MANUAL_EN.md / .pdf   # English manual
└── LICENSE                   # GPL v3
```

Each board folder contains: `BOM` (xlsx), `CPL` (xlsx), `GERBER` (zip), `SCH` (pdf) and `STEP` (3D model).

### Specifications

| Item | Value |
|---|---|
| MCU | ESP32-C3 SuperMini |
| MOSFETs | IRLZ44NPBF (TO-220) |
| Main connector | JST 8-pin (A1501WR-S-8P) |
| Operating voltage | 7.4 V — 11.1 V (2S/3S LiPo) |
| Gearbox types | S8PA (1 sol) / D8PA (2 sol) |
| Fire modes | Safe / Semi / Full / Burst 2–4 |
| Configuration | Web UI (AP WiFi 192.168.4.1) |
| License | GPL v3 |

### How to use

1. Download from [bcgaairsoft.com/fcu/str](https://bcgaairsoft.com/fcu/str) (easiest) or `git clone` this repo.
2. Assemble the board using `SCH` + `BOM` + `GERBER`.
3. Flash the firmware from `firmware/BCGA_FCU_STR/` (or `_PRO/`) via Arduino IDE (ESP32-C3 Dev Module, USB CDC On Boot = Enabled).
4. On first boot the FCU brings up a WiFi AP — connect your phone to `BCGA_FCU_STR` (password `12345678`) and open any URL.
5. Configure the 3 slots from the web panel. Full details in [`firmware/MANUAL_EN.md`](firmware/MANUAL_EN.md).

### Build your own

1. Upload the chosen board's `GERBER` file to a PCB fab (JLCPCB, PCBWay, etc.)
2. Buy the components from the matching `BOM` (most are available on LCSC)
3. Solder the components following the `SCH` (schematic)
4. Flash the firmware to the ESP32-C3 via Arduino IDE

> ⚠️ Requires basic SMD soldering skills. For a plug-and-play version, consider the [FCU Pro](https://bcgaairsoft.com/fcu/pro).

### License

[**GPL v3**](LICENSE) — you can manufacture, modify, sell and distribute freely, as long as derivatives keep the same license and credit the original project.

### Contributing

Issues, pull requests and forks are welcome. For technical support or usage questions, please open an issue on GitHub.

---

**BCGA Airsoft** — [bcgaairsoft.com](https://bcgaairsoft.com)
