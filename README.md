# BCGA FCU Starter

Open source Fire Control Unit (FCU) for airsoft replicas, based on the ESP32-C3 SuperMini.
Designed and maintained by **BCGA Airsoft**.

> 🌐 **Mais informações, manual de montagem, suporte e produto físico:**
> [bcgaairsoft.com/fcu/str](https://bcgaairsoft.com/fcu/str)
>
> 🌐 **Full documentation, build guide, support and physical product:**
> [bcgaairsoft.com/fcu/str](https://bcgaairsoft.com/fcu/str)

---

## 🇧🇷 Português

A **BCGA FCU Starter** é a versão open source da nossa unidade de controle de disparo digital. Foi desenhada para ser simples, replicável e didática — ideal para makers, hobbyistas e quem quer aprender eletrônica de airsoft.

Para o produto comercial com Bluetooth, multi-módulo e proteções avançadas, veja a [FCU Pro](https://bcgaairsoft.com/fcu/pro).

### Estrutura do repositório

```
BCGA_FCU_STR/
├── MotherBoard/              # Placa principal (MOBO)
│   ├── BOM BCGA_FCU_STR.xlsx
│   ├── CPL BCGA_FCU_STR.xlsx
│   ├── GERBER BCGA_FCU_STR.zip
│   ├── SCH BCGA_FCU_STR.pdf
│   └── STEP BCGA_FCU_STR.step
├── DaugtherBoard_BCGA/       # Daughterboard A (DBA) — para gearbox BCGA
│   ├── BOM DBA.xlsx
│   ├── CPL DBA.xlsx
│   ├── Gerber DBA.zip
│   ├── SCH DBA.pdf
│   └── STEP DBA.step
├── Gearbox_DaughterBoard/    # Daughterboard B (DBB) — para gearboxes V2 standard
│   ├── BOM DBB.xlsx
│   ├── CPL.xlsx
│   ├── GERBER_DBB.zip
│   ├── SCH DBB.pdf
│   └── STEP DBB.step
└── LICENSE                   # GPL v3
```

> ⏳ **Em breve:** `firmware/` — código-fonte para ESP32-C3 SuperMini.

### Especificações

| Item | Valor |
|---|---|
| MCU | ESP32-C3 SuperMini |
| MOSFETs | IRLZ44NPBF (TO-220) |
| Conector principal | JST 8 pinos (A1501WR-S-8P) |
| Tensão de operação | 7.4V — 11.1V (LiPo 2S/3S) |
| Modos de tiro | Semi (firmware) |
| Licença | GPL v3 |

### Como usar

#### Opção 1 — Baixar pelo site (mais fácil)

Acesse [bcgaairsoft.com/fcu/str](https://bcgaairsoft.com/fcu/str) e clique em **Baixar ZIP**. Você recebe o pacote completo direto, sem precisar criar conta no GitHub.

#### Opção 2 — Clonar pelo Git

```bash
git clone https://github.com/BCGAdesings/BCGA_FCU_STR.git
cd BCGA_FCU_STR
```

#### Opção 3 — Comprar o produto físico

A BCGA também vende o kit completo (PCB + componentes) através do site, com envio para o Brasil. Veja em [bcgaairsoft.com/fcu/str](https://bcgaairsoft.com/fcu/str).

### Fabricar a sua

1. Suba o arquivo `GERBER` da placa desejada para uma fábrica de PCBs (JLCPCB, PCBWay, etc.)
2. Compre os componentes do BOM correspondente (a maioria está disponível na LCSC)
3. Solde os componentes seguindo o `SCH` (schematic)
4. Grave o firmware no ESP32-C3 (em breve neste repo)

> ⚠️ Requer conhecimento básico de solda SMD. Para uma versão plug-and-play, considere a [FCU Pro](https://bcgaairsoft.com/fcu/pro).

### Licença

[**GPL v3**](LICENSE) — você pode fabricar, modificar, vender e distribuir livremente, desde que mantenha a mesma licença em derivados e dê crédito ao projeto original.

### Contribuindo

Issues, pull requests e forks são bem-vindos. Para suporte técnico ou dúvidas de uso, prefira abrir uma issue aqui no GitHub.

---

## 🇬🇧 English

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
| Fire modes | Semi (firmware) |
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
