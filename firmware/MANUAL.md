# BCGA FCU — Manual de Uso / User Manual

Firmware para ESP32-C3 SuperMini. Duas variantes: **STR** (Starter, perfboard) e **PRO** (produto comercial). O firmware e a interface web são quase idênticos — as diferenças estão listadas na seção [Diferenças entre variantes](#diferencas-entre-variantes--variant-differences).

---

## 🇧🇷 Português

### 1. Primeiro uso

Ao ligar a FCU pela primeira vez (ou após flashar uma versão nova), a NVS é inicializada com os defaults:

- **3 slots** de configuração (nomes `Slot 1` / `Slot 2` / `Slot 3`)
- **D8PA** (2 solenoides: nozzle + poppet) — padrão para ambas variantes
- Seletor **2 posições** (Semi em Pos 1 / Full em Pos 2)
- Gatilho **digital** (microswitch, active-LOW com pull-up interno)
- Timings: `DN=18`, `DR=26`, `DP=25`, `DL=10` ms
- WiFi: SSID `BCGA_FCU_STR` ou `BCGA_FCU_PRO`, senha `12345678`

Dois beeps longos (BOOT + READY) confirmam o first-boot; depois só toca BOOT a cada liga-desliga.

### 2. Abrir o painel Web

A FCU sobe um Access Point WiFi. Conecta o celular/notebook e abre `http://192.168.4.1` (o DNS captive redireciona qualquer domínio automaticamente — na maioria dos celulares abre a página sozinho).

**Como ligar o AP:**

| Variante | Gestos disponíveis |
|---|---|
| **STR** | Segurar o gatilho por 5 s durante os primeiros 5 s pós-boot; OU 5 puxadas em modo SAFE dentro de 3 s |
| **PRO** | Botão WiFi dedicado; OU os mesmos gestos acima |

O AP desliga automaticamente após **10 minutos sem atividade web** (3 beeps confirmam).

> **Atenção:** o firmware entra em **deep sleep após 60 min** sem puxar o gatilho. Uma puxada acorda a FCU (reboot completo); o segundo puxar já dispara.

### 3. Trocar a senha WiFi

Painel web → Seção **WiFi** → digite a nova senha (mínimo 8 caracteres) → **Salvar**. A senha persiste em NVS; só volta ao default num factory reset.

### 4. Slots

3 slots independentes. O seletor dos slots fica no topo da página. O slot ativo fica salvo em NVS — no próximo boot a FCU volta para o último usado.

Cada slot armazena **todos** os parâmetros abaixo de forma independente.

### 5. Tipo de disparo (S8PA / D8PA)

- **S8PA (1 solenoide)** — gearboxes tipo F2 / Pulsar. Cicla apenas o poppet: pulso DP → espera DR → repete.
- **D8PA (2 solenoides)** — gearboxes tipo Jack / Backdraft. Cicla nozzle e poppet: pulso **DN** (SOL2) → espera **DR** → pulso **DP** (SOL1) → espera **DL** → repete.

Escolher S8PA esconde automaticamente os campos DN, DL, swap MOS e o botão de teste do SOL 2.

### 6. Timings

| Parâmetro | O que é | Quando usar |
|---|---|---|
| **DN** (Nozzle Dwell) | Duração do pulso do nozzle (SOL2) | só D8PA |
| **DR** (Return) | D8PA: espera entre DN e DP (bucking sela). S8PA: descanso entre tiros | ambos |
| **DP** (Poppet) | Duração do pulso do poppet (SOL1) — **é o tiro** | ambos |
| **DL** (Post-shot Delay) | Espera após o poppet (BB sai do cano) | só D8PA |

Limites: **2–80 ms** em cada campo. O ROF teórico é calculado em tempo real no painel.

### 7. Seletor

- **2 posições** (padrão): apenas Pos 1 / Pos 2. Cada uma mapeia para um dos modos: SAFE, SEMI, FULL, BURST2, BURST3, BURST4.
- **3 posições**: ativar obriga sensor Hall no seletor. Libera Pos 3.

Cada posição é configurável de forma independente. Ex: Pos 1 = SAFE, Pos 2 = SEMI, Pos 3 = FULL.

### 8. Entrada — gatilho e seletor

- **Tipo**: Switch digital (microswitch) ou Hall analógico. Pode ser misto (gatilho digital + seletor Hall, etc.).
- **Inverter gatilho**: marca se o microswitch é active-HIGH.
- **Swap MOS** (só D8PA): inverte SOL1↔SOL2 eletricamente — útil se soldaste invertido.
- **Silent mode**: silencia os beeps durante o tiro (troca de modo continua apitando).

### 9. Sensibilidade do gatilho Hall

**Captura de ponto único**: puxa até o ponto de disparo desejado, clica "Capturar". A FCU lê o ADC e calcula uma banda de histerese automática em torno daquele valor.

### 10. Calibração completa (Hall)

Para quem quer máxima precisão:

1. **Calibração de ruído**: a FCU dispara um ciclo completo sem BB, medindo o ruído EMI dos solenoides. Alarga automaticamente a banda morta do Hall para não disparar fantasma.
2. **Solto / Pressionado**: grava dois pontos do gatilho; a FCU calcula o threshold.
3. **Seletor 3 posições**: grava uma amostra em cada posição física.

### 11. Diagnóstico

- **Test MOS 1 / MOS 2**: pulsa o MOSFET por 2 s. Use para testar a fiação com um multímetro, LED ou solenoide. *Não funciona com a FCU disparando.*
- **Buzzer test**: toca cada tipo de beep para você conhecer os sons.

### 12. ROF limit e Semi ROF

- **ROF limit** (rounds/sec): teto geral. 0 = sem limite (roda no máximo que os DN/DR/DP/DL permitem).
- **Semi ROF** (ms): cooldown *entre puxadas em semi*. Evita trigger-spam. 0 = desabilitado.

### 13. Factory reset

Bump-e-flasha uma build nova (incrementamos `STORAGE_INIT_VERSION` quando muda schema) OU: coloca-se o WiFi em modo AP, acessa o painel e vai em **Avançado → Reset de fábrica** (se presente nessa versão).

### 14. Modo debug do deep-sleep

Para bancada, edita `firmware/BCGA_FCU_{STR,PRO}/config.h` e descomenta:

```c
#define DEEP_SLEEP_DEBUG
```

Isso baixa o timeout de 60 min para 5 min. Lembra de **re-comentar antes de entregar para produção**.

### Diferenças entre variantes

| Feature | STR | PRO |
|---|:---:|:---:|
| S8PA (1 solenoide) | ✅ | ✅ |
| D8PA (2 solenoides) | ✅ | ✅ |
| Leitura de bateria | ❌ | ✅ |
| Kill latch (protege LiPo de descarga profunda) | ❌ | ✅ |
| Botão WiFi dedicado | ❌ | ✅ |
| Buzzer onboard | ❌ | ✅ |
| Gestos do gatilho para abrir AP | ✅ | ✅ |
| Deep sleep após 60 min | ✅ | ✅ |

---

## 🇬🇧 English

### 1. First boot

On first power-up (or after flashing a new version), NVS is initialized with defaults:

- **3 config slots** (named `Slot 1` / `Slot 2` / `Slot 3`)
- **D8PA** (2 solenoids: nozzle + poppet) — default on both variants
- **2-position selector** (Semi on Pos 1 / Full on Pos 2)
- **Digital trigger** (microswitch, active-LOW with internal pull-up)
- Timings: `DN=18`, `DR=26`, `DP=25`, `DL=10` ms
- WiFi: SSID `BCGA_FCU_STR` or `BCGA_FCU_PRO`, password `12345678`

Two long beeps (BOOT + READY) confirm first-boot; subsequent boots only play BOOT.

### 2. Open the web panel

The FCU brings up a WiFi Access Point. Connect your phone/laptop and open `http://192.168.4.1` (the captive DNS redirects any hostname automatically — most phones open the page for you).

**How to turn the AP on:**

| Variant | Available gestures |
|---|---|
| **STR** | Hold trigger for 5 s during first 5 s after boot; OR 5 trigger pulls while in SAFE within 3 s |
| **PRO** | Dedicated WiFi button; OR the same gestures above |

The AP auto-shuts down after **10 minutes of web inactivity** (3 beeps).

> **Note:** firmware enters **deep sleep after 60 min** without trigger activity. A trigger pull wakes the MCU (full reboot); the second pull actually fires.

### 3. Change the WiFi password

Web panel → **WiFi** section → type new password (min 8 chars) → **Save**. Password persists in NVS; only resets on factory reset.

### 4. Slots

3 independent slots. Slot switcher lives at the top of the page. The active slot persists in NVS — on next boot the FCU returns to the last one used. Each slot stores **all** parameters below independently.

### 5. Firing type (S8PA / D8PA)

- **S8PA (1 solenoid)** — F2 / Pulsar-style gearboxes. Cycles only the poppet: DP pulse → DR wait → repeat.
- **D8PA (2 solenoids)** — Jack / Backdraft-style gearboxes. Cycles nozzle and poppet: **DN** (SOL2) pulse → **DR** wait → **DP** (SOL1) pulse → **DL** wait → repeat.

Choosing S8PA automatically hides DN, DL, MOS swap, and the SOL 2 test button.

### 6. Timings

| Param | Meaning | Used by |
|---|---|---|
| **DN** (Nozzle Dwell) | Nozzle (SOL2) pulse length | D8PA only |
| **DR** (Return) | D8PA: wait between DN and DP (bucking seals). S8PA: inter-shot rest | both |
| **DP** (Poppet) | Poppet (SOL1) pulse length — **this is the shot** | both |
| **DL** (Post-shot Delay) | Wait after poppet (BB leaves the barrel) | D8PA only |

Range: **2–80 ms** per field. Theoretical ROF updates live on the panel.

### 7. Selector

- **2 positions** (default): only Pos 1 / Pos 2. Each maps to one of: SAFE, SEMI, FULL, BURST2, BURST3, BURST4.
- **3 positions**: enabling this forces Hall mode on the selector. Reveals Pos 3.

Each position is independently assignable. E.g. Pos 1 = SAFE, Pos 2 = SEMI, Pos 3 = FULL.

### 8. Input — trigger and selector

- **Type**: Digital switch (microswitch) or analog Hall. Can be mixed (e.g. digital trigger + Hall selector).
- **Invert trigger**: check if your microswitch is active-HIGH.
- **MOS swap** (D8PA only): swaps SOL1↔SOL2 electrically — useful if you wired them backwards.
- **Silent mode**: mutes beeps while firing (mode-change beep stays on).

### 9. Hall trigger sensitivity

**Single-point capture**: pull until the fire point you want, click "Capture". The FCU reads the ADC and builds a symmetric hysteresis band automatically.

### 10. Full calibration (Hall)

For maximum accuracy:

1. **Noise calibration**: FCU fires a full cycle (no BB) while sampling ADC, measuring the EMI from the solenoids. Widens the Hall deadband automatically to prevent ghost fires.
2. **Released / Pressed**: capture two trigger points; FCU computes the threshold.
3. **3-position selector**: capture one sample per physical position.

### 11. Diagnostics

- **Test MOS 1 / MOS 2**: pulses the MOSFET for 2 s. Use with a multimeter, LED, or solenoid to test wiring. *Refuses while firing is active.*
- **Buzzer test**: plays each beep type.

### 12. ROF limit and Semi ROF

- **ROF limit** (rounds/sec): overall ceiling. 0 = no limit (runs as fast as DN/DR/DP/DL allow).
- **Semi ROF** (ms): cooldown *between semi pulls*. Prevents trigger-spam. 0 = disabled.

### 13. Factory reset

Flash a new build that bumps `STORAGE_INIT_VERSION` (NVS gets wiped automatically), OR open the AP and use **Advanced → Factory reset** if exposed in your build.

### 14. Deep-sleep debug mode

For bench testing, edit `firmware/BCGA_FCU_{STR,PRO}/config.h` and uncomment:

```c
#define DEEP_SLEEP_DEBUG
```

This shortens the idle timeout from 60 min to 5 min. **Re-comment before shipping to production.**

### Variant differences

| Feature | STR | PRO |
|---|:---:|:---:|
| S8PA (1 solenoid) | ✅ | ✅ |
| D8PA (2 solenoids) | ✅ | ✅ |
| Battery voltage read | ❌ | ✅ |
| Kill latch (deep-discharge protection) | ❌ | ✅ |
| Dedicated WiFi button | ❌ | ✅ |
| Onboard buzzer | ❌ | ✅ |
| Trigger gestures to open AP | ✅ | ✅ |
| 60-min idle deep sleep | ✅ | ✅ |

---

## License

GPL v3 — see [LICENSE](../LICENSE).
