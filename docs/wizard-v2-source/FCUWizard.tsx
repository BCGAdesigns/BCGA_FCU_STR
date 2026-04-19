import { useEffect, useMemo, useState } from 'react'
import { Link, useNavigate, useSearchParams } from 'react-router-dom'
import Layout from '../components/layout/Layout'
import SEO from '../components/SEO'
import { useGeo } from '../contexts/GeoContext'
import { useAuth } from '../contexts/AuthContext'
import { saveFcuConfig, fetchFcuConfig } from '../lib/api'

type Lang = 'pt' | 'en'
type Engine = 0 | 1 | 2
type TargetMode = 'fps' | 'joule'

interface ChronoReading { s1: number | null; s2: number | null; s3: number | null }
interface Timings { dn: number | null; dr: number | null; dp: number | null; db: number | null }

interface WizardState {
  step: number
  engine: Engine
  firstBoot: boolean | null
  currentTimings: Timings
  bbMass: number
  sweep: {
    psi80: ChronoReading
    psi100: ChronoReading
    psi120: ChronoReading
    hopup: ChronoReading
  }
  target: { psi: number; mode: TargetMode; value: number }
  finalTimings: Timings
  dpIterations: Array<{ dp: number; chrono: ChronoReading }>
  customBbMass: number | null
}

const STORE_KEY = 'bcga_fcu_helper_v2'
const DEFAULT_FIRST_BOOT: Timings = { dn: 18, dr: 26, dp: 80, db: 100 }
const EMPTY_CHRONO: ChronoReading = { s1: null, s2: null, s3: null }

const defaultState = (): WizardState => ({
  step: 1,
  engine: 0,
  firstBoot: null,
  currentTimings: { dn: null, dr: null, dp: null, db: null },
  bbMass: 0.28,
  sweep: {
    psi80: { ...EMPTY_CHRONO },
    psi100: { ...EMPTY_CHRONO },
    psi120: { ...EMPTY_CHRONO },
    hopup: { ...EMPTY_CHRONO },
  },
  target: { psi: 100, mode: 'fps', value: 350 },
  finalTimings: { dn: null, dr: null, dp: null, db: null },
  dpIterations: [],
  customBbMass: null,
})

const loadState = (): WizardState => {
  try {
    const raw = localStorage.getItem(STORE_KEY)
    if (!raw) return defaultState()
    const parsed = JSON.parse(raw)
    const base = defaultState()
    return {
      ...base,
      ...parsed,
      currentTimings: { ...base.currentTimings, ...(parsed.currentTimings || {}) },
      sweep: { ...base.sweep, ...(parsed.sweep || {}) },
      target: { ...base.target, ...(parsed.target || {}) },
      finalTimings: { ...base.finalTimings, ...(parsed.finalTimings || {}) },
      dpIterations: Array.isArray(parsed.dpIterations) ? parsed.dpIterations : [],
    }
  } catch {
    return defaultState()
  }
}

