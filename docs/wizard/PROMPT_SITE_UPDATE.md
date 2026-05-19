# Prompt para Claude Code — atualizar o wizard em bcgaairsoft.com

Cole o conteúdo abaixo numa nova sessão do Claude Code aberta no repositório do site (`~/ClaudeCode/projeto/bcgaairsoft/bcgaairsoft`). Ele descreve o estado atual do firmware da BCGA FCU, as mudanças recentes que precisam refletir no wizard online, e a nova tela inicial de 3 menus.

---

## Contexto

Você está trabalhando no site React da BCGA Airsoft (`bcgaairsoft.com`). O site hospeda um **Setup Wizard** público que ajuda o usuário a planejar a configuração da FCU **antes** de flashar e tunar no airsoft.

A FCU em si tem o repositório separado em `~/ClaudeCode/projeto/bcgaairsoft/BCGA_FCU_STR`. Lá existem dois pontos de referência que valem consultar:

- `docs/wizard/index.html` + `wizard.css` + `wizard.js` — versão standalone original do wizard (5 steps), serve de base visual e estrutural.
- `firmware/web_ui_dev/index_str.html`, `index_pro.html`, `style.css`, `app.js` — UI **runtime** servida pelo ESP no AP (estado atual após várias rodadas de iteração). É a fonte de verdade dos campos/labels/help texts.

O wizard online deve ficar **alinhado com o runtime** (mesmas siglas, mesmos defaults, mesmas faixas), mas mantém o foco em ensinar/planejar — ele não escreve no ESP.

## Tarefa

1. **Encontrar o wizard atual no site** (provavelmente em `src/pages/wizard` ou similar — explore antes de tocar).
2. **Reescrever a tela inicial** para 3 menus (detalhe abaixo).
3. **Reaproveitar o wizard de configuração existente** dentro do menu "Quero configurar do zero".
4. **Adicionar dois novos modos** ("Aprender" e "Resolver problema").
5. **Atualizar todos os campos, defaults, faixas e textos de ajuda** para casar com o estado atual do firmware (lista completa abaixo).

## Tela inicial — 3 menus

Depois do header (logo + nome BCGA FCU · Setup Wizard + toggle PT/EN), o usuário vê 3 cards grandes clicáveis:

1. **Quero apenas aprender** → entra no modo educacional. Tour guiado explicando uma a uma cada função da FCU. Sem perguntas, só conteúdo + navegação Próximo/Anterior. Lista de tópicos sugerida na seção "Modo Aprender" abaixo.

2. **Quero configurar do zero** → entra no wizard atual de 5 steps. Esta é a versão hoje em produção (engine pick → FPS alvo → FPS atingido? → fine-tune → summary). Mantenha o fluxo, mas atualize os campos/labels/textos de ajuda conforme a seção "Mudanças no firmware" abaixo.

3. **Quero resolver um problema** → modo troubleshooting. Usuário descreve o sintoma (input de texto livre OU lista de sintomas comuns clicáveis — escolha o que fizer mais sentido para o site). O wizard responde com diagnóstico + onde mexer. Catálogo de sintomas → soluções na seção "Modo Resolver problema" abaixo.

Adicione um botão de "Voltar ao início" persistente em qualquer ponto dos 3 fluxos.

## Mudanças no firmware desde a última versão do wizard

Atualize TODOS os pontos do wizard atual (modo "Configurar do zero") para refletir o que segue:

### Renomeação de seções

| Antes | Agora |
|---|---|
| "Tipo de disparo" | **Engine** |
| "Entrada" | **Funções** |
| (não existia) | **Avançado** |
| (não existia) | **Estatísticas** |

### Cards do Engine

- "Single Solenoid S8PA" — subtítulo "1 solenoid (Jack, Backdraft)"
- "Dual Solenoid D8PA" — subtítulo "2 solenoids (F2, Pulsar)"

### Defaults validados em campo (use estes nos exemplos do wizard)

- DN = 24 ms (antes 18)
- DR = 24 ms (antes 26)
- DP = 24 ms (antes 80; o "max-tune-down" foi abandonado)
- DB = **5 unidades** (= 0,5 ms; faixa nova: 5–150 unidades = 0,5–15 ms)
- IS = 30 s (antes 60)
- IP = 1 multiplicador (NOVO conceito — explicado abaixo)

### Faixas atuais

