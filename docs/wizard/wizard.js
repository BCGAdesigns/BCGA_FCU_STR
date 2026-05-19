// BCGA FCU — Setup Wizard (static, no backend)
// Naming: wizard + firmware + captive portal all use manual names (DN/DR/DP/DB).
// DB is shown in manual units (1 unit = 0.1 ms); the firmware stores and the
// portal exposes the same units, so summary values go in as-is (no conversion).

(() => {
  'use strict';

  // ── i18n ──────────────────────────────────────────────────────────────────
  const I18N = {
    pt: {
      'steps.1': 'Engine',
      'steps.2': 'FPS alvo',
      'steps.3': 'Chrono',
      'steps.4': 'Timings',
      'steps.5': 'Resumo',

      'nav.back': '← Voltar',
      'nav.next': 'Próximo →',

      's1.title': 'Passo 1 — Tipo de engine',
      's1.lead': 'Qual engine HPA está instalada no seu setup? Isso define quais timings a FCU usa.',
      's1.s8.sub': '1 solenoide',
      's1.s8.body': 'PolarStar JACK / F1, Wolverine INFERNO, GATE PULSAR S. Ciclo: DP → DR. Só DR e DP são usados.',
      's1.d8.sub': '2 solenoides',
      's1.d8.body': 'PolarStar F2 / Fusion, GATE PULSAR D. Ciclo: DN → DR → DP → DB. Os 4 timings são usados.',

      's2.title': 'Passo 2 — Chegar no FPS alvo',
      's2.lead': 'Antes de tudo, acerte o FPS alvo. O resto do tuning só faz sentido com o chrono onde você quer.',
      's2.fpsLabel': 'FPS alvo',
      's2.bbLabel': 'Massa da BB',
      's2.energy': 'Energia estimada:',
      's2.velocity': 'velocidade',
      's2.startTitle': 'Ponto de partida',
      's2.principle': 'Princípio: FPS é dado principalmente por pressão × DP. Os outros 3 timings moldam como o ciclo se comporta (alimentação, vedação, precisão, ROF) — não somam FPS.',

      's3.title': 'Passo 3 — Atirou no chrono?',
      's3.lead': 'Com regulador em 100 psi e timings default (DN=18 / DR=26 / DP=25 / DB=100), atire no chrono. FPS está no alvo?',
      's3.okHead': '✅ FPS no alvo',
      's3.okSub': 'Siga pro passo 4 e ajuste DN / DR / DB.',
      's3.lowHead': '⚠️ FPS abaixo do alvo',
      's3.lowSub': 'Escalonamento em 3 etapas.',
      's3.escTitle': 'Escalonamento para atingir o FPS',
      's3.esc1': 'Suba o DP até o máximo do slider (80 ms). Chrono. Se subiu: abaixe o DP passo a passo até o FPS começar a cair, volte 1–2 ms. Esse é seu DP mínimo eficiente.',
      's3.esc2': 'Se ainda estiver baixo: aumente o regulador para 110 psi. Chrono de novo. Se atingiu: abaixe o DP pro mínimo que segura o FPS alvo.',
      's3.esc3': 'Se ainda estiver baixo: aumente o regulador para 120 psi e ajuste o DP.',
      's3.escWarn': '⚠️ Nunca passe de 120 psi sem validar o equipamento pra essa pressão. Cheque sempre com chrono.',

      's4.title': 'Passo 4 — Ajuste fino dos timings',
      's4.lead': 'Cada timing controla uma fase física do ciclo. Ajuste nesta ordem: DN (alimentação), DR (vedação), DP (FPS — já feito), DB (precisão).',

      's4.dn.name': 'Nozzle Dwell (só D8PA)',
      's4.dn.desc': 'Duração do pulso do nozzle (SOL2). É quanto tempo o nozzle fica aberto para a BB cair na câmara.',
      's4.dn.rule': 'Regra prática: diminua DN até começar a dar tiro vazio, depois adicione 1–2 ms de margem.',
      's4.dn.warn': '⚠️ DN muito alto: risco de dupla alimentação. DN muito baixo: tiro vazio / pode não selar antes do DP.',

      's4.dr.name': 'Return / Rest',
      's4.dr.desc': 'D8PA: espera entre o pulso do nozzle e o pulso do poppet — tempo para o bucking selar na BB. S8PA: descanso entre tiros.',
      's4.dr.rule': 'Regra prática: suba DR até o chrono parar de oscilar em SEMI rápido.',
      's4.dr.warn': '⚠️ DR muito baixo: poppet dispara antes de selar → tiro fraco / FPS baixo e variável.',

      's4.dp.name': 'Poppet Dwell (o tiro)',
      's4.dp.desc': 'Duração do pulso do poppet (SOL1). É quanto tempo o ar pode fluir pelo nozzle empurrando a BB.',
      's4.dp.rule': 'Regra prática: com chrono, ajuste DP até chegar ao FPS alvo. Mais que isso é só desperdício de ar.',
      's4.dp.warn': '⚠️ DP é o único timing que controla FPS junto com a pressão. Já foi ajustado no passo 3.',

      's4.db.name': 'Trigger Debounce (só D8PA)',
      's4.db.desc': 'Espera após o poppet fechar, antes do gatilho poder armar o próximo ciclo. Fisicamente é o tempo para a BB sair do cano e o bucking se recuperar.',
      's4.db.unit': 'Unidade: 1 unit = 0,1 ms. Faixa: 20–800 units (2–80 ms). Default: 100 units (10 ms).',
      's4.db.ms': 'ms equivalente',
      's4.db.rule': 'Regra prática: atire em full-auto num alvo a 20 m. Se aparecerem flyers, aumente DB em 20 units (2 ms) por vez.',
      's4.db.warn': '⚠️ DB muito baixo: BB ainda no cano quando o próximo nozzle pulsa → flyers / FPS inconsistente.',

      's5.title': 'Passo 5 — Resumo',
      's5.lead': 'Valores para digitar no painel captive portal da FCU (conecte no WiFi BCGA_FCU_STR ou BCGA_FCU_PRO e abra 192.168.4.1).',
      's5.dlNote': 'O painel da FCU usa as mesmas unidades do manual: DN/DR/DP em ms e DB em units (0,1 ms). Digite os valores abaixo exatamente como estão.',
      's5.copyJson': 'Copiar como JSON',
      's5.copied': 'Copiado!',
      's5.restart': 'Recomeçar',

      'sum.slot.title': 'Slot',
      'sum.engine': 'Engine',
      'sum.target': 'FPS alvo',
      'sum.bb': 'Massa BB',
      'sum.energy': 'Energia',
      'sum.reg': 'Regulador',
      'sum.regNote': 'ajuste até o FPS atingir o alvo no chrono',
      'sum.timings': 'Timings (digitar no painel)',
      'sum.dn': 'DN',
      'sum.dr': 'DR',
      'sum.dp': 'DP',
      'sum.db': 'DB',
      'sum.units': 'units',
      'sum.ms': 'ms',
      'sum.s8Note': 'S8PA: DN e DB são ignorados pela FCU.',
    },
    en: {
      'steps.1': 'Engine',
      'steps.2': 'Target FPS',
      'steps.3': 'Chrono',
      'steps.4': 'Timings',
      'steps.5': 'Summary',

      'nav.back': '← Back',
      'nav.next': 'Next →',

      's1.title': 'Step 1 — Engine type',
      's1.lead': 'Which HPA engine is installed in your setup? This decides which timings the FCU uses.',
      's1.s8.sub': '1 solenoid',
      's1.s8.body': 'PolarStar JACK / F1, Wolverine INFERNO, GATE PULSAR S. Cycle: DP → DR. Only DR and DP are used.',
      's1.d8.sub': '2 solenoids',
      's1.d8.body': 'PolarStar F2 / Fusion, GATE PULSAR D. Cycle: DN → DR → DP → DB. All 4 timings are used.',

      's2.title': 'Step 2 — Reach target FPS',
      's2.lead': 'Before anything else, hit your target FPS. The rest of the tuning only makes sense once the chrono is where you want it.',
      's2.fpsLabel': 'Target FPS',
      's2.bbLabel': 'BB mass',
      's2.energy': 'Estimated energy:',
      's2.velocity': 'velocity',
      's2.startTitle': 'Starting point',
      's2.principle': 'Principle: FPS is driven mostly by air pressure × DP. The other 3 timings shape how the cycle behaves (feeding, sealing, accuracy, ROF) — they don\'t add FPS.',

      's3.title': 'Step 3 — Fired over the chrono?',
      's3.lead': 'With the regulator at 100 psi and default timings (DN=18 / DR=26 / DP=25 / DB=100), fire over a chrono. Is the FPS on target?',
      's3.okHead': '✅ FPS on target',
      's3.okSub': 'Go to step 4 and tune DN / DR / DB.',
      's3.lowHead': '⚠️ FPS below target',
      's3.lowSub': '3-step escalation.',
      's3.escTitle': 'FPS escalation',
      's3.esc1': 'Push DP to the slider maximum (80 ms). Chrono. If it jumped: lower DP step-by-step until FPS starts to drop, then add 1–2 ms back. That\'s your minimum efficient DP.',
      's3.esc2': 'Still low: raise the regulator to 110 psi. Chrono again. If it hits target: lower DP until you find the minimum that holds target FPS.',
      's3.esc3': 'Still low: raise the regulator to 120 psi and tune DP.',
      's3.escWarn': '⚠️ Don\'t exceed 120 psi without validating your hardware for that pressure. Always verify with a chrono.',

      's4.title': 'Step 4 — Fine-tune the timings',
      's4.lead': 'Each timing controls a physical phase of the cycle. Tune in this order: DN (feeding), DR (sealing), DP (FPS — already done), DB (accuracy).',

      's4.dn.name': 'Nozzle Dwell (D8PA only)',
      's4.dn.desc': 'Length of the nozzle (SOL2) pulse. This is how long the nozzle stays open for the BB to drop into the chamber.',
      's4.dn.rule': 'Rule of thumb: decrease DN until you start getting empty shots, then add 1–2 ms margin.',
      's4.dn.warn': '⚠️ DN too high: risk of double-feed. DN too low: empty shots / may not seal before DP fires.',

      's4.dr.name': 'Return / Rest',
      's4.dr.desc': 'D8PA: wait between the nozzle pulse and the poppet pulse — time for the bucking to seal on the BB. S8PA: inter-shot rest.',
      's4.dr.rule': 'Rule of thumb: raise DR until the chrono stops oscillating under fast SEMI.',
      's4.dr.warn': '⚠️ DR too low: poppet fires before seal → weak shot / low and variable FPS.',

      's4.dp.name': 'Poppet Dwell (the shot)',
      's4.dp.desc': 'Length of the poppet (SOL1) pulse. This is how long air can flow through the nozzle pushing the BB.',
      's4.dp.rule': 'Rule of thumb: with a chrono, tune DP up to your target FPS. Anything more just wastes air.',
      's4.dp.warn': '⚠️ DP is the only timing that drives FPS together with pressure. Already set in step 3.',

      's4.db.name': 'Trigger Debounce (D8PA only)',
      's4.db.desc': 'Wait after the poppet closes, before the trigger can re-arm the next cycle. Physically, this is the time for the BB to clear the barrel and the bucking to recover.',
      's4.db.unit': 'Unit: 1 unit = 0.1 ms. Range: 20–800 units (2–80 ms). Default: 100 units (10 ms).',
      's4.db.ms': 'equivalent ms',
      's4.db.rule': 'Rule of thumb: full-auto on a 20 m target. If flyers appear, bump DB by 20 units (2 ms) at a time.',
      's4.db.warn': '⚠️ DB too low: BB still in the barrel when the next nozzle pulses → flyers / inconsistent FPS.',

      's5.title': 'Step 5 — Summary',
      's5.lead': 'Values to type into the FCU\'s captive portal (connect to the BCGA_FCU_STR or BCGA_FCU_PRO WiFi and open 192.168.4.1).',
      's5.dlNote': 'The FCU panel uses the same units as the manual: DN/DR/DP in ms and DB in units (0.1 ms). Type the values below as-is.',
      's5.copyJson': 'Copy as JSON',
      's5.copied': 'Copied!',
      's5.restart': 'Restart',

      'sum.slot.title': 'Slot',
      'sum.engine': 'Engine',
      'sum.target': 'Target FPS',
      'sum.bb': 'BB mass',
      'sum.energy': 'Energy',
      'sum.reg': 'Regulator',
      'sum.regNote': 'tune until FPS hits target on the chrono',
      'sum.timings': 'Timings (type into the panel)',
      'sum.dn': 'DN',
      'sum.dr': 'DR',
      'sum.dp': 'DP',
      'sum.db': 'DB',
      'sum.units': 'units',
      'sum.ms': 'ms',
      'sum.s8Note': 'S8PA: DN and DB are ignored by the FCU.',
    },
  };

  // ── State ─────────────────────────────────────────────────────────────────
  const STORE_KEY = 'bcga_wizard_v1';
  const DEFAULT_TIMINGS = { dn: 18, dr: 26, dp: 25, db: 100 }; // db in manual units
  const defaultState = () => ({
    lang: 'pt',
    step: 1,
    engine: 0,          // 0 = unset, 1 = S8PA, 2 = D8PA
    fps: 350,
    bbMass: 0.28,
    fpsAchieved: null,  // null | 'ok' | 'low'
    timings: { ...DEFAULT_TIMINGS },
  });

  const loadState = () => {
    try {
      const raw = localStorage.getItem(STORE_KEY);
      if (!raw) return defaultState();
      const parsed = JSON.parse(raw);
      return { ...defaultState(), ...parsed, timings: { ...DEFAULT_TIMINGS, ...(parsed.timings || {}) } };
    } catch { return defaultState(); }
  };
  const saveState = () => localStorage.setItem(STORE_KEY, JSON.stringify(state));

  let state = loadState();

  // ── DOM helpers (textContent-only; no innerHTML) ──────────────────────────
  const el = (tag, cls, text) => {
    const node = document.createElement(tag);
    if (cls)  node.className = cls;
    if (text != null) node.textContent = String(text);
    return node;
  };
  const clear = (node) => { while (node.firstChild) node.removeChild(node.firstChild); };

  // ── i18n apply ────────────────────────────────────────────────────────────
  const t = (key) => (I18N[state.lang] && I18N[state.lang][key]) || key;
  const applyI18n = () => {
    document.documentElement.lang = state.lang === 'pt' ? 'pt-BR' : 'en';
    document.querySelectorAll('[data-i18n]').forEach(node => {
      node.textContent = t(node.getAttribute('data-i18n'));
    });
  };

  // ── Step navigation ───────────────────────────────────────────────────────
  const goStep = (n) => {
    state.step = n;
    saveState();
    document.querySelectorAll('.step').forEach(sec => {
      sec.hidden = Number(sec.dataset.step) !== n;
    });
    document.querySelectorAll('.steps li').forEach(li => {
      const s = Number(li.dataset.step);
      li.classList.toggle('current', s === n);
      li.classList.toggle('done', s < n);
    });
    applyD8paGating();
    if (n === 2) { updateEnergy(); renderPreset(); }
    if (n === 5) renderSummary();
    window.scrollTo({ top: 0, behavior: 'smooth' });
  };

  // ── Engine gating (hide D8PA-only panels for S8PA) ────────────────────────
  const applyD8paGating = () => {
    const isS8 = state.engine === 1;
    document.querySelectorAll('[data-d8pa-only]').forEach(node => {
      node.hidden = isS8;
    });
    document.querySelectorAll('[data-engine]').forEach(btn => {
      btn.classList.toggle('active', Number(btn.dataset.engine) === state.engine);
    });
  };

  // ── Energy calc (step 2) ──────────────────────────────────────────────────
  const updateEnergy = () => {
    const fpsEl  = document.getElementById('fps');
    const bbMassEl = document.getElementById('bbMass');
    const fps = Number(fpsEl && fpsEl.value) || 0;
    const g   = Number(bbMassEl && bbMassEl.value) || 0;
    const v   = fps * 0.3048;   // m/s
    const m   = g / 1000;       // kg
    const E   = 0.5 * m * v * v;
    const energyNode = document.getElementById('energyJ');
    const velNode    = document.getElementById('velocityMs');
    if (energyNode) energyNode.textContent = E.toFixed(2);
    if (velNode)    velNode.textContent    = v.toFixed(1);
    state.fps = fps;
    state.bbMass = g;
    saveState();
  };

  const renderPreset = () => {
    const host = document.getElementById('preset');
    if (!host) return;
    clear(host);
    const lines = [
      ['regulator', '100 psi'],
      ['DN', '18 ms'],
      ['DR', '26 ms'],
      ['DP', '25 ms'],
      ['DB', '100 units (10 ms)'],
    ];
    lines.forEach(([k, v]) => {
      const row = el('div', 'kv');
      row.appendChild(el('span', 'k', k));
      row.appendChild(el('span', 'v', v));
      host.appendChild(row);
    });
  };

  // ── Summary (step 5) ──────────────────────────────────────────────────────
  const summaryRow = (k, v) => {
    const row = el('div', 'srow');
    row.appendChild(el('span', 'k', k));
    row.appendChild(el('span', 'v', v));
    return row;
  };
  const renderSummary = () => {
    const host = document.getElementById('summary');
    if (!host) return;
    clear(host);

    const isS8 = state.engine === 1;
    const engineName = isS8 ? 'S8PA' : 'D8PA';
    const fps = state.fps;
    const m   = state.bbMass;
    const v   = fps * 0.3048;
    const E   = 0.5 * (m / 1000) * v * v;

    const dbUnits = state.timings.db;
    const dbMsRaw = dbUnits / 10;

    host.appendChild(el('h3', null, `${t('sum.slot.title')} 1`));
    host.appendChild(summaryRow(t('sum.engine'), engineName));
    host.appendChild(summaryRow(t('sum.target'), `${fps} FPS`));
    host.appendChild(summaryRow(t('sum.bb'),    `${m.toFixed(2)} g`));
    host.appendChild(summaryRow(t('sum.energy'), `${E.toFixed(2)} J · ${v.toFixed(1)} m/s`));
    host.appendChild(summaryRow(t('sum.reg'),   '100 psi'));
    host.appendChild(el('div', 'note', t('sum.regNote')));

    host.appendChild(el('h3', null, t('sum.timings')));
    if (!isS8) host.appendChild(summaryRow(t('sum.dn'), `${state.timings.dn} ${t('sum.ms')}`));
    host.appendChild(summaryRow(t('sum.dr'), `${state.timings.dr} ${t('sum.ms')}`));
    host.appendChild(summaryRow(t('sum.dp'), `${state.timings.dp} ${t('sum.ms')}`));
    if (!isS8) {
      host.appendChild(summaryRow(
        t('sum.db'),
        `${dbUnits} ${t('sum.units')} (= ${dbMsRaw.toFixed(1)} ${t('sum.ms')})`
      ));
    } else {
      host.appendChild(el('div', 'note', t('sum.s8Note')));
    }
  };

  // ── Copy-as-JSON (SlotConfig shape from firmware/BCGA_FCU_STR/types.h) ────
  const copyJson = async () => {
    const isS8 = state.engine === 1;
    const config = {
      name: 'BCGA Wizard',
      solenoidCount: isS8 ? 1 : 2,
      targetFps: state.fps,
      bbMassG: state.bbMass,
      regulatorPsi: 100,
      dn: state.timings.dn,
      dr: state.timings.dr,
      dp: state.timings.dp,
      db: state.timings.db,
      _note: 'DN/DR/DP in ms, DB in units of 0.1 ms — matches the captive portal fields 1:1.',
    };
    const text = JSON.stringify(config, null, 2);
    try {
      await navigator.clipboard.writeText(text);
    } catch {
      const ta = document.createElement('textarea');
      ta.value = text; document.body.appendChild(ta);
      ta.select(); document.execCommand('copy'); ta.remove();
    }
    const btn = document.getElementById('copyJson');
    if (!btn) return;
    btn.textContent = t('s5.copied');
    setTimeout(() => { btn.textContent = t('s5.copyJson'); }, 1200);
  };

  const restart = () => {
    localStorage.removeItem(STORE_KEY);
    state = defaultState();
    applyI18n();
    const fpsEl = document.getElementById('fps');
    const bbEl  = document.getElementById('bbMass');
    if (fpsEl) fpsEl.value = state.fps;
    if (bbEl)  bbEl.value  = state.bbMass;
    const escEl = document.getElementById('escalation');
    if (escEl) escEl.hidden = true;
    goStep(1);
  };

  // ── Wire up ───────────────────────────────────────────────────────────────
  document.addEventListener('DOMContentLoaded', () => {
    applyI18n();

    // Language toggle
    document.getElementById('langToggle').addEventListener('click', () => {
      state.lang = state.lang === 'pt' ? 'en' : 'pt';
      saveState();
      applyI18n();
      if (state.step === 5) renderSummary();
      if (state.step === 2) renderPreset();
    });

    // Step 1 — engine pick
    document.querySelectorAll('[data-engine]').forEach(btn => {
      btn.addEventListener('click', () => {
        state.engine = Number(btn.dataset.engine);
        saveState();
        goStep(2);
      });
    });

    // Step 2 — FPS / BB inputs
    const fpsEl = document.getElementById('fps');
    const bbEl  = document.getElementById('bbMass');
    if (fpsEl) { fpsEl.value = state.fps; fpsEl.addEventListener('input', updateEnergy); }
    if (bbEl)  { bbEl.value  = state.bbMass; bbEl.addEventListener('input', updateEnergy); }

    // Step 3 — branch buttons
    const fpsOk  = document.getElementById('fpsOk');
    const fpsLow = document.getElementById('fpsLow');
    const esc    = document.getElementById('escalation');
    if (fpsOk) fpsOk.addEventListener('click', () => {
      state.fpsAchieved = 'ok'; saveState();
      if (esc) esc.hidden = true;
      goStep(4);
    });
    if (fpsLow) fpsLow.addEventListener('click', () => {
      state.fpsAchieved = 'low'; saveState();
      if (esc) esc.hidden = false;
    });

    // Generic [data-goto]
    document.querySelectorAll('[data-goto]').forEach(btn => {
      btn.addEventListener('click', () => goStep(Number(btn.dataset.goto)));
    });

    // Step 5 actions
    const copyBtn    = document.getElementById('copyJson');
    const restartBtn = document.getElementById('restart');
    if (copyBtn)    copyBtn.addEventListener('click', copyJson);
    if (restartBtn) restartBtn.addEventListener('click', restart);

    // Restore step
    goStep(state.step || 1);
  });
})();