// ─── i18n ────────────────────────────────────────────────
const I18N: Record<Lang, Record<string, string>> = {
  pt: {
    title: 'BCGA FCU — Ajuda de Configuração',
    subtitle: 'Open source — compatível com BCGA FCU, PolarStar, Wolverine, Backdraft e outras FCUs HPA.',
    github: 'Ver no GitHub',
    backFcu: 'Voltar para FCU',
    'step.engine': 'Engine',
    'step.boot': 'Setup',
    'step.bb': 'BB',
    'step.p80': '80 psi',
    'step.p100': '100 psi',
    'step.p120': '120 psi',
    'step.hop': 'Hopup',
    'step.target': 'Alvo',
    'step.dp': 'Ajuste DP',
    'step.dn': 'DN',
    'step.dr': 'DR',
    'step.dpFinal': 'DP final',
    'step.db': 'DB',
    'step.results': 'Resultado',
    'nav.back': '← Voltar',
    'nav.next': 'Próximo →',
    'nav.skip': 'Pular',
    // STEP 1 Engine
    's1.title': '1. Qual engine HPA está usando?',
    's1.lead': 'Isso define quais timings o wizard vai ajustar e como a matriz é calculada.',
    's1.s8.sub': '1 solenoide',
    's1.s8.body': 'PolarStar JACK/F1, Wolverine INFERNO, GATE PULSAR S. Apenas DR e DP.',
    's1.d8.sub': '2 solenoides',
    's1.d8.body': 'BCGA D8PA, PolarStar F2/Fusion, GATE PULSAR D. DN + DR + DP + DB.',
    's1.crossNote': 'Os nomes DN/DR/DP/DB são a convenção da BCGA. Outras marcas usam termos diferentes (ver tabela de mapeamento no resultado final).',
    // STEP 2 First boot
    's2.title': '2. É a primeira configuração?',
    's2.lead': 'Se já usa o setup, podemos partir dos timings atuais. Senão, usamos defaults seguros.',
    's2.yes': 'Primeira vez — usar defaults',
    's2.yesSub': 'DN=18 · DR=26 · DP=80 · DB=100 (units)',
    's2.no': 'Já tenho valores — vou digitar',
    's2.noSub': 'Informe DN / DR / DP / DB atuais para partirmos deles.',
    's2.currentTitle': 'Timings atuais',
    's2.dn': 'DN (ms)',
    's2.dr': 'DR (ms)',
    's2.dp': 'DP (ms)',
    's2.db': 'DB (units)',
    // STEP 3 BB
    's3.title': '3. Qual massa de BB você vai usar para as leituras?',
    's3.lead': 'Todos os cálculos de energia partem dessa massa. 0.28g é o padrão em campos BR.',
    's3.label': 'Massa BB (g)',
    // STEP 4-7 chrono sweeps (generic)
    's45.lead': 'Mire no chrono e registre 3 tiros consecutivos.',
    's4.title': '4. Leitura em 80 psi — 3 tiros',
    's5.title': '5. Leitura em 100 psi — 3 tiros',
    's6.title': '6. Leitura em 120 psi — 3 tiros',
    's6.warn': 'Não exceda 120 psi sem validar o hardware. Se a vedação é nova, pare em 110.',
    's7.title': '7. Hopup apertado (360°) — 3 tiros',
    's7.lead': 'Aperte o hopup até quase não deixar a BB cair. Atire 3 vezes no chrono (pressão da última etapa).',
    'shot': 'Tiro',
    'mean': 'Média',
    'spread': 'Spread',
    'spreadWarn': 'Spread > 15 FPS — verifique vedação / bucking antes de seguir.',
    'hopGain': 'Ganho do hopup',
    // STEP 8 target
    's8.title': '8. Qual é seu alvo?',
    's8.lead': 'Pressão que vai rodar no jogo e critério (FPS ou Joule).',
    's8.psi': 'Pressão de jogo (psi)',
    's8.mode': 'Critério',
    's8.modeFps': 'FPS',
    's8.modeJ': 'Joule',
    's8.value': 'Alvo',
    's8.fieldCqb': '> 1,14 J — acima do limite típico CQB/Indoor BR.',
    's8.fieldOut': '> 2,32 J — acima do limite típico outdoor BR (policiais/militares).',
    // STEP 9 DP
    's9.title': '9. Ajuste iterativo do DP',
    's9.lead': 'Vamos ajustar o DP até o FPS estabilizar no alvo na pressão de jogo. Faça iterações: mude o DP, atire 3 vezes, registre. O helper sugere o próximo passo.',
    's9.dpField': 'DP atual (ms)',
    's9.add': 'Registrar iteração',
    's9.suggest': 'Sugestão',
    's9.stop': 'Estabilizou — seguir',
    's9.iter': 'Iteração',
    's9.noIter': 'Nenhuma iteração registrada ainda.',
    's9.tooShort': 'DP < 20 ms em D8PA geralmente perde FPS — cuidado.',
    // STEP 10 DN deep-dive
    's10.title': '10. DN — Dwell do nozzle',
    's10.lead': 'Tempo que o solenoide do nozzle mantém o nozzle avançado durante a alimentação. Relevante só em D8PA (dual-solenoid).',
    's10.na': 'Não aplicável em S8PA. Engines single-solenoid usam DP também para alimentação — não há DN independente.',
    's10.what': 'O que é: tempo (ms) que o solenoide do nozzle mantém o nozzle avançado e selado contra o bucking durante a alimentação da BB. Em FCUs dual-solenoid (D8PA, PolarStar F2, GATE PULSAR D) o DN é independente do pulso de tiro (DP), o que permite alimentação consistente mesmo com pulsos de tiro curtos.\n\nO que controla:\n• Tempo físico para a BB sair do magazine, entrar no hop-up e ser empurrada até a posição de disparo.\n• Vedação entre nozzle e bucking no instante do disparo.\n• Consumo de ar por ciclo (DN maior = mais ar por ciclo).',
    's10.symptoms': 'Sintomas de DN errado:\n• Curto demais → nozzle recua antes da BB ser empurrada → empty shots, misfeeds, BB cortada ao meio.\n• Longo demais → rate of fire cai em full-auto; consumo de ar cresce sem ganho de FPS.',
    's10.rule': 'Como ajustar:\n1. Parta de um DN seguro (first-boot: 18 ms).\n2. Diminua DN em 1 ms por vez.\n3. Atire rajadas de 5–10 tiros em full-auto.\n4. Quando aparecer tiro vazio, volte +1 a +2 ms.\n5. Esse é o seu DN mínimo seguro.',
    's10.brands': 'Equivalentes noutras marcas:\n• PolarStar F2 / Fusion: "Nozzle Dwell"\n• Wolverine INFERNO: "Dwell 1"\n• GATE PULSAR D: equivalente proprietário',
    's10.field': 'DN final (ms)',
    // STEP 11 DR deep-dive
    's11.title': '11. DR — Dwell de retorno',
    's11.lead': 'Tempo de reset entre disparos. Controla estabilidade de FPS em SEMI rápido e rate of fire em full-auto.',
    's11.what': 'O que é: intervalo (ms) entre o fim de um disparo e o início do próximo — o "reset" do sistema. DR existe tanto em single-solenoid (S8PA) quanto em dual-solenoid (D8PA).\n\nO que controla:\n• Tempo para a pressão na câmara estabilizar antes do próximo disparo.\n• Tempo para o bucking voltar ao estado de repouso.\n• Rate of fire máximo em full-auto.',
    's11.symptoms': 'Sintomas de DR errado:\n• Curto demais → câmara não re-pressuriza → FPS oscila em SEMI rápido (2º/3º tiros fracos).\n• Longo demais → rate of fire cai sem ganho de estabilidade.',
    's11.rule': 'Como ajustar:\n1. Atire 5 tiros em SEMI o mais rápido que conseguir.\n2. Compare FPS do 1º e do 5º tiro no chrono.\n3. Se FPS caiu >5 FPS, aumente DR em +1 ms. Repita.\n4. Quando FPS se mantém estável em SEMI rápido, você achou o DR mínimo seguro.',
    's11.brands': 'Equivalentes noutras marcas:\n• PolarStar F2: "Return"\n• Wolverine INFERNO: "Dwell 2"\n• GATE PULSAR D: proprietário',
    's11.field': 'DR final (ms)',
    // STEP 12 DP deep-dive (valor do passo 9)
    's12.title': '12. DP — Dwell do poppet',
    's12.lead': 'O timing mais importante para FPS. Já ajustado iterativamente no passo 9; este passo confirma e explica.',
    's12.what': 'O que é: duração (ms) do pulso de ar que sai do engine e propulsiona a BB. DP abre a válvula poppet por N ms — quanto maior DP, mais ar passa, maior FPS.\n\nRelação com FPS:\n• Linear até um ponto: cada +1 ms ≈ +3 a +5 FPS em 0.28 g a 100 psi.\n• Acima do ponto de saturação, DP extra só desperdiça ar (a BB já saiu do cano).\n• Abaixo de ~20 ms em D8PA: perda severa de FPS (poppet não abre completamente).',
    's12.symptoms': 'Sintomas de DP errado:\n• Curto → FPS baixo, consumo baixo, variância alta.\n• Longo → FPS alto porém com consumo desnecessário; em D8PA pode gerar "back-feed" se exceder DN.',
    's12.rule': 'Como é ajustado: no passo 9 você aplicou o método iterativo (atirar 3 vezes, comparar com alvo, ajustar DP ±). Este passo só confirma o valor final. Se quiser refinar, volte ao passo 9.',
    's12.brands': 'Equivalentes noutras marcas:\n• PolarStar F2: "Poppet Dwell"\n• Wolverine INFERNO: "Delay"\n• GATE PULSAR D: proprietário',
    's12.fromStep9': 'Valor final do passo 9',
    's12.goStep9': '← Ir ao passo 9',
    // STEP 13 DB deep-dive
    's13.title': '13. DB — Debounce',
    's13.lead': 'Filtro anti-repique do switch do gatilho. Afeta principalmente full-auto. Relevante só em D8PA.',
    's13.na': 'Não aplicável em S8PA. Engines single-solenoid típicos não expõem DB como parâmetro independente.',
    's13.what': 'O que é: tempo (em units — 1 unit ≈ 0.1 ms) que o FCU aguarda após um disparo antes de aceitar o próximo sinal do micro-switch do gatilho. É um filtro anti-repique (debounce) mecânico.\n\nO que controla:\n• Em full-auto: se dois pulsos do switch chegarem dentro da janela DB, o segundo é ignorado.\n• Em SEMI: filtra bouncing do micro-switch entre pulls.\n• Rate of fire máximo em full-auto.',
    's13.symptoms': 'Sintomas de DB errado:\n• Curto → flyers em full-auto (tiros duplicados), cycle errático, dois BBs saem no mesmo pulso.\n• Longo → rate of fire cai; em SEMI-rapid alguns pulls são perdidos.',
    's13.rule': 'Como ajustar:\n1. Configure full-auto e encha o mag.\n2. Atire rajada longa (15–20 tiros).\n3. Se rate of fire parece irregular ou BBs saem em dupla → DB curto → +20 units (+2 ms).\n4. Se rate of fire caiu muito → DB longo → -10 units.',
    's13.brands': 'Equivalentes noutras marcas:\n• PolarStar F2: "Debounce"\n• Wolverine INFERNO: — (não há equivalente direto no single-solenoid)\n• GATE PULSAR D: "Trigger Debounce"',
    's13.field': 'DB final (units)',
    // STEP 14 results (matriz)
    's14.title': 'Matriz FPS × Joule',
    's14.lead': 'Simulação a partir das suas leituras (conservação de energia). Use como referência, valide no chrono.',
    's14.psiCol': 'Pressão',
    's14.bbCol': 'Massa BB',
    's14.targetCol': 'Alvo',
    's14.extrapWarn': '⚠ Extrapolação > 10 psi fora da faixa medida — valide.',
    's14.safety': 'Segurança',
    's14.safetyList': '• Nunca exceda a pressão que seu hardware foi validado.\n• Use chrono em toda alteração.\n• Valide a vedação antes de subir PSI.\n• > 2,32 J está fora dos limites de quase todos os campos BR.',
    's14.cross': 'Cross-brand',
    's14.crossList': 'BCGA: DN / DR / DP / DB\nPolarStar F2: Nozzle Dwell / Return / Poppet Dwell / Debounce\nWolverine INFERNO: Dwell 1 / Dwell 2 / Delay / —\nGATE PULSAR D: equivalentes com nomes próprios',
    's14.copy': 'Copiar JSON',
    's14.copied': 'Copiado!',
    's14.pdf': 'Baixar PDF',
    's14.save': 'Salvar na minha conta',
    's14.restart': 'Recomeçar',
    's14.loginNote': 'Faça login para salvar múltiplas configs na sua conta.',
    'deep.what': 'O que é',
    'deep.symptoms': 'Sintomas',
    'deep.rule': 'Como ajustar',
    'deep.brands': 'Cross-brand',
    'saveModal.title': 'Salvar configuração',
    'saveModal.name': 'Nome',
    'saveModal.ph': 'Ex.: D8PA jogo 100 psi 0.28',
    'saveModal.cancel': 'Cancelar',
    'saveModal.save': 'Salvar',
    'saveModal.saved': 'Salvo!',
  },
  en: {
    title: 'BCGA FCU — Config Helper',
    subtitle: 'Open source — works with BCGA FCU, PolarStar, Wolverine, Backdraft and other HPA FCUs.',
    github: 'View on GitHub',
    backFcu: 'Back to FCU',
    'step.engine': 'Engine',
    'step.boot': 'Setup',
    'step.bb': 'BB',
    'step.p80': '80 psi',
    'step.p100': '100 psi',
    'step.p120': '120 psi',
    'step.hop': 'Hopup',
    'step.target': 'Target',
    'step.dp': 'DP tune',
    'step.dn': 'DN',
    'step.dr': 'DR',
    'step.dpFinal': 'DP final',
    'step.db': 'DB',
    'step.results': 'Result',
    'nav.back': '← Back',
    'nav.next': 'Next →',
    'nav.skip': 'Skip',
    's1.title': '1. Which HPA engine are you using?',
    's1.lead': 'This drives which timings the helper tunes and how the matrix is computed.',
    's1.s8.sub': '1 solenoid',
    's1.s8.body': 'PolarStar JACK/F1, Wolverine INFERNO, GATE PULSAR S. Only DR and DP.',
    's1.d8.sub': '2 solenoids',
    's1.d8.body': 'BCGA D8PA, PolarStar F2/Fusion, GATE PULSAR D. DN + DR + DP + DB.',
    's1.crossNote': 'DN/DR/DP/DB are BCGA naming. Other brands use different terms (see mapping in the final results).',
    's2.title': '2. First-time setup?',
    's2.lead': 'If the setup already runs, we start from your current timings. Otherwise, safe defaults.',
    's2.yes': 'First boot — use defaults',
    's2.yesSub': 'DN=18 · DR=26 · DP=80 · DB=100 (units)',
    's2.no': 'I have values — let me type them',
    's2.noSub': 'Enter current DN / DR / DP / DB as the starting point.',
    's2.currentTitle': 'Current timings',
    's2.dn': 'DN (ms)',
    's2.dr': 'DR (ms)',
    's2.dp': 'DP (ms)',
    's2.db': 'DB (units)',
    's3.title': '3. Which BB mass will you chrono with?',
    's3.lead': 'All energy math uses this mass. 0.28g is a common default.',
    's3.label': 'BB mass (g)',
    's45.lead': 'Fire 3 shots over the chrono and enter the readings.',
    's4.title': '4. Chrono at 80 psi — 3 shots',
    's5.title': '5. Chrono at 100 psi — 3 shots',
    's6.title': '6. Chrono at 120 psi — 3 shots',
    's6.warn': "Don't exceed 120 psi without validating hardware. Stop at 110 if the seal is new.",
    's7.title': '7. Hopup tight (360°) — 3 shots',
    's7.lead': 'Tighten hopup until the BB barely drops. Fire 3 shots (same pressure as last step).',
    'shot': 'Shot',
    'mean': 'Mean',
    'spread': 'Spread',
    'spreadWarn': 'Spread > 15 FPS — check sealing / bucking before proceeding.',
    'hopGain': 'Hopup gain',
    's8.title': '8. Target',
    's8.lead': 'In-game pressure and criterion (FPS or Joule).',
    's8.psi': 'Game pressure (psi)',
    's8.mode': 'Criterion',
    's8.modeFps': 'FPS',
    's8.modeJ': 'Joule',
    's8.value': 'Target',
    's8.fieldCqb': '> 1.14 J — above typical CQB/Indoor caps.',
    's8.fieldOut': '> 2.32 J — above typical BR outdoor caps (police/military).',
    's9.title': '9. Iterative DP tuning',
    's9.lead': 'Tune DP until FPS stabilises at target on the game pressure. Change DP, fire 3 shots, record. Helper suggests the next step.',
    's9.dpField': 'Current DP (ms)',
    's9.add': 'Record iteration',
    's9.suggest': 'Suggestion',
    's9.stop': 'Stable — continue',
    's9.iter': 'Iteration',
    's9.noIter': 'No iterations recorded yet.',
    's9.tooShort': 'DP < 20 ms on D8PA usually loses FPS — careful.',
    's10.title': '10. DN — Nozzle dwell',
    's10.lead': 'Time the nozzle solenoid holds the nozzle forward during feeding. D8PA (dual-solenoid) only.',
    's10.na': 'Not applicable on S8PA. Single-solenoid engines use DP for feeding too — no independent DN.',
    's10.what': 'What it is: time (ms) the nozzle solenoid keeps the nozzle forward and sealed against the bucking during BB feeding. On dual-solenoid FCUs (D8PA, PolarStar F2, GATE PULSAR D) DN is independent from the firing pulse (DP), enabling consistent feeding with short firing pulses.\n\nWhat it controls:\n• Physical time for the BB to exit magazine, enter hop-up and be pushed to firing position.\n• Seal between nozzle and bucking at firing instant.\n• Air consumption per cycle (higher DN = more air per cycle).',
    's10.symptoms': 'Symptoms of wrong DN:\n• Too short → nozzle retracts before BB is pushed → empty shots, misfeeds, chopped BBs.\n• Too long → rate of fire drops on full-auto; air consumption grows without FPS gain.',
    's10.rule': 'How to tune:\n1. Start from a safe DN (first-boot: 18 ms).\n2. Decrease DN by 1 ms at a time.\n3. Fire 5–10 shot bursts on full-auto.\n4. When empty shots appear, add +1 to +2 ms back.\n5. That is your minimum safe DN.',
    's10.brands': 'Equivalents on other brands:\n• PolarStar F2 / Fusion: "Nozzle Dwell"\n• Wolverine INFERNO: "Dwell 1"\n• GATE PULSAR D: proprietary equivalent',
    's10.field': 'Final DN (ms)',
    's11.title': '11. DR — Return dwell',
    's11.lead': 'Reset time between shots. Controls FPS stability on rapid SEMI and full-auto rate of fire.',
    's11.what': 'What it is: interval (ms) between end of one shot and start of the next — the "reset" of the system. DR exists on both single-solenoid (S8PA) and dual-solenoid (D8PA) engines.\n\nWhat it controls:\n• Time for chamber pressure to stabilise before the next shot.\n• Time for bucking to return to rest state.\n• Max rate of fire on full-auto.',
    's11.symptoms': 'Symptoms of wrong DR:\n• Too short → chamber does not re-pressurise → FPS oscillates on rapid SEMI (weak 2nd/3rd shots).\n• Too long → rate of fire drops without stability gain.',
    's11.rule': 'How to tune:\n1. Fire 5 shots on SEMI as fast as you can.\n2. Compare 1st and 5th shot FPS on chrono.\n3. If FPS dropped >5 FPS, raise DR by +1 ms. Repeat.\n4. When FPS stays stable on rapid SEMI, you have your minimum safe DR.',
    's11.brands': 'Equivalents on other brands:\n• PolarStar F2: "Return"\n• Wolverine INFERNO: "Dwell 2"\n• GATE PULSAR D: proprietary',
    's11.field': 'Final DR (ms)',
    's12.title': '12. DP — Poppet dwell',
    's12.lead': 'The most important timing for FPS. Already tuned iteratively in step 9; this step confirms and explains.',
    's12.what': 'What it is: duration (ms) of the air pulse that exits the engine and propels the BB. DP opens the poppet valve for N ms — the larger DP, the more air passes and the higher FPS goes.\n\nRelation with FPS:\n• Linear up to a point: every +1 ms ≈ +3 to +5 FPS on 0.28 g @ 100 psi.\n• Past saturation, extra DP just wastes air (BB already left the barrel).\n• Below ~20 ms on D8PA: severe FPS loss (poppet does not open fully).',
    's12.symptoms': 'Symptoms of wrong DP:\n• Short → low FPS, low consumption, high variance.\n• Long → high FPS but wasteful consumption; on D8PA may cause back-feed if exceeding DN.',
    's12.rule': 'How it was tuned: in step 9 you applied the iterative method (fire 3 shots, compare to target, adjust DP ±). This step only confirms the final value. To refine, go back to step 9.',
    's12.brands': 'Equivalents on other brands:\n• PolarStar F2: "Poppet Dwell"\n• Wolverine INFERNO: "Delay"\n• GATE PULSAR D: proprietary',
    's12.fromStep9': 'Final value from step 9',
    's12.goStep9': '← Go to step 9',
    's13.title': '13. DB — Debounce',
    's13.lead': 'Anti-bounce filter for the trigger switch. Mostly affects full-auto. D8PA only.',
    's13.na': 'Not applicable on S8PA. Typical single-solenoid engines do not expose DB as a separate parameter.',
    's13.what': 'What it is: time (in units — 1 unit ≈ 0.1 ms) the FCU waits after a shot before accepting the next trigger switch pulse. It is a mechanical bounce filter.\n\nWhat it controls:\n• On full-auto: if two switch pulses arrive within the DB window, the second is ignored.\n• On SEMI: filters bouncing of the micro-switch between pulls.\n• Max full-auto rate of fire.',
    's13.symptoms': 'Symptoms of wrong DB:\n• Short → flyers on full-auto (double shots), erratic cycle, two BBs on the same pulse.\n• Long → rate of fire drops; on rapid SEMI some pulls are missed.',
    's13.rule': 'How to tune:\n1. Set full-auto and load a full mag.\n2. Fire a long burst (15–20 shots).\n3. If rate of fire feels irregular or double BBs appear → DB too short → +20 units (+2 ms).\n4. If rate of fire dropped heavily → DB too long → -10 units.',
    's13.brands': 'Equivalents on other brands:\n• PolarStar F2: "Debounce"\n• Wolverine INFERNO: — (no direct equivalent on single-solenoid)\n• GATE PULSAR D: "Trigger Debounce"',
    's13.field': 'Final DB (units)',
    's14.title': 'FPS × Joule matrix',
    's14.lead': 'Simulation from your readings (energy conservation). Reference only — always verify with a chrono.',
    's14.psiCol': 'Pressure',
    's14.bbCol': 'BB mass',
    's14.targetCol': 'Target',
    's14.extrapWarn': '⚠ Extrapolation > 10 psi outside measured range — validate.',
    's14.safety': 'Safety',
    's14.safetyList': '• Never exceed validated hardware pressure.\n• Chrono every change.\n• Validate sealing before raising PSI.\n• > 2.32 J exceeds most BR field caps.',
    's14.cross': 'Cross-brand',
    's14.crossList': 'BCGA: DN / DR / DP / DB\nPolarStar F2: Nozzle Dwell / Return / Poppet Dwell / Debounce\nWolverine INFERNO: Dwell 1 / Dwell 2 / Delay / —\nGATE PULSAR D: brand-named equivalents',
    's14.copy': 'Copy JSON',
    's14.copied': 'Copied!',
    's14.pdf': 'Download PDF',
    's14.save': 'Save to my account',
    's14.restart': 'Restart',
    's14.loginNote': 'Log in to save multiple configs to your account.',
    'deep.what': 'What it is',
    'deep.symptoms': 'Symptoms',
    'deep.rule': 'How to tune',
    'deep.brands': 'Cross-brand',
    'saveModal.title': 'Save config',
    'saveModal.name': 'Name',
    'saveModal.ph': 'e.g. D8PA field 100 psi 0.28',
    'saveModal.cancel': 'Cancel',
    'saveModal.save': 'Save',
    'saveModal.saved': 'Saved!',
  },
}