| Campo | Faixa | Unidade | Default |
|---|---|---|---|
| DN | 2–80 | ms | 24 |
| DR | 2–80 | ms | 24 |
| DP | 2–80 | ms | 24 |
| DB | 5–150 | unidades de 0,1 ms (= 0,5–15 ms) | 5 |
| ROF max | 0–50 | rps (0 = sem limite) | 0 |
| Semi Max ROF | 0–500 | ms | 0 |
| IS | 0–600 | segundos | 30 |
| IP | 0–5 | multiplicador de DP | 1 |

### Anti-stiction (NOVO — IS + IP)

Conceito que NÃO existia no wizard antigo. Adicione um bloco explicando:

- **Stiction** = o o-ring do poppet "gruda" depois da arma ficar parada por um tempo. Resultado: primeiro tiro depois de tempo parado sai com FPS baixo / sem cadência.
- **IS (Inter-Stiction Idle)** — tempo em segundos de inatividade até o boost ser armado. Padrão 30 s.
- **IP (Inter-Stiction Pulse)** — multiplicador de DP. Cada incremento adiciona um DP inteiro ao próximo tiro pós-idle.
  - DP = 24 ms, IP = 1 → primeiro tiro pós-idle dispara com DP = 48 ms (24+24)
  - DP = 24 ms, IP = 2 → primeiro tiro pós-idle dispara com DP = 72 ms (24+24+24)
  - IP = 0 → desabilitado.
- **Quando ajustar:** se o primeiro tiro depois de ficar 30+ s parado sair fraco, suba o IP em 1 e teste no chrono. Tiros subsequentes voltam ao DP normal automaticamente.

### Tipo de seletor (Selector input)

Antes só dava pra mexer em selPos1/2/3. Agora a UI tem um dropdown explícito **"Selector input: Microswitch / Hall (analog)"**, espelhando o do gatilho. A seção de Calibração do Seletor só aparece quando Hall é escolhido. 3-pos continua forçando Hall (e trava o dropdown).

### "Inverter gatilho" sumiu

Campo `invertTrig` continua no firmware mas **não aparece mais na UI**. Não mencione no wizard.

### Idioma padrão = EN

Page-load default agora é EN (antes era BR). Ainda tem toggle BR/EN. Mantenha o toggle no wizard online.

### Estatísticas (somente runtime — NÃO precisa no wizard online)

A FCU agora exibe stats em tempo real (Tiros sessão, Tiros total, Bateria no PRO, próxima lubrificação a cada 10k tiros). É só pra você saber que existe — não é parte do fluxo de planejamento.

### Config LOCK e Factory Reset (somente runtime)

Mesma situação. A FCU tem agora trava de configuração via senha + reset de fábrica por gesture (gatilho 30 s no boot, no PRO + botão WiFi). Mencione brevemente no modo "Aprender" mas não no fluxo de configuração.

## Modo Aprender — sugestão de capítulos

Cada capítulo é uma página/card com explicação curta (~150 palavras), 1 imagem ou diagrama opcional, e botões Anterior/Próximo:

1. **O que é uma FCU** — diferença entre AEG eletrônica e HPA, qual papel a FCU faz no airsoft HPA.
2. **Engines: S8PA vs D8PA** — single vs dual solenoid, exemplos (Jack/Backdraft vs F2/Pulsar/Wolverine), implicação prática (D8PA tem nozzle independente, alimentação mais consistente).
3. **Ciclo de disparo D8PA** — animação ou diagrama: pulso SOL2 (DN) → espera (DR) → pulso SOL1 (DP) → espera pós-tiro (DB) → repete.
4. **Ciclo de disparo S8PA** — só pulso SOL1 (DP) → espera (DR) → repete. DN e DB inativos.
5. **DN — Nozzle Dwell** — para que serve, sintomas de DN curto (falha de alimentação) vs longo (desperdício de gás). Faixa, padrão.
6. **DR — Dwell Rest** — em D8PA: espera entre nozzle e poppet pra mola vedar o bucking. Em S8PA: descanso entre tiros. Faixa, padrão.
7. **DP — Dwell Poppet** — duração do pulso de tiro. Sintomas: DP curto = FPS baixo; DP longo = desperdício de gás + risco de tiro duplo. Tunar no chrono.
8. **DB — Trigger Debounce** — pausa entre ciclos pra BB sair do cano antes do próximo tiro (D8PA). Faixa nova 5–150 unidades de 0,1 ms.
9. **ROF max e ROF teórico** — diferença entre limite e teórico, como o teórico é calculado.
10. **Anti-stiction (IS + IP)** — bloco completo conforme descrito acima.
11. **Seletor: Pos1/Pos2/Pos3 e modos de fogo** — Safe/Semi/Full/Burst2/3/4.
12. **Microswitch vs Hall** — quando escolher cada um, calibração necessária para Hall.
13. **Calibração Hall** — fluxo de noise + posições (gatilho solto/pressionado, ou 3 posições do seletor).
14. **Modo silencioso e MOSFET swap** — quando usar.
15. **Config LOCK e Factory Reset por gesture** — como travar configuração pra evitar mexida acidental, como recuperar se esquecer (gesto físico no boot).
16. **Estatísticas em tempo real** — bateria, tiros, próxima lubrificação a cada 10k.

