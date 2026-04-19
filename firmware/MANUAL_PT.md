# BCGA FCU — Manual de Uso (PT-BR)

Firmware open-source para ESP32-C3 SuperMini. Duas variantes: **STR** (Starter, perfboard) e **PRO** (produto comercial). A interface web e a lógica de disparo são quase idênticas — as diferenças estão no fim do documento.

Esse manual é prático: cada seção começa com **o que mexer para obter o quê**.

> 🇬🇧 English version: [MANUAL_EN.md](./MANUAL_EN.md)

---

## Índice

1. [Entendendo a FCU em 1 minuto](#1-entendendo-a-fcu-em-1-minuto)
2. [Os 4 timings — guia de tuning](#2-os-4-timings--guia-de-tuning)
3. [Fluxo recomendado de tuning](#3-fluxo-recomendado-de-tuning)
4. [Tipo de disparo: S8PA vs D8PA](#4-tipo-de-disparo-s8pa-vs-d8pa)
5. [Seletor — 2 ou 3 posições](#5-seletor--2-ou-3-posicoes)
6. [Gatilho Hall — calibração passo a passo](#6-gatilho-hall--calibracao-passo-a-passo)
7. [Seletor Hall — calibração passo a passo](#7-seletor-hall--calibracao-passo-a-passo)
8. [Primeiro uso e acesso ao painel Web](#8-primeiro-uso-e-acesso-ao-painel-web)
9. [Slots](#9-slots)
10. [Flags úteis (inverter gatilho, swap MOS, silent)](#10-flags-uteis)
11. [Limitar cadência (ROF limit e Semi ROF)](#11-limitar-cadencia)
12. [Diagnóstico e WiFi](#12-diagnostico-e-wifi)
13. [Deep sleep e modo debug](#13-deep-sleep-e-modo-debug)
14. [Diferenças STR vs PRO](#14-diferencas-str-vs-pro)

---

## 1. Entendendo a FCU em 1 minuto

A FCU controla **1 ou 2 solenoides** que substituem o gatilho mecânico de uma gearbox HPA. Em cada tiro, ela envia pulsos eletricamente calibrados para os solenoides, na ordem correta, com as esperas corretas entre eles. Tuning = ajustar esses pulsos para o teu sistema específico (pressão, bucking, BB, peso).

- **S8PA** (1 solenoide): gearboxes tipo F2/Pulsar. Só o poppet é controlado.
- **D8PA** (2 solenoides): gearboxes tipo Jack/Backdraft. Nozzle + poppet controlados separadamente.

---

## 2. Os 4 timings — guia de tuning

Todos em **milissegundos**, faixa permitida **2–80 ms** por campo. A página mostra o ROF teórico em tempo real.

### DN — Nozzle Dwell (só D8PA)

Duração do pulso do **nozzle (SOL2)**. É quanto tempo o nozzle fica aberto para a BB cair na câmara.

| Se DN aumenta | Se DN diminui |
|---|---|
| ✅ alimentação mais fácil (BBs pesadas, pressão baixa, cold shot) | ✅ economiza gás, previne dupla alimentação |
| ❌ **risco de dupla alimentação** (2 BBs caem) | ❌ tiro vazio / alimentação inconsistente se o spring de retorno for lento |
| ❌ mais desgaste no nozzle | ❌ pode não selar direito antes do DP |

**Regra prática:** diminua DN até começar a dar tiro vazio, depois adicione 1–2 ms de margem.

### DR — Return / Rest

- **Em D8PA**: espera entre o pulso do nozzle e o pulso do poppet. É o tempo para o nozzle voltar e o **bucking selar na BB**.
- **Em S8PA**: descanso entre tiros (inter-shot rest).

| Se DR aumenta | Se DR diminui |
|---|---|
| ✅ vedação consistente → **FPS estável** | ✅ **ROF mais alto** |
| ✅ menos dispersão tiro-a-tiro | ❌ poppet dispara antes de selar → tiro fraco / FPS baixo e variável |
| ❌ ROF mais baixo | ❌ BBs caindo na frente do nozzle aberto = dupla / pick-up ruim |

**Regra prática:** suba DR até o chrono parar de oscilar em SEMI rápido.

### DP — Poppet Dwell (o tiro)

Duração do pulso do **poppet (SOL1)**. É quanto tempo o gás pode fluir pelo nozzle empurrando a BB.

| Se DP aumenta | Se DP diminui |
|---|---|
| ✅ **FPS mais alto** (mais gás atrás da BB) | ✅ economiza gás (mais tiros por garrafa/mag) |
| ❌ desperdício de gás após a BB sair | ✅ ROF mais alto (valve fecha rápido) |
| ❌ pode atrasar recycle do poppet | ❌ **FPS abaixo do alvo**, tiro sem potência |

**Regra prática:** com chrono, ajuste DP até chegar ao FPS alvo. Mais que isso é só desperdício.

### DL — Post-shot Delay (só D8PA)

Espera após o poppet fechar. É o tempo para a **BB sair do cano** antes do próximo ciclo.

| Se DL aumenta | Se DL diminui |
|---|---|
| ✅ **melhor precisão** (BB já saiu quando o próximo nozzle abre) | ✅ **ROF mais alto** |
| ✅ bucking recupera forma sem perturbação | ❌ BB ainda dentro do cano quando o próximo nozzle pulsa → **flyers / FPS inconsistente** |
| ❌ ROF mais baixo | ❌ vida útil reduzida do bucking |

**Regra prática:** atire em full-auto num alvo a 20 m. Se aparecerem flyers, aumente DL 2 ms por vez.

### Defaults (D8PA genérico, Jack-style, 110 psi, BB 0.28)

```
DN = 18 ms    DR = 26 ms    DP = 25 ms    DL = 10 ms
```

Ponto de partida seguro. Ajuste a partir daí.

---

## 3. Fluxo recomendado de tuning

Siga nesta ordem — cada passo depende do anterior estar estável.

1. **Alimentação (DN)** — dispare SEMI lento. Diminua DN até pegar tiro vazio. Volte 2 ms.
2. **Vedação (DR)** — dispare SEMI rápido. Se o chrono oscilar, aumente DR 2 ms.
3. **FPS (DP)** — com chrono, ajuste DP até atingir o alvo. Não suba mais que o necessário.
4. **Precisão (DL, só D8PA)** — full-auto em alvo. Se flyers, aumente DL.
5. **Cadência (ROF limit)** — opcional. Limita o ROF máximo independentemente dos timings.
6. **Anti-spam (Semi ROF)** — opcional. Define tempo mínimo entre puxadas em SEMI.

> ⚠️ Cheque sempre com chrono. FPS drifta com temperatura da garrafa / nível de gás.

---

## 4. Tipo de disparo: S8PA vs D8PA

Escolhido por slot, na primeira seção do painel.

- **S8PA** — só o poppet é pulsado. Ciclo: `DP → DR → repete`. Use com F2, Pulsar, ou qualquer gearbox de 1 solenoide. Os campos DN, DL, swap MOS e o botão de teste do SOL 2 somem da interface.
- **D8PA** — nozzle + poppet separados. Ciclo: `DN → DR → DP → DL → repete`. Use com Jack, Backdraft, e qualquer sistema com 2 solenoides.

---

## 5. Seletor — 2 ou 3 posições

Ativar "3 posições" **exige seletor Hall** (o microswitch só diferencia 2 estados). Ao ligar, a interface revela Pos 3.

Cada posição é atribuída a um modo **independente**:

| Modo | Comportamento |
|---|---|
| **SAFE** | Puxar o gatilho não faz nada. Beeps de modo continuam. |
| **SEMI** | Um tiro por puxada. Respeita Semi ROF se configurado. |
| **FULL** | Dispara enquanto o gatilho estiver pressionado. |
| **BURST 2 / 3 / 4** | Dispara 2/3/4 tiros por puxada, mesmo soltando o gatilho. |

Exemplos comuns:
- **2 posições**: Pos 1 = SAFE, Pos 2 = SEMI *(ou Pos 1 = SEMI, Pos 2 = FULL)*
- **3 posições**: Pos 1 = SAFE, Pos 2 = SEMI, Pos 3 = FULL

---

## 6. Gatilho Hall — calibração passo a passo

Use Hall se quiser um gatilho **sem desgaste mecânico** e com **ponto de disparo ajustável**.

**Hardware esperado:** sensor Hall linear (DRV5055 ou similar) alimentado em 3.3 V, saída ligada em **PIN_TRIG** (GPIO 0 em ambas variantes). Ímã na alavanca do gatilho.

### 6.1 Seleção do modo

Seção **Entrada** → "Tipo gatilho" → **Hall** → **Salvar**.

### 6.2 Calibração rápida (single-point)

Para a maioria dos casos — 30 segundos.

1. Na seção **Sensibilidade**, puxe o gatilho **até o ponto exato onde quer que dispare** e segure.
2. Clique **Capturar ponto**.
3. A FCU lê o ADC e cria automaticamente uma banda de histerese simétrica em torno desse valor.
4. Salve o slot.

Se disparar antes de chegar no ponto → capture num ponto um pouco mais puxado.
Se não disparar mesmo puxando até o fim → capture num ponto menos puxado.

### 6.3 Calibração completa (para máxima precisão)

Na seção **Calibração do gatilho**:

1. **Calibração de ruído**
   - A FCU dispara **um ciclo completo sem BB** medindo o ADC durante o pulso dos solenoides.
   - Calcula o ruído EMI e **alarga automaticamente a banda morta** do gatilho.
   - Previne disparo-fantasma causado pelo próprio chute dos MOSFETs.
   - ⚠️ **Rode com a gearbox montada e garrafa conectada** — o ruído depende do setup físico.

2. **Captura Solto**
   - Gatilho totalmente solto → **Capturar Solto**.

3. **Captura Pressionado**
   - Gatilho totalmente puxado → **Capturar Pressionado**.

4. A FCU calcula o threshold central e uma banda de histerese baseada nos dois pontos + ruído medido.

5. **Salvar slot**.

### Tuning do gatilho Hall

| Sintoma | Causa provável | Ajuste |
|---|---|---|
| Dispara sozinho (fantasma) | Ruído EMI do solenoide maior que a banda morta | Re-rode a calibração de ruído |
| Dispara antes do ponto | Threshold baixo demais | Re-capture num ponto mais próximo do final |
| Não dispara até o fim | Threshold alto demais ou sensor mal-posicionado | Re-capture, ou aproxime mais o ímã |
| Intermitente no limiar | Banda de histerese estreita | Use calibração completa em vez de single-point |

---

## 7. Seletor Hall — calibração passo a passo

Hall no seletor permite **3 posições físicas** (SAFE/SEMI/FULL clássico) ou simplesmente uma leitura mais confiável de 2 posições.

**Hardware esperado:** sensor Hall linear no corpo da gearbox, com ímã na placa de seleção. Saída em **PIN_SEL** (GPIO 1).

### 7.1 Seleção do modo

Seção **Entrada** → "Tipo seletor" → **Hall** → marque "3 posições" se aplicável → **Salvar**.

### 7.2 Calibração

Seção **Calibração do seletor**:

1. **Calibração de ruído** (opcional mas recomendada) — mesmo princípio do gatilho.
2. Rode o seletor para **Pos 1** → **Capturar Pos 1**.
3. Rode para **Pos 2** → **Capturar Pos 2**.
4. (se 3-pos) Rode para **Pos 3** → **Capturar Pos 3**.
5. A FCU calcula os thresholds entre cada par de amostras.
6. **Salvar slot**.

### 7.3 Atribuir modos

Em **Seletor → Pos 1/2/3 Modo**, escolha um modo de disparo para cada posição. Estes campos funcionam independente do tipo (digital ou Hall).

---

## 8. Primeiro uso e acesso ao painel Web

### First-boot

Ao ligar pela primeira vez (ou após flashar uma build nova que mudou schema), a NVS é reinicializada com os defaults. Dois beeps longos (BOOT + READY) confirmam o first-boot; depois só toca BOOT em cada power-up normal.

**Defaults iniciais:**

- 3 slots nomeados `Slot 1`, `Slot 2`, `Slot 3`
- D8PA, 2-pos, Pos1=SEMI, Pos2=FULL
- Gatilho digital (microswitch, active-LOW)
- Timings `DN=18 / DR=26 / DP=25 / DL=10`
- SSID = `BCGA_FCU_STR` ou `BCGA_FCU_PRO`
- Senha WiFi = `12345678`

### Abrir o painel

Conecte o celular/notebook ao AP e abra qualquer URL — o DNS captive redireciona para o painel. Endereço fixo: `http://192.168.4.1`.

**Ligar o AP:**

| Variante | Gestos |
|---|---|
| **STR** | Segurar o gatilho por 5 s durante os primeiros 5 s pós-boot; OU 5 puxadas em SAFE dentro de 3 s |
| **PRO** | Botão WiFi dedicado; OU os mesmos gestos acima |

AP desliga sozinho após **10 min sem atividade web** (3 beeps).

---

## 9. Slots

3 slots independentes. Cada um guarda **tudo**: tipo (S8PA/D8PA), timings, seletor, calibrações Hall, flags. Troque pelo botão no topo da página.

A FCU lembra qual era o último slot usado e volta para ele após reboot.

**Uso típico:**
- Slot 1 = seu setup principal
- Slot 2 = ajuste agressivo (mais ROF, menos DR)
- Slot 3 = backup / teste

---

## 10. Flags úteis

- **Inverter gatilho** — marque se o seu microswitch é active-HIGH (raro).
- **Swap MOS** (só D8PA) — inverte SOL1↔SOL2 **por software**. Use se você soldou errado e não quer dessoldar.
- **Silent mode** — silencia apenas os beeps *durante* o tiro. Beep de troca de modo continua.

---

## 11. Limitar cadência

### ROF limit (rounds/sec)

Teto geral. Se seus timings permitem 25 rps mas você quer 15 rps, ponha `15`. Um delay é inserido automaticamente entre tiros.

- `0` = sem limite (ROF determinado só pelos timings)
- Valor baixo demais = a FCU ignora a sua puxada em FULL até o limit permitir o próximo tiro

### Semi ROF (ms)

Cooldown **entre puxadas** em modo SEMI. Previne trigger-spam (dedo rápido).

- `0` = desabilitado
- `150 ms` = no máximo ~6 puxadas/seg em SEMI
- Não afeta FULL/BURST

---

## 12. Diagnóstico e WiFi

### Teste MOS

- **Test MOS 1** — pulsa o MOSFET 1 por 2 s.
- **Test MOS 2** — pulsa o MOSFET 2 por 2 s (respeita `Swap MOS`).

Use com multímetro, LED, ou solenoide para verificar fiação.

> Recusa acontecer se a FCU estiver disparando.

### Buzzer test

Toca cada tipo de beep. Útil para identificar sons antes de sair pra jogar.

### Trocar senha WiFi

Seção **WiFi** → nova senha (mínimo **8 caracteres**) → **Salvar**. Persiste em NVS. Só volta ao default num factory reset.

### Factory reset

Duas formas:
1. Flashar uma build que incrementa `STORAGE_INIT_VERSION` (NVS é limpa no próximo boot automaticamente).
2. Se a build expor **Avançado → Reset de fábrica** no painel.

---

## 13. Deep sleep e modo debug

Após **60 min sem atividade no gatilho**, a FCU entra em deep-sleep (consumo <10 µA). A próxima puxada **acorda o MCU via reboot completo** — o segundo puxar já dispara.

### Modo debug (5 min)

Para bancada, edite `firmware/BCGA_FCU_{STR,PRO}/config.h` e descomente:

```c
#define DEEP_SLEEP_DEBUG
```

Timeout cai de 60 min para 5 min. **Re-comente antes de entregar para produção.**

---

## 14. Diferenças STR vs PRO

| Feature | STR | PRO |
|---|:---:|:---:|
| S8PA (1 solenoide) | ✅ | ✅ |
| D8PA (2 solenoides) | ✅ | ✅ |
| Todos os modos (SAFE/SEMI/FULL/BURST2-4) | ✅ | ✅ |
| Seletor Hall 3-pos | ✅ | ✅ |
| Calibração Hall completa | ✅ | ✅ |
| Painel web + gesto de abertura do AP | ✅ | ✅ |
| Deep sleep 60 min | ✅ | ✅ |
| **Leitura de bateria** | ❌ | ✅ |
| **Kill latch** (protege LiPo de descarga profunda) | ❌ | ✅ |
| **Botão WiFi dedicado** | ❌ | ✅ |
| **Buzzer onboard** | ❌ | ✅ |

---

## Licença

GPL v3 — ver [LICENSE](../LICENSE).