// ─── Math helpers ────────────────────────────────────────
const DEFAULT_MASSES = [0.20, 0.25, 0.28, 0.30, 0.32]

function valuesOf(c: ChronoReading): number[] {
  return [c.s1, c.s2, c.s3].filter((x): x is number => typeof x === 'number' && Number.isFinite(x) && x > 0)
}
function meanOf(c: ChronoReading): number | null {
  const v = valuesOf(c)
  if (v.length === 0) return null
  return v.reduce((a, b) => a + b, 0) / v.length
}
function spreadOf(c: ChronoReading): number | null {
  const v = valuesOf(c)
  if (v.length < 2) return null
  return Math.max(...v) - Math.min(...v)
}
function isChronoComplete(c: ChronoReading): boolean {
  return valuesOf(c).length === 3
}
function fpsToMs(fps: number): number { return fps * 0.3048 }
function energyJ(fps: number, massG: number): number {
  const v = fpsToMs(fps)
  return 0.5 * (massG / 1000) * v * v
}
// Convert FPS reading with reference mass to FPS with target mass (conservation of energy).
function fpsForMass(fpsRef: number, massRef: number, massTarget: number): number {
  return fpsRef * Math.sqrt(massRef / massTarget)
}
// Linear regression through the 3 PSI data points; returns FPS at arbitrary PSI.
function fpsAtPsi(
  mean80: number | null,
  mean100: number | null,
  mean120: number | null,
  psi: number
): { fps: number | null; extrapolated: boolean } {
  const pts: Array<[number, number]> = []
  if (mean80 != null) pts.push([80, mean80])
  if (mean100 != null) pts.push([100, mean100])
  if (mean120 != null) pts.push([120, mean120])
  if (pts.length === 0) return { fps: null, extrapolated: false }
  if (pts.length === 1) return { fps: pts[0][1], extrapolated: psi !== pts[0][0] }
  const n = pts.length
  const sx = pts.reduce((a, p) => a + p[0], 0)
  const sy = pts.reduce((a, p) => a + p[1], 0)
  const sxx = pts.reduce((a, p) => a + p[0] * p[0], 0)
  const sxy = pts.reduce((a, p) => a + p[0] * p[1], 0)
  const denom = n * sxx - sx * sx
  if (denom === 0) return { fps: sy / n, extrapolated: true }
  const m = (n * sxy - sx * sy) / denom
  const b = (sy - m * sx) / n
  const minPsi = Math.min(...pts.map((p) => p[0]))
  const maxPsi = Math.max(...pts.map((p) => p[0]))
  const extrapolated = psi < minPsi - 10 || psi > maxPsi + 10
  return { fps: m * psi + b, extrapolated }
}

// ─── Component ───────────────────────────────────────────
const TOTAL_STEPS = 14