Pode reagrupar ou cortar capítulos conforme fizer sentido visualmente. Mantenha foco em "tuning-first" — toda explicação de timing leva ao "o que aumentar/diminuir pra qual sintoma".

## Modo Resolver problema — catálogo de sintomas

Sugestão de input: lista de sintomas clicáveis (mais robusto que parsing de texto livre numa primeira versão). Cada item leva a uma tela com:
- Diagnóstico provável
- Setting a verificar
- Range/passo sugerido pra ajuste
- Avisos

Catálogo mínimo:

| Sintoma | Causa provável | Onde mexer |
|---|---|---|
| Arma não dispara | Bateria descarregada / seletor em SAFE / fire mode SAFE em todas as posições | Conferir bateria · Verificar selPos1/2/3 |
| FPS baixo geral | DP curto / regulador baixo / vazamento na engine | Subir DP (testar no chrono) · Conferir pressão · Inspecionar o-rings |
| Primeiro tiro fraco após arma parada | Stiction do o-ring do poppet | **Habilitar IP (=1) com IS=30s**; subir IP até estabilizar |
| FPS inconsistente tiro a tiro | Vazamento intermitente / regulador instável / BB mass inconsistente | Trocar BBs · Inspecionar regulador · Conferir vazamentos |
| Falha de alimentação (BB não chega) | DN muito curto (D8PA) / nozzle gasto / mola fraca | Subir DN · Trocar nozzle · Trocar mola da torre |
| Cadência muito alta (incontrolável) | DR curto / DB curto / sem ROF cap | Subir DR · Subir DB · Definir ROF max |
| Cadência muito baixa | Soma de timings alta | Reduzir DN, DR, DB pelo menor valor que ainda funcione |
| Tiros duplos em SEMI | Trigger ricochete / Hall mal calibrado | Habilitar Semi Max ROF (tente 100ms) · Recalibrar Hall com noise |
| Cadência sobe e desce em FULL | ROF cap zerado + bateria caindo | Definir ROF max abaixo do pico real |
| Esqueci a senha do Config LOCK | Sem recuperação digital | Factory reset por gesture: gatilho 30s no boot (STR) ou gatilho + botão WiFi 30s (PRO) |
| Seletor Hall lê posição errada | Calibração de seletor desatualizada | Calibração do seletor (3 posições) — fluxo Hall |
| Gatilho Hall dispara sozinho | Ruído eletromagnético do solenoide | Calibração de ruído ANTES da calibração do gatilho |

Adicione mais conforme fizer sentido. Para cada solução, link pra explicação no modo "Aprender" se houver.

## Estilo

- Mantenha a paleta atual do site bcgaairsoft.com (memory note: "site is the visual source of truth — pull palette/fonts from there"). Não invente cores novas.
- O wizard runtime usa accent verde (`#8aab47` STR / `#00bcd4` PRO). O site provavelmente já tem palette própria — siga o do site.
- Toggle PT/EN persistente no header. Idioma default = EN.

## Entregáveis

1. Tela inicial 3-menus integrada ao wizard.
2. Modo "Aprender" com pelo menos 12 dos 16 capítulos sugeridos.
3. Modo "Resolver problema" com pelo menos 8 dos 12 sintomas do catálogo.
4. Modo "Configurar do zero" atualizado com todos os campos/defaults/textos novos.
5. README curto explicando como rodar localmente e onde editar conteúdo (capítulos do Aprender e catálogo de Resolver), porque eles vão crescer.

## O que NÃO fazer

- Não copiar o `firmware/web_ui_dev/` direto — aquilo é UI de configuração runtime, este wizard tem propósito diferente (pré-flash, educacional).
- Não inventar campos que não existam no firmware.
- Não escrever em `firmware/` neste repo — você está no repo do site.
- Não traduzir nomes técnicos consagrados (DN, DR, DP, DB, IS, IP, S8PA, D8PA, F2, Jack, Pulsar) — só os textos de ajuda em volta.

Quando terminar, mostre URL local pra eu testar e abra um PR descrevendo as mudanças.
