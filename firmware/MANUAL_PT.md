# BCGA FCU — Manual de Uso (PT-BR)

Firmware open-source para ESP32-C3 SuperMini. Duas variantes: **STR** (Starter, perfboard) e **PRO** (produto comercial). A interface web e a lógica de disparo são quase idênticas — as diferenças estão no fim do documento.

Esse manual é prático: cada seção começa com **o que mexer para obter o quê**.

> 🇬🇧 English version: [MANUAL_EN.md](./MANUAL_EN.md)

---

## Índice

1. [Entendendo a FCU em 1 minuto](#1-entendendo-a-fcu-em-1-minuto)
2. [Os 4 timings — guia de tuning](#2-os-4-timings--guia-de-tuning)
3. [Início rápido — chegar no FPS alvo primeiro](#3-inicio-rapido--chegar-no-fps-alvo-primeiro)
4. [Fluxo recomendado de tuning](#4-fluxo-recomendado-de-tuning)
5. [Tipo de disparo: S8PA vs D8PA](#5-tipo-de-disparo-s8pa-vs-d8pa)
6. [Seletor — 2 ou 3 posições](#6-seletor--2-ou-3-posicoes)
7. [Gatilho Hall — calibração passo a passo](#7-gatilho-hall--calibracao-passo-a-passo)
8. [Seletor Hall — calibração passo a passo](#8-seletor-hall--calibracao-passo-a-passo)
9. [Primeiro uso e acesso ao painel Web](#9-primeiro-uso-e-acesso-ao-painel-web)
10. [Slots](#10-slots)
11. [Flags úteis (inverter gatilho, swap MOS, silent)](#11-flags-uteis)
12. [Limitar cadência (ROF limit e Semi ROF)](#12-limitar-cadencia)
13. [Diagnóstico e WiFi](#13-diagnostico-e-wifi)
14. [Deep sleep e modo debug](#14-deep-sleep-e-modo-debug)
15. [Diferenças STR vs PRO](#15-diferencas-str-vs-pro)
16. [BCGA FCU vs FCUs comerciais](#16-bcga-fcu-vs-fcus-comerciais)

---

## 1. Entendendo a FCU em 1 minuto

A FCU controla **1 ou 2 solenoides** que substituem o gatilho mecânico de uma gearbox HPA. Em cada tiro, ela envia pulsos eletricamente calibrados para os solenoides, na ordem correta, com as esperas corretas entre eles. Tuning = ajustar esses pulsos para o teu sistema específico (pressão, bucking, BB, peso).

- **S8PA** (1 solenoide): PolarStar JACK / F1, Wolverine INFERNO, GATE PULSAR S, ou qualquer outra gearbox de 1 solenoide. Só o poppet é controlado.
- **D8PA** (2 solenoides): PolarStar F2 / Fusion Engine, GATE PULSAR D, ou qualquer outra gearbox de 2 solenoides. Nozzle + poppet controlados separadamente.

---

## 2. Os 4 timings — guia de tuning

A FCU expõe **4 timings independentes** que mapeiam diretamente para cada fase física do ciclo de disparo: **DN**, **DR**, **DP**, **DB**. A página mostra o ROF teórico em tempo real enquanto você mexe nos sliders.

> **Nota sobre unidades:** DN, DR e DP estão em **milissegundos** (faixa 2–80 ms). O **DB** (Trigger Debounce) usa **units** — 1 unit = 0,1 ms, faixa 20–800 units (= 2–80 ms). Isso alinha o DB à resolução interna de 0,1 ms do firmware.

> **Nota sobre o rename:** o parâmetro agora chamado **DB (Trigger Debounce)** era anteriormente **DL (Post-shot Delay)**. O comportamento físico é idêntico — só mudou o nome e a unidade exibida.

### DN — Nozzle Dwell (só D8PA)

Duração do pulso do **nozzle (SOL2)**. É quanto tempo o nozzle fica aberto para a BB cair na câmara.

| Se DN aumenta | Se DN diminui |
|---|---|
| ✅ alimentação mais fácil (BBs pesadas, pressão baixa, cold shot) | ✅ economiza ar, previne dupla alimentação |
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

Duração do pulso do **poppet (SOL1)**. É quanto tempo o ar pode fluir pelo nozzle empurrando a BB.

| Se DP aumenta | Se DP diminui |
|---|---|
| ✅ **FPS mais alto** (mais ar atrás da BB) | ✅ economiza ar (mais tiros por garrafa/mag) |
| ❌ desperdício de ar após a BB sair | ✅ ROF mais alto (valve fecha rápido) |
| ❌ pode atrasar recycle do poppet | ❌ **FPS abaixo do alvo**, tiro sem potência |

**Regra prática:** com chrono, ajuste DP até chegar ao FPS alvo. Mais que isso é só desperdício.

### DB — Trigger Debounce (só D8PA)

> Antigamente **DL — Post-shot Delay**. Mesmo comportamento, novo nome e nova unidade.

Espera após o poppet fechar, antes do gatilho poder armar o próximo ciclo. Fisicamente é o tempo para a **BB sair do cano** e o bucking se recuperar.

**Unidade:** 1 unit = 0,1 ms. **Faixa:** 20–800 units (2–80 ms). **Default:** 100 units (10 ms).

| Units | Equivalente em ms |
|---:|---:|
| 20  | 2,0  |
| 50  | 5,0  |
| 100 | 10,0 |
| 200 | 20,0 |
| 800 | 80,0 |

| Se DB aumenta | Se DB diminui |
|---|---|
| ✅ **melhor precisão** (BB já saiu quando o próximo nozzle abre) | ✅ **ROF mais alto** |
| ✅ bucking recupera forma sem perturbação | ❌ BB ainda dentro do cano quando o próximo nozzle pulsa → **flyers / FPS inconsistente** |
| ❌ ROF mais baixo | ❌ vida útil reduzida do bucking |

**Regra prática:** atire em full-auto num alvo a 20 m. Se aparecerem flyers, aumente DB 20 units (2 ms) por vez.

### Defaults (D8PA genérico, 110 psi, BB 0.28)

```
DN = 18 ms    DR = 26 ms    DP = 25 ms    DB = 100 units (10 ms)
```

Ponto de partida seguro. Ajuste a partir daí.

---

## 3. Início rápido — chegar no FPS alvo primeiro

Primeira vez na BCGA FCU? Antes de tudo, acerte o FPS alvo. O resto do tuning só faz sentido com o chrono onde você quer.

1. **Coloque o regulador em 100 psi.** Pressão inicial padrão para a maioria dos setups D8PA.
2. **Mantenha os timings default** (`DN=18 / DR=26 / DP=25 / DB=100`). Já vêm assim de fábrica.
3. **Atire no chrono.** O FPS (ou joule) está no alvo?
   - **Sim** → pule direto pra seção 4 e ajuste alimentação, vedação e precisão.
   - **Não, abaixo do alvo** → passo 4.
4. **Suba o DP até o máximo do slider** (80 ms). Chrono de novo.
   - **FPS subiu pro alvo** → abaixe o DP passo a passo até o FPS começar a cair, depois volte 1–2 ms. Esse é o seu DP mínimo eficiente. Vá pra seção 4.
   - **FPS ainda baixo** → passo 5.
5. **Aumente o regulador pra 110 psi.** Chrono de novo.
   - **No alvo** → abaixe o DP até achar o mínimo que segura o FPS alvo. Vá pra seção 4.
   - **Ainda baixo** → suba pra **120 psi** e chrono de novo.
6. Com FPS alvo estável, **vá pra seção 4** e ajuste DN (alimentação), DR (vedação) e DB (precisão) nessa ordem.

> **Princípio:** FPS é dado principalmente por **pressão × DP**. Os outros 3 timings moldam *como* o ciclo se comporta (alimentação, vedação, precisão, ROF) — não somam FPS. Acerte o FPS primeiro com DP/pressão, ajuste o resto depois.

---

## 4. Fluxo recomendado de tuning

Siga nesta ordem — cada passo depende do anterior estar estável. Assume que você já atingiu o FPS alvo pela seção 3.

1. **Alimentação (DN)** — dispare SEMI lento. Diminua DN até pegar tiro vazio. Volte 2 ms.
2. **Vedação (DR)** — dispare SEMI rápido. Se o chrono oscilar, aumente DR 2 ms.
3. **FPS (DP)** — com chrono, ajuste DP até atingir o alvo. Não suba mais que o necessário.
4. **Precisão (DB, só D8PA)** — full-auto em alvo. Se flyers, aumente DB em 20 units.
5. **Cadência (ROF limit)** — opcional. Limita o ROF máximo independentemente dos timings.
6. **Anti-spam (Semi ROF)** — opcional. Define tempo mínimo entre puxadas em SEMI.

> ⚠️ Cheque sempre com chrono. FPS drifta com temperatura da garrafa / nível de ar.

---

## 5. Tipo de disparo: S8PA vs D8PA

Escolhido por slot, na primeira seção do painel.

- **S8PA** — só o poppet é pulsado. Ciclo: `DP → DR → repete`. Use com PolarStar JACK / F1, Wolverine INFERNO, GATE PULSAR S, ou qualquer gearbox de 1 solenoide. Os campos DN, DB, swap MOS e o botão de teste do SOL 2 somem da interface.
- **D8PA** — nozzle + poppet separados. Ciclo: `DN → DR → DP → DB → repete`. Use com PolarStar F2 / Fusion Engine, GATE PULSAR D, ou qualquer sistema com 2 solenoides.

---

## 6. Seletor — 2 ou 3 posições

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

## 7. Gatilho Hall — calibração passo a passo

Use Hall se quiser um gatilho **sem desgaste mecânico** e com **ponto de disparo ajustável**.

**Hardware esperado:** sensor Hall linear (DRV5055 ou similar) alimentado em 3.3 V, saída ligada em **PIN_TRIG** (GPIO 0 em ambas variantes). Ímã na alavanca do gatilho.

### 7.1 Seleção do modo

Seção **Entrada** → "Tipo gatilho" → **Hall** → **Salvar**.

### 7.2 Calibração rápida (single-point)

Para a maioria dos casos — 30 segundos.

1. Na seção **Sensibilidade**, puxe o gatilho **até o ponto exato onde quer que dispare** e segure.
2. Clique **Capturar ponto**.
3. A FCU lê o ADC e cria automaticamente uma banda de histerese simétrica em torno desse valor.
4. Salve o slot.

Se disparar antes de chegar no ponto → capture num ponto um pouco mais puxado.
Se não disparar mesmo puxando até o fim → capture num ponto menos puxado.

### 7.3 Calibração completa (para máxima precisão)

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

## 8. Seletor Hall — calibração passo a passo

Hall no seletor permite **3 posições físicas** (SAFE/SEMI/FULL clássico) ou simplesmente uma leitura mais confiável de 2 posições.

**Hardware esperado:** sensor Hall linear no corpo da gearbox, com ímã na placa de seleção. Saída em **PIN_SEL** (GPIO 1).

### 8.1 Seleção do modo

Seção **Entrada** → "Tipo seletor" → **Hall** → marque "3 posições" se aplicável → **Salvar**.

### 8.2 Calibração

Seção **Calibração do seletor**:

1. **Calibração de ruído** (opcional mas recomendada) — mesmo princípio do gatilho.
2. Rode o seletor para **Pos 1** → **Capturar Pos 1**.
3. Rode para **Pos 2** → **Capturar Pos 2**.
4. (se 3-pos) Rode para **Pos 3** → **Capturar Pos 3**.
5. A FCU calcula os thresholds entre cada par de amostras.
6. **Salvar slot**.

### 8.3 Atribuir modos

Em **Seletor → Pos 1/2/3 Modo**, escolha um modo de disparo para cada posição. Estes campos funcionam independente do tipo (digital ou Hall).

---

## 9. Primeiro uso e acesso ao painel Web

### First-boot

Ao ligar pela primeira vez (ou após flashar uma build nova que mudou schema), a NVS é reinicializada com os defaults. Dois beeps longos (BOOT + READY) confirmam o first-boot; depois só toca BOOT em cada power-up normal.

**Defaults iniciais:**

- 3 slots nomeados `Slot 1`, `Slot 2`, `Slot 3`
- D8PA, 2-pos, Pos1=SEMI, Pos2=FULL
- Gatilho digital (microswitch, active-LOW)
- Timings `DN=18 ms / DR=26 ms / DP=25 ms / DB=100 units (10 ms)`
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

## 10. Slots

3 slots independentes. Cada um guarda **tudo**: tipo (S8PA/D8PA), timings, seletor, calibrações Hall, flags. Troque pelo botão no topo da página.

A FCU lembra qual era o último slot usado e volta para ele após reboot.

**Uso típico:**
- Slot 1 = seu setup principal
- Slot 2 = ajuste agressivo (mais ROF, menos DR)
- Slot 3 = backup / teste

---

## 11. Flags úteis

- **Inverter gatilho** — marque se o seu microswitch é active-HIGH (raro).
- **Swap MOS** (só D8PA) — inverte SOL1↔SOL2 **por software**. Use se você soldou errado e não quer dessoldar.
- **Silent mode** — silencia apenas os beeps *durante* o tiro. Beep de troca de modo continua.

---

## 12. Limitar cadência

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

## 13. Diagnóstico e WiFi

### Teste MOS

- **Test MOS 1** — pulsa o MOSFET 1 por 2 s.
- **Test MOS 2** — pulsa o MOSFET 2 por 2 s (respeita `Swap MOS`).

Use com multímetro, LED, ou solenoide para verificar fiação.

> Recusa acontecer se a FCU estiver disparando.

### Buzzer test

Toca cada tipo de beep. Útil para identificar sons antes de sair pra jogar. No STR o piezo não vem populado de fábrica; o código continua rodando mas fica mudo a menos que você adicione um piezo no PIN_BUZZER.

### Trocar senha WiFi

Seção **WiFi** → nova senha (mínimo **8 caracteres**) → **Salvar**. Persiste em NVS. Só volta ao default num factory reset.

### Factory reset

Duas formas:
1. Flashar uma build que incrementa `STORAGE_INIT_VERSION` (NVS é limpa no próximo boot automaticamente).
2. Se a build expor **Avançado → Reset de fábrica** no painel.

---

## 14. Deep sleep e modo debug

Após **60 min sem atividade no gatilho**, a FCU entra em deep-sleep (consumo <10 µA). A próxima puxada **acorda o MCU via reboot completo** — o segundo puxar já dispara.

### Modo debug (5 min)

Para bancada, edite `firmware/BCGA_FCU_{STR,PRO}/config.h` e descomente:

```c
#define DEEP_SLEEP_DEBUG
```

Timeout cai de 60 min para 5 min. **Re-comente antes de entregar para produção.**

---

## 15. Diferenças STR vs PRO

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

## 16. BCGA FCU vs FCUs comerciais

Esta seção compara a BCGA FCU com as principais FCUs comerciais do mercado airsoft HPA (PolarStar REV3, Wolverine BLINC, GATE TITAN II, Gorilla FCU). É uma avaliação factual — inclui vantagens e lacunas honestas.

### 16.1 Onde a BCGA FCU ganha

1. **WiFi nativo vs Bluetooth.** Configure pelo browser de qualquer dispositivo — iOS, Android, PC, Linux, qualquer coisa que abra uma página web. Sem instalar app, sem pareamento, sem vendor lock-in. TITAN II (BLE 5.2), BLINC e Gorilla exigem apps proprietários específicos.

2. **4 timings independentes (DN/DR/DP/DB).** Cada fase do ciclo D8PA tem seu próprio parâmetro. Alimentação (DN), vedação (DR), FPS (DP) e debounce pós-tiro (DB) são ajustados separadamente sem trade-offs. FCUs comerciais chegam no máximo a 3 timings expostos ao usuário (REV3 em modo dual-solenoid: dP/dN/dr) ou dwell único em single-solenoid. Nenhuma FCU comercial expõe parâmetro dedicado para debounce pós-tiro.

3. **Calibração automática de ruído EMI no gatilho Hall.** Única FCU do mercado com rotina que dispara os solenoides a seco e mede o chute EMI no ADC, alargando automaticamente o deadband do Hall. Elimina ghost fires sem sacrificar sensibilidade.

4. **3 slots completos e independentes.** Cada slot armazena **tudo** — tipo de engine, os 4 timings, configuração de selector, calibrações Hall individuais, flags. Trocar slot = trocar perfil de jogo completo.

5. **Open-source, GPL v3.** Código totalmente aberto. Auditar, modificar, compilar e flashar sem depender de firmware proprietário ou app do fabricante. TITAN II, BLINC, REV3 e Gorilla são fechados.

6. **Gatilho Hall com calibração de 2 pontos + histerese automática.** Captura o ponto exato de disparo e calcula banda de histerese a partir de medições reais — não valores fixos. Sem potenciômetro mecânico.

7. **Seletor Hall de 3 posições.** SAFE/SEMI/FULL via sensor Hall sem desgaste mecânico. Cada posição livre para qualquer modo (inclusive BURST 2/3/4).

8. **BOM drasticamente mais barato.** O STR pode ser construído com componentes THT fáceis de achar por uma fração do custo de qualquer FCU Bluetooth comercial.

9. **ROF teórico em tempo real na UI.** O painel web mostra o ROF máximo alcançável enquanto você mexe nos sliders — sem cronógrafo para uma estimativa inicial.

10. **Troca S8PA/D8PA por slot via software.** Cada um dos 3 slots armazena independentemente o tipo de engine — alterne o slot entre S8PA e D8PA direto pelo painel web, sem tocar em hardware. FCUs comerciais que cobrem as duas arquiteturas (TITAN II com PULSAR S/D, Gorilla) fazem isso via cabos dedicados distintos por engine, não via chaveamento por slot no software.

### 16.2 Limitações honestas

Quem está comprando precisa saber disto antes de escolher a BCGA FCU:

1. **Sem binary trigger.** Não implementado. Presente no Wolverine BLINC, GATE TITAN II e Gorilla FCU.
2. **Sem tournament lock com senha.** Contorno: configurar Semi ROF alto + ROF limit baixo antes do evento. Presente no TITAN II (Expert) e Gorilla.
3. **Kill latch e buzzer integrado apenas no PRO.** A variante STR não tem leitura de bateria nem corte de LiPo. Use com cuidado em packs 2S/3S sem proteção externa.
4. **Primeira puxada após deep sleep acorda via reboot.** Após 60 min de inatividade, a FCU entra em deep-sleep. A próxima puxada acorda o MCU por reboot completo — a **segunda** puxada é a que dispara. Diferente de FCUs que dormem por gate-hold do MOSFET.

### 16.3 Comparativo lado-a-lado

| Dimensão | BCGA FCU STR/PRO | PolarStar REV3 | Wolverine BLINC | GATE TITAN II | Gorilla FCU |
|---|---|---|---|---|---|
| MCU | ESP32-C3 | Proprietário | Proprietário | ARM + BLE 5.2 | Proprietário + BLE |
| Licença | **GPL v3 (open-source)** | Proprietária | Proprietária | Proprietária | Proprietária |
| Single-solenoid | ✅ (S8PA) | ✅ (FCF1) | ✅ | ✅ (PULSAR S) | ✅ |
| Dual-solenoid | ✅ (D8PA) | ✅ (FCFE) | ❌ (só single) | ✅ (PULSAR D) | ✅ |
| Timings independentes | **4 (DN/DR/DP/DB)** | 3 (dual) / 1 (single) | 1 + autotune | Auto cycle-sync ou manual | Dwells SEMI/AUTO separados |
| Interface | **Web UI via WiFi** | LCD + joystick | App BLE | App BLE 5.2 | App BLE |
| App necessário | **Não** | Não | ✅ obrigatório | ✅ obrigatório | ✅ obrigatório |
| Hall noise calibration | **✅ única no mercado** | ❌ | ❌ | ❌ | ❌ |
| Slots de configuração | **3 completos** | 1 set | 1 perfil | Perfis por engine | 1 set |
| Binary trigger | ❌ | Hack (burst=01) | ✅ | ✅ | ✅ |
| Tournament lock | ❌ | — | ❌ | ✅ (Expert) | ✅ |
| Custo aproximado | **~R$50–100 BOM** | ~US$80 FCU | ~US$160 | ~US$300–440 combo | ~US$200 |
| Deep sleep | ✅ 60 min | — | ✅ | ✅ | — |

### 16.4 Para quem é a BCGA FCU

- **Builders DIY HPA** que querem controle total sobre o ciclo de disparo com 4 timings independentes.
- **Instaladores de campo** sem um app Bluetooth específico no celular — qualquer browser serve.
- **Makers com orçamento apertado** construindo um engine S8PA/D8PA do zero.
- **Defensores de open-source** que não aceitam rifle com firmware fechado.
- **Quem quer 3 perfis distintos de jogo** numa única FCU (skirmish, DMR, CQB).

A BCGA FCU **não** é a escolha certa se você precisa de binary trigger ou tournament lock com senha de fábrica — para isso pegue TITAN II, BLINC ou Gorilla.

---

## Licença

GPL v3 — ver [LICENSE](../LICENSE).