export default function FCUWizard() {
  const { isBR } = useGeo()
  const { user } = useAuth()
  const navigate = useNavigate()
  const [searchParams] = useSearchParams()
  const lang: Lang = isBR ? 'pt' : 'en'
  const t = (k: string) => I18N[lang][k] ?? k

  const [state, setState] = useState<WizardState>(loadState)
  const [copied, setCopied] = useState(false)
  const [showSaveModal, setShowSaveModal] = useState(false)

  // Load saved config if ?load=<id>
  useEffect(() => {
    const loadId = searchParams.get('load')
    if (!loadId || !user) return
    const id = Number(loadId)
    if (!Number.isFinite(id)) return
    fetchFcuConfig(id)
      .then(({ config }) => {
        if (config && config.state_json) {
          const parsed = typeof config.state_json === 'string'
            ? JSON.parse(config.state_json)
            : config.state_json
          setState({ ...defaultState(), ...parsed })
          navigate('/fcu/wizard', { replace: true })
        }
      })
      .catch(() => {})
  }, [searchParams, user, navigate])

  useEffect(() => {
    try { localStorage.setItem(STORE_KEY, JSON.stringify(state)) } catch {}
  }, [state])
  useEffect(() => { window.scrollTo({ top: 0, behavior: 'smooth' }) }, [state.step])

  const isD8 = state.engine === 2

  const means = {
    p80: meanOf(state.sweep.psi80),
    p100: meanOf(state.sweep.psi100),
    p120: meanOf(state.sweep.psi120),
    hop: meanOf(state.sweep.hopup),
  }

  // Step navigation with branching for S8 (skip DN/DB-only actions still flows the same)
  const goStep = (n: number) => setState((s) => ({ ...s, step: Math.max(1, Math.min(TOTAL_STEPS, n)) }))

  const pickEngine = (engine: Engine) => setState((s) => ({ ...s, engine, step: 2 }))

  const setFirstBoot = (v: boolean) => setState((s) => ({
    ...s,
    firstBoot: v,
    currentTimings: v ? { ...DEFAULT_FIRST_BOOT } : s.currentTimings,
    // Seed finalTimings DP from the starting point so step 9 has a base.
    finalTimings: {
      ...s.finalTimings,
      dp: s.finalTimings.dp ?? (v ? DEFAULT_FIRST_BOOT.dp : s.currentTimings.dp),
    },
    step: v ? 3 : 2,
  }))

  const updateCurrentTiming = (key: keyof Timings, val: number | null) =>
    setState((s) => ({ ...s, currentTimings: { ...s.currentTimings, [key]: val } }))

  const updateSweep = (k: keyof WizardState['sweep'], idx: 's1'|'s2'|'s3', val: number | null) =>
    setState((s) => ({ ...s, sweep: { ...s.sweep, [k]: { ...s.sweep[k], [idx]: val } } }))

  const setBbMass = (v: number) => setState((s) => ({ ...s, bbMass: v }))
  const setTarget = (patch: Partial<WizardState['target']>) =>
    setState((s) => ({ ...s, target: { ...s.target, ...patch } }))

  const setFinalTiming = (k: keyof Timings, v: number | null) =>
    setState((s) => ({ ...s, finalTimings: { ...s.finalTimings, [k]: v } }))

  const addDpIteration = (dp: number, chrono: ChronoReading) =>
    setState((s) => ({ ...s, dpIterations: [...s.dpIterations, { dp, chrono }] }))

  const clearDpIterations = () => setState((s) => ({ ...s, dpIterations: [] }))

  const restart = () => {
    try { localStorage.removeItem(STORE_KEY) } catch {}
    setState(defaultState())
  }

  // Derived: target FPS (if target is in Joule, convert back to FPS)
  const targetFps = state.target.mode === 'fps'
    ? state.target.value
    : (() => {
        const m = state.bbMass / 1000
        if (m <= 0) return 0
        const v = Math.sqrt((2 * state.target.value) / m)
        return v / 0.3048
      })()

  // DP suggestion: compare last iteration mean to targetFps
  const dpSuggestion = useMemo(() => {
    const last = state.dpIterations.at(-1)
    if (!last) return null
    const m = meanOf(last.chrono)
    if (m == null) return null
    const diff = targetFps - m
    // Rough coefficient: ~4 FPS per 1 ms of DP near target (airsoft rule of thumb).
    const deltaMs = diff / 4
    return { diff, deltaMs, mean: m }
  }, [state.dpIterations, targetFps])

  // Matrix build
  const masses = useMemo(() => {
    const arr = [...DEFAULT_MASSES]
    if (state.customBbMass && !arr.includes(state.customBbMass)) arr.push(state.customBbMass)
    return arr
  }, [state.customBbMass])

  const matrixCols: Array<{ key: string; label: string; psi: number | null; refFps: number | null; extrap?: boolean }> = useMemo(() => {
    const cols: Array<{ key: string; label: string; psi: number | null; refFps: number | null; extrap?: boolean }> = []
    cols.push({ key: 'p80', label: '80 psi', psi: 80, refFps: means.p80 })
    cols.push({ key: 'p100', label: '100 psi', psi: 100, refFps: means.p100 })
    cols.push({ key: 'p120', label: '120 psi', psi: 120, refFps: means.p120 })
    cols.push({ key: 'hop', label: `hopup ${isBR ? '(apertado)' : '(tight)'}`, psi: null, refFps: means.hop })
    const tgt = fpsAtPsi(means.p80, means.p100, means.p120, state.target.psi)
    cols.push({ key: 'target', label: `${isBR ? 'alvo' : 'target'} ${state.target.psi} psi`, psi: state.target.psi, refFps: tgt.fps, extrap: tgt.extrapolated })
    return cols
  }, [means.p80, means.p100, means.p120, means.hop, state.target.psi, isBR])

  // ─── Save ─────
  const buildDerived = () => ({
    engine: state.engine === 2 ? 2 : 1,
    bbMass: state.bbMass,
    targetPsi: state.target.psi,
    targetMode: state.target.mode,
    targetValue: state.target.value,
    dn: state.finalTimings.dn,
    dr: state.finalTimings.dr,
    dp: state.finalTimings.dp,
    db: state.finalTimings.db,
  }) as const

  const copyJson = async () => {
    const payload = { ...state, _derived: buildDerived(), _generatedAt: new Date().toISOString() }
    const text = JSON.stringify(payload, null, 2)
    try { await navigator.clipboard.writeText(text) } catch {
      const ta = document.createElement('textarea')
      ta.value = text; document.body.appendChild(ta); ta.select()
      try { document.execCommand('copy') } catch {}
      ta.remove()
    }
    setCopied(true)
    setTimeout(() => setCopied(false), 1400)
  }

  const downloadPdf = async () => {
    const { jsPDF } = await import('jspdf')
    const doc = new jsPDF({ unit: 'pt', format: 'a4' })
    const L = (k: string) => t(k)
    let y = 48

    doc.setFont('helvetica', 'bold'); doc.setFontSize(16)
    doc.text(L('title'), 48, y); y += 22
    doc.setFont('helvetica', 'normal'); doc.setFontSize(9); doc.setTextColor(120)
    doc.text(L('subtitle'), 48, y); y += 20
    doc.setTextColor(0); doc.setFontSize(11)

    const line = (label: string, val: string) => {
      doc.setFont('helvetica', 'bold'); doc.text(label, 48, y)
      doc.setFont('helvetica', 'normal'); doc.text(val, 180, y)
      y += 16
    }

    line(isBR ? 'Engine' : 'Engine', isD8 ? 'D8PA (dual-solenoid)' : 'S8PA (single-solenoid)')
    line(isBR ? 'Massa BB' : 'BB mass', `${state.bbMass.toFixed(2)} g`)
    line(isBR ? 'Alvo' : 'Target', `${state.target.psi} psi · ${state.target.mode === 'fps' ? `${state.target.value} FPS` : `${state.target.value} J`}`)
    y += 6
    doc.setFont('helvetica', 'bold'); doc.text(isBR ? 'Timings finais' : 'Final timings', 48, y); y += 14
    doc.setFont('helvetica', 'normal')
    if (isD8) line('DN', `${state.finalTimings.dn ?? '-'} ms`)
    line('DR', `${state.finalTimings.dr ?? '-'} ms`)
    line('DP', `${state.finalTimings.dp ?? '-'} ms`)
    if (isD8) line('DB', `${state.finalTimings.db ?? '-'} units`)

    y += 10
    doc.setFont('helvetica', 'bold'); doc.setFontSize(12)
    doc.text(L('s14.title'), 48, y); y += 16
    doc.setFontSize(9); doc.setFont('helvetica', 'normal')

    // Table headers
    const colX = [48, 140, 220, 300, 380, 470]
    doc.setFont('helvetica', 'bold')
    doc.text(L('s14.bbCol'), colX[0], y)
    matrixCols.forEach((c, i) => doc.text(c.label, colX[i + 1], y))
    y += 12
    doc.setFont('helvetica', 'normal')

    masses.forEach((m) => {
      doc.text(`${m.toFixed(2)} g`, colX[0], y)
      matrixCols.forEach((c, i) => {
        let txt = '-'
        if (c.refFps != null) {
          const fps = fpsForMass(c.refFps, state.bbMass, m)
          const j = energyJ(fps, m)
          txt = `${fps.toFixed(0)} / ${j.toFixed(2)}J`
        }
        doc.text(txt, colX[i + 1], y)
      })
      y += 12
    })

    y += 14
    doc.setFont('helvetica', 'bold'); doc.text(L('s14.safety'), 48, y); y += 12
    doc.setFont('helvetica', 'normal'); doc.setFontSize(8)
    L('s14.safetyList').split('\n').forEach((ln) => { doc.text(ln, 48, y); y += 11 })
    y += 6
    doc.setFont('helvetica', 'bold'); doc.setFontSize(9); doc.text(L('s14.cross'), 48, y); y += 12
    doc.setFont('helvetica', 'normal'); doc.setFontSize(8)
    L('s14.crossList').split('\n').forEach((ln) => { doc.text(ln, 48, y); y += 11 })

    doc.save(`bcga-fcu-config-${Date.now()}.pdf`)
  }

  const stepLabels = [
    t('step.engine'),
    t('step.boot'),
    t('step.bb'),
    t('step.p80'),
    t('step.p100'),
    t('step.p120'),
    t('step.hop'),
    t('step.target'),
    t('step.dp'),
    t('step.dn'),
    t('step.dr'),
    t('step.dpFinal'),
    t('step.db'),
    t('step.results'),
  ]

  return (
    <Layout>
      <SEO
        title={isBR ? 'BCGA FCU — Ajuda de Configuração | BCGA Airsoft' : 'BCGA FCU — Config Helper | BCGA Airsoft'}
        description={isBR
          ? 'Ferramenta open source para calibrar FCUs HPA (BCGA, PolarStar, Wolverine e outras): leituras de chrono, matriz FPS × Joule, export PDF.'
          : 'Open-source helper to calibrate HPA FCUs (BCGA, PolarStar, Wolverine and others): chrono sweep, FPS × Joule matrix, PDF export.'}
      />

      <div className="space-y-6 pb-10">
        {/* Header */}
        <section className="px-4 pt-2">
          <div className="max-w-4xl mx-auto">
            <Link to="/fcu" className="inline-flex items-center gap-1 text-sm text-zinc-400 hover:text-copper-400 transition-colors">
              <svg className="w-4 h-4" viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth={2} aria-hidden>
                <path strokeLinecap="round" strokeLinejoin="round" d="M15 19l-7-7 7-7" />
              </svg>
              {t('backFcu')}
            </Link>
            <div className="mt-3 flex flex-col md:flex-row md:items-end md:justify-between gap-3">
              <div>
                <h1 className="text-2xl md:text-3xl font-bold text-zinc-100">{t('title')}</h1>
                <p className="text-zinc-400 text-sm mt-1 max-w-2xl">{t('subtitle')}</p>
              </div>
              <a
                href="https://github.com/BCGAdesigns/BCGA_FCU_STR"
                target="_blank" rel="noopener"
                className="inline-flex items-center gap-2 text-xs px-3 py-2 rounded-lg border border-zinc-700 text-zinc-300 hover:border-copper-500 hover:text-copper-300 transition-colors self-start md:self-auto"
              >
                <svg className="w-4 h-4" viewBox="0 0 24 24" fill="currentColor" aria-hidden>
                  <path d="M12 .5C5.7.5.5 5.7.5 12c0 5.1 3.3 9.4 7.9 10.9.6.1.8-.3.8-.6v-2c-3.2.7-3.9-1.5-3.9-1.5-.5-1.3-1.3-1.7-1.3-1.7-1.1-.7.1-.7.1-.7 1.2.1 1.8 1.2 1.8 1.2 1 1.8 2.7 1.3 3.4 1 .1-.8.4-1.3.8-1.6-2.6-.3-5.3-1.3-5.3-5.8 0-1.3.4-2.3 1.2-3.2-.1-.3-.5-1.5.1-3.1 0 0 1-.3 3.2 1.2.9-.3 1.9-.4 3-.4s2.1.1 3 .4c2.2-1.5 3.2-1.2 3.2-1.2.6 1.6.2 2.8.1 3.1.7.9 1.2 1.9 1.2 3.2 0 4.5-2.7 5.5-5.3 5.8.4.4.8 1 .8 2.1v3.1c0 .3.2.7.8.6 4.6-1.5 7.9-5.8 7.9-10.9C23.5 5.7 18.3.5 12 .5Z"/>
                </svg>
                {t('github')}
              </a>
            </div>
          </div>
        </section>

        {/* Wizard card */}
        <section className="px-4">
          <div className="max-w-4xl mx-auto bg-zinc-900/60 border border-zinc-800 rounded-2xl p-5 md:p-8 shadow-xl">
            {/* Progress */}
            <nav aria-label="Progress" className="mb-6">
              <ol className="flex items-center gap-1 overflow-x-auto">
                {stepLabels.map((label, idx) => {
                  const n = idx + 1
                  const current = n === state.step
                  const done = n < state.step
                  return (
                    <li key={n} className="flex-1 min-w-[72px]">
                      <button
                        type="button"
                        onClick={() => goStep(n)}
                        className={[
                          'w-full flex flex-col items-center gap-1 px-1 py-2 rounded-lg border transition-colors text-[10px] md:text-[11px]',
                          current
                            ? 'border-copper-500/60 bg-copper-500/10 text-copper-300'
                            : done
                              ? 'border-emerald-600/40 bg-emerald-600/5 text-emerald-300 hover:border-emerald-500/60'
                              : 'border-zinc-800 bg-zinc-900/40 text-zinc-500 hover:border-zinc-700',
                        ].join(' ')}
                      >
                        <span className={[
                          'inline-flex items-center justify-center w-6 h-6 rounded-full text-[11px] font-semibold',
                          current ? 'bg-copper-500 text-white' : done ? 'bg-emerald-600 text-white' : 'bg-zinc-800 text-zinc-400',
                        ].join(' ')}>{n}</span>
                        <span className="truncate w-full text-center">{label}</span>
                      </button>
                    </li>
                  )
                })}
              </ol>
            </nav>

            {/* STEP 1 Engine */}
            {state.step === 1 && (
              <section>
                <h2 className="text-xl md:text-2xl font-bold text-zinc-100">{t('s1.title')}</h2>
                <p className="text-zinc-400 text-sm mt-2">{t('s1.lead')}</p>
                <div className="grid grid-cols-1 md:grid-cols-2 gap-3 mt-5">
                  {[
                    { id: 1 as const, head: 'S8PA', sub: t('s1.s8.sub'), body: t('s1.s8.body') },
                    { id: 2 as const, head: 'D8PA', sub: t('s1.d8.sub'), body: t('s1.d8.body') },
                  ].map((opt) => {
                    const active = state.engine === opt.id
                    return (
                      <button
                        key={opt.id}
                        type="button"
                        onClick={() => pickEngine(opt.id)}
                        className={[
                          'text-left p-5 rounded-xl border transition-all',
                          active
                            ? 'border-copper-500 bg-copper-500/10 ring-1 ring-copper-500/30'
                            : 'border-zinc-800 bg-zinc-900/60 hover:border-copper-500/60 hover:bg-zinc-900',
                        ].join(' ')}
                      >
                        <div className="text-lg font-bold text-zinc-100">{opt.head}</div>
                        <div className="text-xs uppercase tracking-wider text-copper-400 mt-0.5">{opt.sub}</div>
                        <div className="text-sm text-zinc-400 mt-2">{opt.body}</div>
                      </button>
                    )
                  })}
                </div>
                <p className="text-xs text-zinc-500 mt-5">{t('s1.crossNote')}</p>
              </section>
            )}

            {/* STEP 2 First boot */}
            {state.step === 2 && (
              <section>
                <h2 className="text-xl md:text-2xl font-bold text-zinc-100">{t('s2.title')}</h2>
                <p className="text-zinc-400 text-sm mt-2">{t('s2.lead')}</p>
                <div className="grid grid-cols-1 md:grid-cols-2 gap-3 mt-5">
                  <button
                    type="button"
                    onClick={() => setFirstBoot(true)}
                    className={[
                      'text-left p-5 rounded-xl border transition-all',
                      state.firstBoot === true
                        ? 'border-emerald-500 bg-emerald-500/10 ring-1 ring-emerald-500/30'
                        : 'border-zinc-800 bg-zinc-900/60 hover:border-emerald-500/60',
                    ].join(' ')}
                  >
                    <div className="text-zinc-100 font-semibold">{t('s2.yes')}</div>
                    <div className="text-sm text-zinc-400 mt-1">{t('s2.yesSub')}</div>
                  </button>
                  <button
                    type="button"
                    onClick={() => setFirstBoot(false)}
                    className={[
                      'text-left p-5 rounded-xl border transition-all',
                      state.firstBoot === false
                        ? 'border-copper-500 bg-copper-500/10 ring-1 ring-copper-500/30'
                        : 'border-zinc-800 bg-zinc-900/60 hover:border-copper-500/60',
                    ].join(' ')}
                  >
                    <div className="text-zinc-100 font-semibold">{t('s2.no')}</div>
                    <div className="text-sm text-zinc-400 mt-1">{t('s2.noSub')}</div>
                  </button>
                </div>

                {state.firstBoot === false && (
                  <div className="mt-5 bg-zinc-950/60 border border-zinc-800 rounded-xl p-4">
                    <h3 className="text-zinc-100 font-semibold">{t('s2.currentTitle')}</h3>
                    <div className="mt-3 grid grid-cols-2 md:grid-cols-4 gap-3">
                      {isD8 && <NumberInput label={t('s2.dn')} value={state.currentTimings.dn} onChange={(v) => updateCurrentTiming('dn', v)} />}
                      <NumberInput label={t('s2.dr')} value={state.currentTimings.dr} onChange={(v) => updateCurrentTiming('dr', v)} />
                      <NumberInput label={t('s2.dp')} value={state.currentTimings.dp} onChange={(v) => updateCurrentTiming('dp', v)} />
                      {isD8 && <NumberInput label={t('s2.db')} value={state.currentTimings.db} onChange={(v) => updateCurrentTiming('db', v)} step={10} />}
                    </div>
                  </div>
                )}

                <NavFooter onBack={() => goStep(1)} onNext={() => goStep(3)} tBack={t('nav.back')} tNext={t('nav.next')} nextDisabled={state.firstBoot === null} />
              </section>
            )}

            {/* STEP 3 BB mass */}
            {state.step === 3 && (
              <section>
                <h2 className="text-xl md:text-2xl font-bold text-zinc-100">{t('s3.title')}</h2>
                <p className="text-zinc-400 text-sm mt-2">{t('s3.lead')}</p>
                <div className="mt-5 max-w-xs">
                  <NumberInput
                    label={t('s3.label')}
                    value={state.bbMass}
                    onChange={(v) => setBbMass(v ?? 0.28)}
                    step={0.01}
                    min={0.12} max={0.5}
                  />
                </div>
                <NavFooter onBack={() => goStep(2)} onNext={() => goStep(4)} tBack={t('nav.back')} tNext={t('nav.next')} />
              </section>
            )}

            {/* STEPS 4, 5, 6 chrono sweeps */}
            {(state.step === 4 || state.step === 5 || state.step === 6) && (() => {
              const psiKey = state.step === 4 ? 'psi80' : state.step === 5 ? 'psi100' : 'psi120'
              const psiNum = state.step === 4 ? 80 : state.step === 5 ? 100 : 120
              const titleKey = state.step === 4 ? 's4.title' : state.step === 5 ? 's5.title' : 's6.title'
              const chrono = state.sweep[psiKey]
              const mean = meanOf(chrono)
              const spread = spreadOf(chrono)
              return (
                <section>
                  <h2 className="text-xl md:text-2xl font-bold text-zinc-100">{t(titleKey)}</h2>
                  <p className="text-zinc-400 text-sm mt-2">{t('s45.lead')}</p>
                  {state.step === 6 && <p className="text-amber-300 text-xs mt-2">{t('s6.warn')}</p>}
                  <div className="mt-5 grid grid-cols-3 gap-3">
                    {(['s1', 's2', 's3'] as const).map((k, i) => (
                      <NumberInput
                        key={k}
                        label={`${t('shot')} ${i + 1}`}
                        value={chrono[k]}
                        onChange={(v) => updateSweep(psiKey, k, v)}
                        min={0} step={1} suffix="FPS"
                      />
                    ))}
                  </div>
                  {mean != null && (
                    <div className="mt-4 grid grid-cols-2 gap-3 text-sm">
                      <Stat label={t('mean')} value={`${mean.toFixed(1)} FPS · ${energyJ(mean, state.bbMass).toFixed(2)} J`} />
                      {spread != null && (
                        <Stat
                          label={t('spread')}
                          value={`${spread.toFixed(0)} FPS`}
                          warn={spread > 15 ? t('spreadWarn') : undefined}
                        />
                      )}
                    </div>
                  )}
                  <NavFooter
                    onBack={() => goStep(state.step - 1)}
                    onNext={() => goStep(state.step + 1)}
                    tBack={t('nav.back')} tNext={t('nav.next')}
                    nextDisabled={!isChronoComplete(chrono)}
                  />
                  <div className="text-[11px] text-zinc-500 mt-3">psi = {psiNum}</div>
                </section>
              )
            })()}

            {/* STEP 7 Hopup */}
            {state.step === 7 && (
              <section>
                <h2 className="text-xl md:text-2xl font-bold text-zinc-100">{t('s7.title')}</h2>
                <p className="text-zinc-400 text-sm mt-2">{t('s7.lead')}</p>
                <div className="mt-5 grid grid-cols-3 gap-3">
                  {(['s1', 's2', 's3'] as const).map((k, i) => (
                    <NumberInput
                      key={k}
                      label={`${t('shot')} ${i + 1}`}
                      value={state.sweep.hopup[k]}
                      onChange={(v) => updateSweep('hopup', k, v)}
                      min={0} step={1} suffix="FPS"
                    />
                  ))}
                </div>
                {means.hop != null && (
                  <div className="mt-4 grid grid-cols-2 gap-3 text-sm">
                    <Stat label={t('mean')} value={`${means.hop.toFixed(1)} FPS`} />
                    {means.p120 != null && (
                      <Stat
                        label={t('hopGain')}
                        value={`${(means.hop - means.p120).toFixed(1)} FPS`}
                      />
                    )}
                  </div>
                )}
                <NavFooter
                  onBack={() => goStep(6)}
                  onNext={() => goStep(8)}
                  tBack={t('nav.back')} tNext={t('nav.next')}
                  nextDisabled={!isChronoComplete(state.sweep.hopup)}
                />
              </section>
            )}

            {/* STEP 8 Target */}
            {state.step === 8 && (
              <section>
                <h2 className="text-xl md:text-2xl font-bold text-zinc-100">{t('s8.title')}</h2>
                <p className="text-zinc-400 text-sm mt-2">{t('s8.lead')}</p>
                <div className="mt-5 grid grid-cols-1 md:grid-cols-3 gap-3">
                  <NumberInput
                    label={t('s8.psi')}
                    value={state.target.psi}
                    onChange={(v) => setTarget({ psi: v ?? 100 })}
                    min={60} max={140} step={5}
                  />
                  <label className="block">
                    <span className="text-zinc-300 text-sm">{t('s8.mode')}</span>
                    <div className="mt-1 flex gap-2">
                      {(['fps', 'joule'] as const).map((m) => (
                        <button
                          key={m}
                          type="button"
                          onClick={() => setTarget({ mode: m })}
                          className={[
                            'flex-1 px-3 py-2 rounded-lg border text-sm transition-colors',
                            state.target.mode === m
                              ? 'border-copper-500 bg-copper-500/10 text-copper-300'
                              : 'border-zinc-800 bg-zinc-950 text-zinc-400 hover:border-zinc-600',
                          ].join(' ')}
                        >
                          {m === 'fps' ? t('s8.modeFps') : t('s8.modeJ')}
                        </button>
                      ))}
                    </div>
                  </label>
                  <NumberInput
                    label={t('s8.value')}
                    value={state.target.value}
                    onChange={(v) => setTarget({ value: v ?? 0 })}
                    step={state.target.mode === 'fps' ? 5 : 0.01}
                    min={0}
                    suffix={state.target.mode === 'fps' ? 'FPS' : 'J'}
                  />
                </div>

                {/* Derived joule / fps + warnings */}
                {(() => {
                  const joule = state.target.mode === 'joule'
                    ? state.target.value
                    : energyJ(state.target.value, state.bbMass)
                  return (
                    <div className="mt-4 text-sm text-zinc-300 bg-zinc-950/60 border border-zinc-800 rounded-lg px-4 py-3">
                      {state.target.mode === 'fps'
                        ? (isBR ? 'Energia equivalente: ' : 'Equivalent energy: ')
                        : (isBR ? 'FPS equivalente: ' : 'Equivalent FPS: ')}
                      <strong className="text-copper-300 tabular-nums">
                        {state.target.mode === 'fps' ? `${joule.toFixed(2)} J` : `${targetFps.toFixed(0)} FPS`}
                      </strong>
                      {joule > 2.32 && <div className="text-amber-300 text-xs mt-1">{t('s8.fieldOut')}</div>}
                      {joule > 1.14 && joule <= 2.32 && <div className="text-amber-300 text-xs mt-1">{t('s8.fieldCqb')}</div>}
                    </div>
                  )
                })()}

                <NavFooter onBack={() => goStep(7)} onNext={() => goStep(9)} tBack={t('nav.back')} tNext={t('nav.next')} />
              </section>
            )}

            {/* STEP 9 DP iteration */}
            {state.step === 9 && (
              <DpStep
                state={state}
                t={t} isBR={isBR} isD8={isD8}
                targetFps={targetFps}
                suggestion={dpSuggestion}
                onAdd={addDpIteration}
                onClear={clearDpIterations}
                onSetFinalDp={(v) => setFinalTiming('dp', v)}
                onBack={() => goStep(8)}
                onNext={() => goStep(10)}
              />
            )}

            {/* STEP 10 DN deep-dive */}
            {state.step === 10 && (
              <TimingDeepDive
                title={t('s10.title')} lead={t('s10.lead')}
                what={t('s10.what')} symptoms={t('s10.symptoms')}
                rule={t('s10.rule')} brands={t('s10.brands')}
                labels={{ what: t('deep.what'), symptoms: t('deep.symptoms'), rule: t('deep.rule'), brands: t('deep.brands') }}
                notApplicable={!isD8}
                naNote={t('s10.na')}
                field={{ label: t('s10.field'), value: state.finalTimings.dn, onChange: (v) => setFinalTiming('dn', v), min: 5, max: 40, step: 1, suffix: 'ms' }}
                onBack={() => goStep(9)} onNext={() => goStep(11)}
                tBack={t('nav.back')} tNext={t('nav.next')}
              />
            )}

            {/* STEP 11 DR deep-dive */}
            {state.step === 11 && (
              <TimingDeepDive
                title={t('s11.title')} lead={t('s11.lead')}
                what={t('s11.what')} symptoms={t('s11.symptoms')}
                rule={t('s11.rule')} brands={t('s11.brands')}
                labels={{ what: t('deep.what'), symptoms: t('deep.symptoms'), rule: t('deep.rule'), brands: t('deep.brands') }}
                field={{ label: t('s11.field'), value: state.finalTimings.dr, onChange: (v) => setFinalTiming('dr', v), min: 5, max: 40, step: 1, suffix: 'ms' }}
                onBack={() => goStep(10)} onNext={() => goStep(12)}
                tBack={t('nav.back')} tNext={t('nav.next')}
              />
            )}

            {/* STEP 12 DP deep-dive (read-only, from step 9) */}
            {state.step === 12 && (
              <TimingDeepDive
                title={t('s12.title')} lead={t('s12.lead')}
                what={t('s12.what')} symptoms={t('s12.symptoms')}
                rule={t('s12.rule')} brands={t('s12.brands')}
                labels={{ what: t('deep.what'), symptoms: t('deep.symptoms'), rule: t('deep.rule'), brands: t('deep.brands') }}
                readOnly={{
                  label: t('s12.fromStep9'),
                  value: state.finalTimings.dp != null ? `${state.finalTimings.dp} ms` : '—',
                  action: { label: t('s12.goStep9'), onClick: () => goStep(9) },
                }}
                onBack={() => goStep(11)} onNext={() => goStep(13)}
                tBack={t('nav.back')} tNext={t('nav.next')}
              />
            )}

            {/* STEP 13 DB deep-dive */}
            {state.step === 13 && (
              <TimingDeepDive
                title={t('s13.title')} lead={t('s13.lead')}
                what={t('s13.what')} symptoms={t('s13.symptoms')}
                rule={t('s13.rule')} brands={t('s13.brands')}
                labels={{ what: t('deep.what'), symptoms: t('deep.symptoms'), rule: t('deep.rule'), brands: t('deep.brands') }}
                notApplicable={!isD8}
                naNote={t('s13.na')}
                field={{ label: t('s13.field'), value: state.finalTimings.db, onChange: (v) => setFinalTiming('db', v), min: 0, max: 500, step: 5, suffix: 'units' }}
                onBack={() => goStep(12)} onNext={() => goStep(14)}
                tBack={t('nav.back')} tNext={t('nav.next')}
              />
            )}

            {/* STEP 14 Results */}
            {state.step === 14 && (
              <section>
                <h2 className="text-xl md:text-2xl font-bold text-zinc-100">{t('s14.title')}</h2>
                <p className="text-zinc-400 text-sm mt-2">{t('s14.lead')}</p>

                {/* Target summary */}
                <div className="mt-4 bg-zinc-950/60 border border-zinc-800 rounded-xl p-4 text-sm grid grid-cols-2 md:grid-cols-4 gap-3">
                  <Kv k="Engine" v={isD8 ? 'D8PA' : 'S8PA'} />
                  <Kv k={isBR ? 'Massa BB' : 'BB mass'} v={`${state.bbMass.toFixed(2)} g`} />
                  <Kv k={isBR ? 'Alvo' : 'Target'} v={`${state.target.psi} psi · ${state.target.value} ${state.target.mode === 'fps' ? 'FPS' : 'J'}`} />
                  <Kv k="Timings" v={`${isD8 ? `DN ${state.finalTimings.dn ?? '-'} · ` : ''}DR ${state.finalTimings.dr ?? '-'} · DP ${state.finalTimings.dp ?? '-'}${isD8 ? ` · DB ${state.finalTimings.db ?? '-'}` : ''}`} />
                </div>

                {/* Matrix */}
                <div className="mt-5 overflow-x-auto">
                  <table className="w-full text-sm border border-zinc-800 rounded-lg overflow-hidden">
                    <thead className="bg-zinc-900/80 text-zinc-300 text-xs uppercase tracking-wider">
                      <tr>
                        <th className="text-left px-3 py-2 border-b border-zinc-800">{t('s14.bbCol')}</th>
                        {matrixCols.map((c) => (
                          <th key={c.key} className="text-left px-3 py-2 border-b border-zinc-800">
                            {c.label}
                            {c.extrap && <span className="block text-amber-300 text-[10px] normal-case tracking-normal">⚠ extrap.</span>}
                          </th>
                        ))}
                      </tr>
                    </thead>
                    <tbody className="font-mono tabular-nums">
                      {masses.map((m) => (
                        <tr key={m} className="odd:bg-zinc-900/30">
                          <td className="px-3 py-1.5 text-zinc-200">{m.toFixed(2)} g</td>
                          {matrixCols.map((c) => {
                            if (c.refFps == null) return <td key={c.key} className="px-3 py-1.5 text-zinc-600">—</td>
                            const fps = fpsForMass(c.refFps, state.bbMass, m)
                            const j = energyJ(fps, m)
                            return (
                              <td key={c.key} className="px-3 py-1.5">
                                <span className="text-zinc-100">{fps.toFixed(0)}</span>
                                <span className="text-zinc-500"> / </span>
                                <span className="text-copper-300">{j.toFixed(2)}J</span>
                              </td>
                            )
                          })}
                        </tr>
                      ))}
                    </tbody>
                  </table>
                </div>

                {/* Custom mass input */}
                <div className="mt-3 max-w-xs">
                  <NumberInput
                    label={isBR ? 'Massa custom (g)' : 'Custom mass (g)'}
                    value={state.customBbMass}
                    onChange={(v) => setState((s) => ({ ...s, customBbMass: v }))}
                    min={0.12} max={0.5} step={0.01}
                  />
                </div>

                {/* Safety + Cross-brand */}
                <div className="mt-5 grid grid-cols-1 md:grid-cols-2 gap-3 text-xs text-zinc-400">
                  <div className="bg-zinc-950/40 border border-zinc-800 rounded-lg p-3">
                    <div className="text-zinc-200 font-semibold mb-1">{t('s14.safety')}</div>
                    <pre className="whitespace-pre-wrap font-sans text-zinc-400">{t('s14.safetyList')}</pre>
                  </div>
                  <div className="bg-zinc-950/40 border border-zinc-800 rounded-lg p-3">
                    <div className="text-zinc-200 font-semibold mb-1">{t('s14.cross')}</div>
                    <pre className="whitespace-pre-wrap font-sans text-zinc-400">{t('s14.crossList')}</pre>
                  </div>
                </div>

                {/* Actions */}
                <div className="mt-6 flex flex-col sm:flex-row flex-wrap gap-2 justify-end">
                  <button type="button" onClick={restart}
                    className="px-4 py-2.5 rounded-xl border border-zinc-700 text-zinc-300 hover:border-zinc-500 hover:text-zinc-100 text-sm">
                    {t('s14.restart')}
                  </button>
                  <button type="button" onClick={copyJson}
                    className="px-4 py-2.5 rounded-xl border border-zinc-700 text-zinc-300 hover:border-copper-500 hover:text-copper-300 text-sm">
                    {copied ? t('s14.copied') : t('s14.copy')}
                  </button>
                  <button type="button" onClick={downloadPdf}
                    className="px-4 py-2.5 rounded-xl border border-copper-500/60 text-copper-300 hover:bg-copper-500/10 text-sm">
                    {t('s14.pdf')}
                  </button>
                  {user ? (
                    <button type="button" onClick={() => setShowSaveModal(true)}
                      className="bg-copper-500 hover:bg-copper-400 text-white font-semibold py-2.5 px-5 rounded-xl transition-colors text-sm">
                      {t('s14.save')}
                    </button>
                  ) : (
                    <Link to="/entrar"
                      className="bg-copper-500 hover:bg-copper-400 text-white font-semibold py-2.5 px-5 rounded-xl transition-colors text-sm">
                      {t('s14.loginNote')}
                    </Link>
                  )}
                </div>
              </section>
            )}
          </div>
        </section>
      </div>

      {showSaveModal && (
        <SaveModal
          t={t}
          onClose={() => setShowSaveModal(false)}
          onSave={async (name) => {
            await saveFcuConfig({ name, stateJson: state, derived: buildDerived() })
          }}
        />
      )}
    </Layout>
  )
}

