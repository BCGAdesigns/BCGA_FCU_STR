# BCGA FCU — Fire Control Unit open-source para HPA

[![Licença: GPL v3](https://img.shields.io/badge/License-GPL%20v3-blue.svg)](LICENSE)
[![Linguagem: C++](https://img.shields.io/badge/language-C%2B%2B-orange.svg)](firmware/)
[![Plataforma: ESP32-C3](https://img.shields.io/badge/platform-ESP32--C3-green.svg)](https://www.espressif.com/en/products/socs/esp32-c3)
[![Status: Ativo](https://img.shields.io/badge/status-active-brightgreen.svg)](https://github.com/BCGAdesigns/BCGA_FCU_STR)

> Fire Control Unit open-source para replicas airsoft HPA. Configuração nativa por WiFi, suporte a dual-solenoid, 4 timings de ciclo independentes e gatilho Hall com calibração automática de ruído EMI — única no mercado.

> 🇬🇧 Read in English: [README.md](README.md)

---

## O que é a BCGA FCU?

A **BCGA FCU** é uma Fire Control Unit digital para replicas airsoft HPA (High Pressure Air), baseada no ESP32-C3 SuperMini. Ela controla o(s) solenoide(s) que substituem o gatilho mecânico da gearbox HPA e permite ajustar cada fase do ciclo de disparo pelo browser via WiFi — sem precisar de app.

Duas variantes de hardware compartilham o mesmo firmware e a mesma interface web:

- **STR** (Starter) — montagem em perfboard / THT, componentes de fácil aquisição, ideal para makers e hobbyistas aprendendo eletrônica HPA.
- **PRO** — PCB SMD comercial com monitoramento de bateria, kill latch para LiPo, botão WiFi dedicado e buzzer integrado.

As duas variantes suportam **S8PA** (engines de 1 solenoide: F2, Pulsar, JACK-style) e **D8PA** (engines de 2 solenoides: Jack, Backdraft, Fusion-style) por slot. Você escolhe o tipo de engine por perfil — a UI se adapta automaticamente.

O projeto é desenhado, fabricado e mantido pela **[BCGA Airsoft](https://bcgaairsoft.com)** no Brasil. É **open-source sob GPL v3** — você pode auditar, modificar, fabricar e vender, desde que derivados mantenham a mesma licença e creditem o projeto original.

---

## Por que BCGA FCU?

Comparada com FCUs comerciais (PolarStar REV3, Wolverine BLINC, GATE TITAN II, Gorilla FCU), a BCGA FCU tem vantagens concretas — e algumas limitações honestas. Esta seção é factual; o comparativo lado-a-lado completo está em [`firmware/MANUAL_PT.md`](firmware/MANUAL_PT.md) seção 15.

### Onde a BCGA FCU ganha

1. **WiFi nativo vs Bluetooth.** Configure pelo browser de qualquer dispositivo — iOS, Android, PC, Linux. Sem instalar app, sem pareamento, sem vendor lock-in. O TITAN II (BLE 5.2) e o BLINC exigem apps proprietários específicos.

2. **4 timings independentes (DN/DR/DP/DB).** Cada fase do ciclo D8PA tem seu próprio parâmetro. Alimentação (DN), vedação (DR), FPS (DP) e debounce pós-tiro (DB) ajustam independente, sem trade-offs. FCUs comerciais single-solenoid usam dwell único.

3. **Calibração automática de ruído EMI no gatilho Hall.** Única FCU do mercado com rotina que dispara os solenoides a seco e mede o chute EMI no ADC, alargando automaticamente o deadband do Hall. Elimina ghost fires sem sacrificar sensibilidade.

4. **3 slots completos e independentes.** Cada slot armazena tudo — tipo de engine, os 4 timings, configuração de selector, calibrações Hall individuais, flags. Trocar slot = trocar perfil de jogo completo.

5. **Open-source, GPL v3.** Código totalmente aberto. Auditar, modificar, compilar e flashar sem depender de firmware proprietário ou app do fabricante. TITAN II, BLINC e REV3 são fechados.

6. **Gatilho Hall com calibração de 2 pontos + histerese automática.** Captura o ponto exato de disparo e calcula banda de histerese a partir de medições reais — não valores fixos. Sem potenciômetro mecânico.

7. **Seletor Hall de 3 posições.** SAFE/SEMI/FULL via sensor Hall sem desgaste mecânico. Cada posição livre para qualquer modo (inclusive BURST 2/3/4).

8. **BOM drasticamente mais barato.** O STR pode ser construído com componentes THT fáceis de achar por uma fração do custo de qualquer FCU Bluetooth comercial.

9. **ROF teórico em tempo real na UI.** O painel web mostra o ROF máximo alcançável enquanto você mexe nos sliders — sem cronógrafo para estimativa inicial.

10. **Suporte nativo D8PA + S8PA por slot.** Cada slot é independentemente S8PA ou D8PA. FCUs 3rd party (Gorilla, TITAN II) exigem chicotes adaptadores para controlar um engine de dois solenoides.

### Limitações honestas

- **Sem binary trigger.** Não implementado. Presente no Wolverine BLINC, GATE TITAN II e Gorilla FCU.
- **Sem tournament lock com senha.** Contorno: configurar Semi ROF alto + ROF limit baixo antes do evento. Presente no TITAN II (Expert) e Gorilla.
- **Kill latch e buzzer integrado apenas no PRO.** A variante STR não tem leitura de bateria nem corte de LiPo — use com cuidado em packs 2S/3S sem proteção externa.
- **Primeira puxada após deep sleep acorda via reboot.** Após 60 min de inatividade, a próxima puxada acorda o MCU por reboot completo — a **segunda** puxada dispara.

---

## Especificações de hardware

| Item | Valor |
|---|---|
| MCU | ESP32-C3 SuperMini |
| MOSFETs | IRLZ44NPBF (TO-220, THT) |
| Conector principal | JST 8 pinos (A1501WR-S-8P) — nativo do ecossistema D8PA |
| Tensão de operação | 7,4 V — 11,1 V (LiPo 2S/3S) |
| Tipos de engine | S8PA (1 solenoide) / D8PA (2 solenoides) |
| Modos de tiro | SAFE / SEMI / FULL / BURST 2 / BURST 3 / BURST 4 |
| Configuração | Web UI via AP WiFi (192.168.4.1) |
| Deep sleep | 60 min de inatividade → <10 µA |
| Licença | GPL v3 |

---

## Estrutura do repositório

```
BCGA_FCU_STR/
├── MotherBoard/              # Placa principal (MOBO) — BOM, CPL, GERBER, SCH, STEP
├── DaugtherBoard_BCGA/       # Daughterboard A — gearbox BCGA (S8PA ou D8PA)
├── Gearbox_DaughterBoard/    # Daughterboard B — V2 standard (conversão AEG)
├── firmware/
│   ├── BCGA_FCU_STR/         # Sketch Arduino — variante Starter
│   ├── BCGA_FCU_PRO/         # Sketch Arduino — variante Pro
│   ├── MANUAL_EN.md / .pdf   # Manual em inglês
│   └── MANUAL_PT.md / .pdf   # Manual em português (este LEIA-ME aponta pra cá)
├── README.md                 # Readme em inglês
├── LEIA-ME.md                # Readme em português (este arquivo)
└── LICENSE                   # GPL v3
```

Cada pasta de placa contém: `BOM` (xlsx), `CPL` (xlsx), `GERBER` (zip), `SCH` (pdf) e `STEP` (modelo 3D).

---

## Como usar

1. Consiga o hardware — compre a placa montada em [bcgaairsoft.com/fcu/str](https://bcgaairsoft.com/fcu/str) / [fcu/pro](https://bcgaairsoft.com/fcu/pro), ou clone este repositório e construa (veja seção seguinte).
2. Flashe o firmware correspondente da pasta `firmware/BCGA_FCU_STR/` ou `firmware/BCGA_FCU_PRO/` pelo Arduino IDE. Board: **ESP32-C3 Dev Module**, com **USB CDC On Boot = Enabled**.
3. No primeiro boot, a FCU sobe um AP WiFi.
   - SSID padrão: `BCGA_FCU_STR` ou `BCGA_FCU_PRO`
   - Senha padrão: `12345678`
4. Conecte o celular/notebook ao AP. Uma página captive abre em `http://192.168.4.1`.
5. Configure os 3 slots: tipo de engine (S8PA/D8PA), timings (DN/DR/DP/DB), seletor, flags. O workflow completo de tuning está em [`firmware/MANUAL_PT.md`](firmware/MANUAL_PT.md).

Para reabrir o AP depois:
- **STR** — segure o gatilho por 5 s durante os primeiros 5 s após o boot; OU dê 5 puxadas no gatilho em SAFE dentro de 3 s.
- **PRO** — aperte o botão WiFi dedicado; ou use os mesmos gestos do STR.

O AP desliga automaticamente após 10 min sem atividade web.

---

## Construa a sua

1. Suba o `GERBER` zip da placa escolhida para uma fabricante de PCB (JLCPCB, PCBWay, etc.).
2. Compre os componentes pelo `BOM` correspondente. A maioria está disponível na [LCSC](https://www.lcsc.com/).
3. Solde seguindo o `SCH` (esquemático).
4. Flashe o firmware de `firmware/BCGA_FCU_{STR,PRO}/` pelo Arduino IDE.
5. Calibre o gatilho Hall e o seletor pela interface web (seções 6 e 7 do manual).

> ⚠️ O **STR** requer habilidades básicas de solda THT (MOSFETs TO-220, headers passantes). O **PRO** requer habilidades básicas de solda SMD. Para uma placa pronta, compre a versão pré-montada em [bcgaairsoft.com](https://bcgaairsoft.com).

---

## Licença

[**GPL v3**](LICENSE). Você pode fabricar, modificar, vender e distribuir livremente, desde que derivados mantenham a mesma licença e creditem o projeto original. Forks fechados não são permitidos.

---

## Links

- **Páginas do produto:** [bcgaairsoft.com/fcu/str](https://bcgaairsoft.com/fcu/str) · [bcgaairsoft.com/fcu/pro](https://bcgaairsoft.com/fcu/pro)
- **Site principal:** [bcgaairsoft.com](https://bcgaairsoft.com)
- **Manual do usuário (português):** [firmware/MANUAL_PT.md](firmware/MANUAL_PT.md) · [PDF](firmware/MANUAL_PT.pdf)
- **Manual do usuário (inglês):** [firmware/MANUAL_EN.md](firmware/MANUAL_EN.md) · [PDF](firmware/MANUAL_EN.pdf)
- **Issues e suporte:** [GitHub Issues](https://github.com/BCGAdesigns/BCGA_FCU_STR/issues)

---

**BCGA Airsoft** — [bcgaairsoft.com](https://bcgaairsoft.com)