// ─── Sub-components ──────────────────────────────────────
function NumberInput({ label, value, onChange, min, max, step, suffix }: {
  label: string
  value: number | null
  onChange: (v: number | null) => void
  min?: number
  max?: number
  step?: number
  suffix?: string
}) {
  return (
    <label className="block">
      <span className="text-zinc-300 text-xs uppercase tracking-wider">{label}</span>
      <div className="mt-1 flex items-center gap-2">
        <input
          type="number"
          value={value ?? ''}
          min={min} max={max} step={step ?? 1}
          inputMode="decimal"
          onChange={(e) => {
            const raw = e.target.value
            if (raw === '') onChange(null)
            else {
              const n = Number(raw)
              onChange(Number.isFinite(n) ? n : null)
            }
          }}
          className="w-full bg-zinc-950 border border-zinc-800 rounded-lg px-3 py-2 text-zinc-100 focus:outline-none focus:border-copper-500"
        />
        {suffix && <span className="text-zinc-500 text-xs uppercase tracking-wider">{suffix}</span>}
      </div>
    </label>
  )
}

function Stat({ label, value, warn }: { label: string; value: string; warn?: string }) {
  return (
    <div className="bg-zinc-950/60 border border-zinc-800 rounded-lg px-3 py-2">
      <div className="text-zinc-500 text-xs uppercase tracking-wider">{label}</div>
      <div className="text-zinc-100 font-mono tabular-nums">{value}</div>
      {warn && <div className="text-amber-300 text-xs mt-1">{warn}</div>}
    </div>
  )
}

function NavFooter({ onBack, onNext, tBack, tNext, nextDisabled }: {
  onBack: () => void
  onNext?: () => void
  tBack: string
  tNext: string
  nextDisabled?: boolean
}) {
  return (
    <div className="mt-6 flex items-center justify-between gap-3">
      <button type="button" onClick={onBack} className="text-zinc-400 hover:text-zinc-200 text-sm">
        {tBack}
      </button>
      {onNext && (
        <button
          type="button" onClick={onNext} disabled={nextDisabled}
          className="bg-copper-500 hover:bg-copper-400 disabled:opacity-50 disabled:cursor-not-allowed text-white font-semibold py-2.5 px-5 rounded-xl transition-colors"
        >
          {tNext}
        </button>
      )}
    </div>
  )
}

function Kv({ k, v }: { k: string; v: string }) {
  return (
    <div>
      <div className="text-zinc-500 text-[11px] uppercase tracking-wider">{k}</div>
      <div className="text-zinc-100 font-mono tabular-nums text-sm">{v}</div>
    </div>
  )
}

// ─── Timing deep-dive step (used for DN / DR / DP / DB) ───
function TimingDeepDive({
  title, lead, what, symptoms, rule, brands, labels,
  notApplicable, naNote, field, readOnly, onBack, onNext, tBack, tNext,
}: {
  title: string
  lead: string
  what: string
  symptoms: string
  rule: string
  brands: string
  labels: { what: string; symptoms: string; rule: string; brands: string }
  notApplicable?: boolean
  naNote?: string
  field?: {
    label: string
    value: number | null
    onChange: (v: number | null) => void
    min?: number; max?: number; step?: number; suffix?: string
  }
  readOnly?: {
    label: string
    value: string
    action?: { label: string; onClick: () => void }
  }
  onBack: () => void
  onNext: () => void
  tBack: string
  tNext: string
}) {
  return (
    <section>
      <h2 className="text-xl md:text-2xl font-bold text-zinc-100">{title}</h2>
      <p className="text-zinc-400 text-sm mt-2">{lead}</p>

      {notApplicable ? (
        <div className="mt-5 bg-amber-500/10 border border-amber-500/40 rounded-xl p-4 text-sm text-amber-200">
          {naNote}
        </div>
      ) : (
        <div className="mt-5 space-y-4 text-sm">
          <div className="bg-zinc-950/40 border border-zinc-800 rounded-xl p-4">
            <div className="text-zinc-100 font-semibold text-xs uppercase tracking-wider mb-2">{labels.what}</div>
            <pre className="whitespace-pre-wrap font-sans text-zinc-300 leading-relaxed">{what}</pre>
          </div>
          <div className="bg-zinc-950/40 border border-zinc-800 rounded-xl p-4">
            <div className="text-zinc-100 font-semibold text-xs uppercase tracking-wider mb-2">{labels.symptoms}</div>
            <pre className="whitespace-pre-wrap font-sans text-zinc-300 leading-relaxed">{symptoms}</pre>
          </div>
          <div className="bg-zinc-950/40 border border-copper-500/40 rounded-xl p-4">
            <div className="text-copper-300 font-semibold text-xs uppercase tracking-wider mb-2">{labels.rule}</div>
            <pre className="whitespace-pre-wrap font-sans text-zinc-200 leading-relaxed">{rule}</pre>
          </div>
          <div className="bg-zinc-950/40 border border-zinc-800 rounded-xl p-4">
            <div className="text-zinc-100 font-semibold text-xs uppercase tracking-wider mb-2">{labels.brands}</div>
            <pre className="whitespace-pre-wrap font-sans text-zinc-400 leading-relaxed">{brands}</pre>
          </div>

          {field && (
            <div className="mt-2 max-w-sm">
              <NumberInput
                label={field.label}
                value={field.value}
                onChange={field.onChange}
                min={field.min} max={field.max}
                step={field.step ?? 1}
                suffix={field.suffix}
              />
            </div>
          )}
          {readOnly && (
            <div className="bg-zinc-950/60 border border-zinc-800 rounded-xl p-4 flex flex-col sm:flex-row sm:items-center gap-3">
              <div className="flex-1">
                <div className="text-zinc-300 text-sm font-semibold">{readOnly.label}</div>
                <div className="text-copper-300 font-mono tabular-nums text-2xl mt-1">{readOnly.value}</div>
              </div>
              {readOnly.action && (
                <button type="button" onClick={readOnly.action.onClick}
                  className="text-xs px-3 py-2 rounded-lg border border-zinc-700 text-zinc-300 hover:border-copper-500 hover:text-copper-300 transition-colors whitespace-nowrap">
                  {readOnly.action.label}
                </button>
              )}
            </div>
          )}
        </div>
      )}

      <NavFooter onBack={onBack} onNext={onNext} tBack={tBack} tNext={tNext} />
    </section>
  )
}

// ─── DP iterative step ───────────────────────────────────
function DpStep({
  state, t, isBR, isD8, targetFps, suggestion, onAdd, onClear, onSetFinalDp, onBack, onNext,
}: {
  state: WizardState
  t: (k: string) => string
  isBR: boolean
  isD8: boolean
  targetFps: number
  suggestion: { diff: number; deltaMs: number; mean: number } | null
  onAdd: (dp: number, chrono: ChronoReading) => void
  onClear: () => void
  onSetFinalDp: (v: number | null) => void
  onBack: () => void
  onNext: () => void
}) {
  const seedDp = state.finalTimings.dp ?? state.currentTimings.dp ?? DEFAULT_FIRST_BOOT.dp!
  const [dp, setDp] = useState<number | null>(seedDp)
  const [chrono, setChrono] = useState<ChronoReading>({ ...EMPTY_CHRONO })

  const canAdd = dp != null && isChronoComplete(chrono)

  const handleAdd = () => {
    if (!canAdd || dp == null) return
    onAdd(dp, chrono)
    onSetFinalDp(dp)
    setChrono({ ...EMPTY_CHRONO })
  }

  return (
    <section>
      <h2 className="text-xl md:text-2xl font-bold text-zinc-100">{t('s9.title')}</h2>
      <p className="text-zinc-400 text-sm mt-2">{t('s9.lead')}</p>

      <div className="mt-4 bg-zinc-950/60 border border-zinc-800 rounded-xl p-4">
        <div className="grid grid-cols-1 md:grid-cols-5 gap-3">
          <NumberInput label={t('s9.dpField')} value={dp} onChange={setDp} min={5} max={120} step={1} suffix="ms" />
          <NumberInput label={`${t('shot')} 1`} value={chrono.s1} onChange={(v) => setChrono((c) => ({ ...c, s1: v }))} min={0} step={1} suffix="FPS" />
          <NumberInput label={`${t('shot')} 2`} value={chrono.s2} onChange={(v) => setChrono((c) => ({ ...c, s2: v }))} min={0} step={1} suffix="FPS" />
          <NumberInput label={`${t('shot')} 3`} value={chrono.s3} onChange={(v) => setChrono((c) => ({ ...c, s3: v }))} min={0} step={1} suffix="FPS" />
          <div className="flex items-end">
            <button
              type="button" onClick={handleAdd} disabled={!canAdd}
              className="w-full bg-copper-500 hover:bg-copper-400 disabled:opacity-40 text-white font-semibold py-2.5 rounded-xl transition-colors text-sm"
            >
              {t('s9.add')}
            </button>
          </div>
        </div>
        {isD8 && dp != null && dp < 20 && (
          <div className="text-amber-300 text-xs mt-2">{t('s9.tooShort')}</div>
        )}
      </div>

      {/* Suggestion */}
      <div className="mt-4 bg-zinc-950/40 border border-zinc-800 rounded-xl p-4 text-sm">
        <div className="text-zinc-300 font-semibold">{t('s9.suggest')}</div>
        {suggestion ? (
          <div className="mt-2 text-zinc-300">
            {isBR ? 'Última média: ' : 'Last mean: '}
            <strong className="text-copper-300 tabular-nums">{suggestion.mean.toFixed(1)} FPS</strong>
            {' · '}
            {isBR ? 'alvo: ' : 'target: '}
            <strong className="text-copper-300 tabular-nums">{targetFps.toFixed(0)} FPS</strong>
            {' → '}
            {Math.abs(suggestion.diff) < 3
              ? (isBR ? <span className="text-emerald-300">estabilizou ✓</span> : <span className="text-emerald-300">stable ✓</span>)
              : suggestion.diff > 0
                ? (isBR
                    ? <span>↑ aumente DP em ~<strong className="text-copper-300">{suggestion.deltaMs.toFixed(1)} ms</strong></span>
                    : <span>↑ raise DP by ~<strong className="text-copper-300">{suggestion.deltaMs.toFixed(1)} ms</strong></span>)
                : (isBR
                    ? <span>↓ reduza DP em ~<strong className="text-copper-300">{Math.abs(suggestion.deltaMs).toFixed(1)} ms</strong></span>
                    : <span>↓ lower DP by ~<strong className="text-copper-300">{Math.abs(suggestion.deltaMs).toFixed(1)} ms</strong></span>)}
          </div>
        ) : (
          <div className="mt-2 text-zinc-500">{t('s9.noIter')}</div>
        )}
      </div>

      {/* Iteration log */}
      {state.dpIterations.length > 0 && (
        <div className="mt-4">
          <div className="flex items-center justify-between mb-2">
            <div className="text-zinc-300 text-sm font-semibold">{t('s9.iter')}s</div>
            <button type="button" onClick={onClear} className="text-xs text-zinc-500 hover:text-zinc-300">
              {isBR ? 'Limpar' : 'Clear'}
            </button>
          </div>
          <div className="overflow-x-auto">
            <table className="w-full text-sm border border-zinc-800 rounded-lg overflow-hidden">
              <thead className="bg-zinc-900/80 text-zinc-400 text-xs uppercase tracking-wider">
                <tr>
                  <th className="text-left px-3 py-2">#</th>
                  <th className="text-left px-3 py-2">DP</th>
                  <th className="text-left px-3 py-2">{t('shot')} 1/2/3</th>
                  <th className="text-left px-3 py-2">{t('mean')}</th>
                  <th className="text-left px-3 py-2">{t('spread')}</th>
                </tr>
              </thead>
              <tbody className="font-mono tabular-nums">
                {state.dpIterations.map((it, idx) => (
                  <tr key={idx} className="odd:bg-zinc-900/30">
                    <td className="px-3 py-1.5 text-zinc-500">{idx + 1}</td>
                    <td className="px-3 py-1.5 text-copper-300">{it.dp} ms</td>
                    <td className="px-3 py-1.5 text-zinc-300">{it.chrono.s1}/{it.chrono.s2}/{it.chrono.s3}</td>
                    <td className="px-3 py-1.5 text-zinc-100">{(meanOf(it.chrono) ?? 0).toFixed(1)}</td>
                    <td className="px-3 py-1.5 text-zinc-400">{(spreadOf(it.chrono) ?? 0).toFixed(0)}</td>
                  </tr>
                ))}
              </tbody>
            </table>
          </div>
        </div>
      )}

      <div className="mt-6 flex items-center justify-between gap-3">
        <button type="button" onClick={onBack} className="text-zinc-400 hover:text-zinc-200 text-sm">
          {t('nav.back')}
        </button>
        <button
          type="button" onClick={onNext}
          disabled={state.finalTimings.dp == null}
          className="bg-copper-500 hover:bg-copper-400 disabled:opacity-40 text-white font-semibold py-2.5 px-5 rounded-xl transition-colors"
        >
          {t('s9.stop')}
        </button>
      </div>
    </section>
  )
}

// ─── Save modal ──────────────────────────────────────────
function SaveModal({
  t, onClose, onSave,
}: {
  t: (k: string) => string
  onClose: () => void
  onSave: (name: string) => Promise<void>
}) {
  const [name, setName] = useState('')
  const [saving, setSaving] = useState(false)
  const [saved, setSaved] = useState(false)
  const [err, setErr] = useState<string | null>(null)

  const handle = async () => {
    const trimmed = name.trim()
    if (!trimmed) return
    setSaving(true); setErr(null)
    try {
      await onSave(trimmed)
      setSaved(true)
      setTimeout(onClose, 900)
    } catch (e: any) {
      setErr(e?.message || 'Error')
    } finally {
      setSaving(false)
    }
  }

  return (
    <div className="fixed inset-0 bg-black/60 flex items-center justify-center z-50 p-4" onClick={onClose}>
      <div className="bg-zinc-900 border border-zinc-700 rounded-2xl p-5 w-full max-w-sm" onClick={(e) => e.stopPropagation()}>
        <h3 className="text-zinc-100 font-semibold">{t('saveModal.title')}</h3>
        <label className="block mt-3">
          <span className="text-zinc-400 text-xs uppercase tracking-wider">{t('saveModal.name')}</span>
          <input
            autoFocus
            type="text" value={name} maxLength={80}
            onChange={(e) => setName(e.target.value)}
            placeholder={t('saveModal.ph')}
            className="mt-1 w-full bg-zinc-950 border border-zinc-800 rounded-lg px-3 py-2 text-zinc-100 focus:outline-none focus:border-copper-500"
          />
        </label>
        {err && <div className="mt-2 text-red-400 text-xs">{err}</div>}
        <div className="mt-4 flex justify-end gap-2">
          <button type="button" onClick={onClose} className="px-3 py-2 text-sm text-zinc-400 hover:text-zinc-200">
            {t('saveModal.cancel')}
          </button>
          <button
            type="button" onClick={handle} disabled={saving || !name.trim() || saved}
            className="bg-copper-500 hover:bg-copper-400 disabled:opacity-50 text-white font-semibold py-2 px-4 rounded-lg text-sm"
          >
            {saved ? t('saveModal.saved') : saving ? '…' : t('saveModal.save')}
          </button>
        </div>
      </div>
    </div>
  )
}
