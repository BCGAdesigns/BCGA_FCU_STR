"use strict";
const IS_PRO = document.body.classList.contains("variant-pro");

const I = {
  br:{
    slotName:"Nome do slot",
    solSec:"Engine",
    solSHead:"Single Solenoide S8PA",solDHead:"Dual Solenoide D8PA",
    solS:"1 solenoide (Jack, Backdraft)",solD:"2 solenoides (F2, Pulsar)",
    timSec:"Timings",rofLbl:"ROF máx (rps)",rofTheoLbl:"ROF teórico",semiRof:"Semi Max ROF (ms)",
    selSec:"Seletor",pos1:"Posição 1",pos2:"Posição 2",pos3:"Posição 3",
    sel3:"Seletor de 3 posições",sel3H:"habilitar Pos 3 (ativa Hall)",
    funcSec:"Funções",advSec:"Avançado",
    statsSec:"Estatísticas",statBattLbl:"Bateria",statShotsSession:"Tiros (sessão)",statShotsTotal:"Tiros (total)",statLube:"Próxima lubrificação",
    cfgLock:"Config LOCK",cfgLockOff:"OFF (sem senha)",cfgLockOnUnlocked:"ATIVO — destrancado",
    cfgLockSetLbl:"Nova senha",cfgLockSet:"Definir",cfgLockLock:"Trancar agora",cfgLockClear:"Remover senha",
    cfgLockTooShort:"Senha curta (mín. 4)",cfgLockOk:"Senha gravada",cfgLockCleared:"Senha removida",
    cfgLockUnlocked:"Destrancado",cfgLockBadPwd:"Senha incorreta",
    cfgLockClearConfirm:"Remover a senha do Config LOCK?",
    lockTitle:"Configuração trancada",lockMsg:"Digite a senha para destravar a configuração do FCU.",
    lockPwd:"Senha",lockUnlock:"Destrancar",
    lockForgotHint:"Esqueceu a senha? Segure o gatilho 30 s no boot para reset de fábrica.",
    trigIn:"Gatilho",selIn:"Tipo do seletor",sw:"Microswitch",hl:"Hall (analog)",
    invTrig:"Inverter gatilho",
    mosSwap:"Trocar MOS 1 ↔ 2",mosSwapH:"corrigir fiação trocada",
    silent:"Modo silencioso",silentH:"não tocar buzzer no uso",
    sensSec:"Sensibilidade do gatilho",sensHelp:"Puxe o gatilho até onde quer que dispare e clique em Salvar.",
    curPoint:"Ponto atual",savePoint:"Salvar ponto",liveAdc:"Leitura ADC",
    calTrigSec:"Calibração do gatilho",calTrigHelp:"Comece pelo ruído (faz 1 disparo de teste). Não puxe o gatilho.",
    calSelSec:"Calibração do seletor",calSelHelp:"Comece pelo ruído (faz 1 disparo de teste). Não toque no seletor.",
    calNoise:"Ruído",calTrig:"Gatilho",calSel:"Seletor (3 pos)",calBtn:"Calibrar",
    diagSec:"Diagnóstico",buzz:"Buzzer",bzMode:"Trocar modo",bzSave:"Salvar OK",bzWifi:"WiFi ON",bzLow:"Bateria baixa",bzCut:"Bateria CUT",bzTest:"Teste",play:"Tocar",mosTest:"Teste MOSFET (2s)",
    off:"off",
    trigTest:"Teste gatilho",trigTestStart:"Iniciar",trigTestStop:"Parar",trigTestIdle:"—",trigTestDone:"teste concluído",
    wifiSec:"WiFi",newPwd:"Nova senha",changePwd:"Trocar senha",
    reload:"Recarregar",save:"Salvar slot",resetSlot:"Resetar slot",cancel:"Cancelar",capture:"Capturar",saveCal:"Salvar",
    fSafe:"SAFE",fSemi:"SEMI",fFull:"FULL",fB2:"Burst 2",fB3:"Burst 3",fB4:"Burst 4",
    saved:"Slot salvo",saveFail:"Falha ao salvar",reloaded:"Recarregado",resetOk:"Slot resetado",
    resetConfirm:"Restaurar este slot para os valores padrão?",
    pwdOk:"Senha trocada",pwdBad:"Senha curta (mín. 8)",buzzed:"Buzzer ✓",
    mosOk:"MOSFET pulsando 2s",mosFail:"Não foi possível",
    pointSaved:"Ponto salvo: ADC ",
    noiseRunning:"Disparando ciclo de teste…",noiseOk:"Ruído capturado: ±",
    calStep1:"Solte completamente o gatilho e clique em Capturar.",
    calStep2:"Pressione o gatilho até o fim e clique em Capturar.",
    calStep3:"Valores capturados. Clique em Salvar para gravar.",
    calStepSel:"Mova o seletor para a posição",
    calRangeErr:"Curso insuficiente — repita.",
    help:{
      dn:"Tempo (ms) que a SOL 2 (nozzle) fica acionada — recua o nozzle para alimentar a BB. Aumente se a arma estiver falhando alimentação. Padrão 18 ms.",
      dr:"Em D8PA: espera entre DN e DP, dá tempo da mola empurrar o bico e vedar o bucking. Em S8PA: descanso entre tiros. Padrão 26 ms (D8PA) / 20 ms (S8PA).",
      dp:"Tempo (ms) que a SOL 1 (poppet) fica aberta — quanto maior, mais ar passa pelo cano. Aumente para canos longos ou BBs pesadas. Padrão 25 ms.",
      db:"Debounce do gatilho em unidades de 0,1 ms (1 unidade = 100 µs). Pausa antes do próximo ciclo, esperando a BB sair do cano. Só D8PA. Faixa 5–150 (0,5–15 ms). Padrão 5.",
      is:"Tempo (segundos) sem disparar até armar o boost antiaderência. Quando passar esse tempo parado, o próximo tiro recebe DP+IP. iP=0 desativa o boost.",
      ip:"Multiplicador do DP só no primeiro tiro depois do iS expirar. IP=1 → tiro com 2×DP, IP=2 → 3×DP, etc. Quebra a aderência do o-ring do poppet. Aumente até o primeiro tiro sair com o FPS normal.",
      rof:"Limite de cadência em tiros por segundo (rps). 0 = sem limite (cadência só pelos timings).",
      semiRof:"Tempo mínimo (ms) entre tiros em SEMI. Impede auto-disparo acidental com gatilhos rápidos. 0 = desativado.",
      rofTheo:"Cadência teórica em rps calculada a partir dos timings: 1000 ÷ (soma dos delays).",
      pos:"Modo de fogo quando o seletor está nesta posição.",
      sel3:"Habilita uma terceira posição do seletor. Requer sensor Hall (a opção de 3 stops digital não existe). Ao ligar, o seletor passa automaticamente para Hall.",
      trigMode:"Tipo do sensor do gatilho: microswitch digital ou Hall analógico.",
      selMode:"Tipo do sensor do seletor: microswitch digital ou Hall analógico. Hall é obrigatório se 3-pos estiver ativo. Quando Hall, a seção de calibração do seletor aparece.",
      mosSwap:"Se você soldou nozzle e poppet nos pinos trocados, marque para corrigir via software (D8PA).",
      silent:"Não toca confirmação do buzzer durante o uso normal.",
      sens:"Puxe o gatilho até o ponto onde quer disparar e clique Salvar — o ADC vira o ponto de disparo.",
      noise:"Faz 1 disparo completo medindo o ruído elétrico no Hall. Use ANTES de calibrar gatilho para definir a margem da deadband.",
      cal:"Captura os valores ADC do gatilho/seletor solto e pressionado.",
      mosTest:"Pulsa o MOSFET por 2 segundos. Use só para conferir a fiação — sem gas/munição.",
      trigTest:"Acende o indicador enquanto você puxa o gatilho. Não dispara solenoide. Auto-encerra após 15s sem atividade.",
      buzz:"Toca um padrão de aviso. Útil para testar o buzzer.",
      pwd:"Senha do AP WiFi. Mínimo 8 caracteres.",
      cfgLock:"Quando ativo e trancado, salvar slots e trocar de slot via web ficam bloqueados até você destrancar com a senha. Se esquecer a senha, segure o gatilho por 30 s no boot para reset de fábrica."
    }
  },
  en:{
    slotName:"Slot name",
    solSec:"Engine",
    solSHead:"Single Solenoid S8PA",solDHead:"Dual Solenoid D8PA",
    solS:"1 solenoid (Jack, Backdraft)",solD:"2 solenoids (F2, Pulsar)",
    timSec:"Timings",rofLbl:"ROF cap (rps)",rofTheoLbl:"Theoretical ROF",semiRof:"Semi Max ROF (ms)",
    selSec:"Selector",pos1:"Position 1",pos2:"Position 2",pos3:"Position 3",
    sel3:"3-position selector",sel3H:"enable Pos 3 (auto-Hall)",
    funcSec:"Functions",advSec:"Advanced",
    statsSec:"Statistics",statBattLbl:"Battery",statShotsSession:"Session shots",statShotsTotal:"Total shots",statLube:"Next lubrication",
    cfgLock:"Config LOCK",cfgLockOff:"OFF (no password)",cfgLockOnUnlocked:"ON — unlocked",
    cfgLockSetLbl:"New password",cfgLockSet:"Set",cfgLockLock:"Lock now",cfgLockClear:"Clear password",
    cfgLockTooShort:"Password too short (min 4)",cfgLockOk:"Password saved",cfgLockCleared:"Password cleared",
    cfgLockUnlocked:"Unlocked",cfgLockBadPwd:"Wrong password",
    cfgLockClearConfirm:"Clear the Config LOCK password?",
    lockTitle:"Configuration locked",lockMsg:"Enter the password to unlock the FCU configuration.",
    lockPwd:"Password",lockUnlock:"Unlock",
    lockForgotHint:"Forgot the password? Hold the trigger at boot for 30 s to factory-reset.",
    trigIn:"Trigger",selIn:"Selector type",sw:"Microswitch",hl:"Hall (analog)",
    invTrig:"Invert trigger",
    mosSwap:"Swap MOS 1 ↔ 2",mosSwapH:"fix swapped wiring",
    silent:"Silent mode",silentH:"skip buzzer while in use",
    sensSec:"Trigger sensitivity",sensHelp:"Pull trigger to the desired fire point and click Save.",
    curPoint:"Current point",savePoint:"Save point",liveAdc:"ADC reading",
    calTrigSec:"Trigger calibration",calTrigHelp:"Start with noise (fires 1 test cycle). Don't pull the trigger.",
    calSelSec:"Selector calibration",calSelHelp:"Start with noise (fires 1 test cycle). Don't touch the selector.",
    calNoise:"Noise",calTrig:"Trigger",calSel:"Selector (3 pos)",calBtn:"Calibrate",
    diagSec:"Diagnostics",buzz:"Buzzer",bzMode:"Mode change",bzSave:"Save OK",bzWifi:"WiFi ON",bzLow:"Low battery",bzCut:"Battery CUT",bzTest:"Test",play:"Play",mosTest:"MOSFET test (2s)",
    off:"off",
    trigTest:"Trigger test",trigTestStart:"Start",trigTestStop:"Stop",trigTestIdle:"—",trigTestDone:"test ended",
    wifiSec:"WiFi",newPwd:"New password",changePwd:"Change password",
    reload:"Reload",save:"Save slot",resetSlot:"Reset slot",cancel:"Cancel",capture:"Capture",saveCal:"Save",
    fSafe:"SAFE",fSemi:"SEMI",fFull:"FULL",fB2:"Burst 2",fB3:"Burst 3",fB4:"Burst 4",
    saved:"Slot saved",saveFail:"Save failed",reloaded:"Reloaded",resetOk:"Slot reset",
    resetConfirm:"Reset this slot to default values?",
    pwdOk:"Password changed",pwdBad:"Password too short (min 8)",buzzed:"Buzzer ✓",
    mosOk:"MOSFET pulsing 2s",mosFail:"Could not start",
    pointSaved:"Point saved: ADC ",
    noiseRunning:"Firing test cycle…",noiseOk:"Noise captured: ±",
    calStep1:"Fully release the trigger and click Capture.",
    calStep2:"Press trigger fully and click Capture.",
    calStep3:"Captured. Click Save to write.",
    calStepSel:"Move selector to position",
    calRangeErr:"Travel too short — retry.",
    help:{
      dn:"Time (ms) SOL 2 (nozzle) is energized — pulls back the nozzle to feed a BB. Increase if feeding fails. Default 18 ms.",
      dr:"In D8PA: wait between DN and DP for the spring to seal the bucking. In S8PA: inter-shot rest. Default 26 ms (D8PA) / 20 ms (S8PA).",
      dp:"Time (ms) SOL 1 (poppet) stays open — longer = more air through the barrel. Increase for long barrels or heavy BBs. Default 25 ms.",
      db:"Trigger debounce in units of 0.1 ms (1 unit = 100 µs). Pause before the next cycle, waiting for the BB to exit the barrel. D8PA only. Range 5–150 (0.5–15 ms). Default 5.",
      is:"Idle time (seconds) before arming the anti-stiction boost. After this much idle, the next shot fires with DP+IP. Set IP=0 to disable.",
      ip:"DP multiplier applied only to the first shot after IS expires. IP=1 → shot fires at 2×DP, IP=2 → 3×DP, etc. Breaks the poppet o-ring stiction. Raise it until the first shot reaches normal FPS.",
      rof:"Rate cap in rounds per second (rps). 0 = unlimited (cadence ruled by timings only).",
      semiRof:"Minimum time (ms) between SEMI shots. Blocks accidental double-taps from fast triggers. 0 = disabled.",
      rofTheo:"Theoretical cadence in rps from timings: 1000 ÷ (sum of delays).",
      pos:"Fire mode when the selector is in this position.",
      sel3:"Enables a third selector position. Requires Hall sensor (no 3-stop digital). Toggling on auto-switches the selector input to Hall.",
      trigMode:"Trigger sensor type: digital microswitch or analog Hall.",
      selMode:"Selector sensor type: digital microswitch or analog Hall. Hall is required when 3-pos is enabled. When Hall, the selector calibration section appears.",
      mosSwap:"If you soldered nozzle/poppet to the wrong pins, check this to fix in software (D8PA).",
      silent:"Skips buzzer confirmations during normal use.",
      sens:"Pull trigger to the point where you want it to fire and click Save — that ADC becomes the fire point.",
      noise:"Fires 1 full cycle while sampling the Hall ADC. Run BEFORE calibrating the trigger to set the deadband margin.",
      cal:"Captures ADC values for released/pressed trigger or each selector position.",
      mosTest:"Pulses the MOSFET for 2 seconds. Use only to check wiring — no gas/ammo.",
      trigTest:"Lights the indicator while you pull the trigger. Does not fire any solenoid. Auto-stops after 15s idle.",
      buzz:"Plays a warning pattern. Useful to test the buzzer.",
      pwd:"WiFi AP password. Minimum 8 characters.",
      cfgLock:"When active and locked, slot save/switch/reset via web are blocked until you unlock with the password. If you forget the password, hold the trigger at boot for 30 s to factory-reset."
    }
  }
};
let lang = "en";
let cur = { i:0, slot:null, meta:null };
let noiseMargin    = 0;
let noiseMarginSel = 0;
let openHelp = null;
let sensPollT = null;

function $(id){ return document.getElementById(id); }
function show(el, on){ if(!el) return; el.classList.toggle("hide", !on); }

async function jget(url){
  const r = await fetch(url, {cache:"no-store"});
  return r.json();
}
async function jpost(url, body){
  const r = await fetch(url, {method:"POST", headers:{"content-type":"application/json"}, body: JSON.stringify(body||{})});
  return r.json();
}

function toast(msg, isErr){
  const t = $("toast"); t.textContent = msg;
  t.classList.toggle("err", !!isErr);
  t.classList.add("show");
  clearTimeout(toast._t);
  toast._t = setTimeout(()=>t.classList.remove("show"), 2000);
}

function applyLang(){
  document.querySelectorAll("[data-i]").forEach(el=>{
    const k = el.getAttribute("data-i");
    if (I[lang][k]!=null && typeof I[lang][k] === "string") el.textContent = I[lang][k];
  });
  $("langBR").classList.toggle("on", lang==="br");
  $("langEN").classList.toggle("on", lang==="en");
  fillModeOpts($("selPos1"));
  fillModeOpts($("selPos2"));
  fillModeOpts($("selPos3"));
  if (cur.slot) {
    $("selPos1").value = cur.slot.selPos1;
    $("selPos2").value = cur.slot.selPos2;
    $("selPos3").value = cur.slot.selPos3;
  }
  recalcRofTheo();
  refreshStepperUi();
}

function fillModeOpts(sel){
  while (sel.firstChild) sel.removeChild(sel.firstChild);
  const opts = [
    ["0", I[lang].fSafe],
    ["1", I[lang].fSemi],
    ["2", I[lang].fFull],
    ["3", I[lang].fB2],
    ["4", I[lang].fB3],
    ["5", I[lang].fB4]
  ];
  for (const [v,t] of opts){
    const o = document.createElement("option");
    o.value = v; o.textContent = t;
    sel.appendChild(o);
  }
}

function fillSlotPicker(n, last){
  const sp = $("slotPick");
  while (sp.firstChild) sp.removeChild(sp.firstChild);
  for (let i=0;i<n;i++){
    const b = document.createElement("button");
    b.type = "button";
    b.className = "slot-btn"+(i===last?" on":"");
    b.textContent = String(i+1);
    b.dataset.idx = String(i);
    b.addEventListener("click", async ()=>{
      const idx = parseInt(b.dataset.idx,10);
      await loadSlot(idx);
      await jpost("/setslot?i="+idx, {});
      cur.meta.lastSlot = idx;
      Array.from(sp.children).forEach(c=>c.classList.remove("on"));
      b.classList.add("on");
      updateHeader();
    });
    sp.appendChild(b);
  }
}

function classifyBatt(b){
  if (!b) return null;
  if (b.cut) return "cut";
  if (b.low) return "err";
  const cells = b.cells|0;
  if (cells <= 0) return null;
  const perCell = (b.mv|0) / cells;
  if (perCell >= 3700) return "ok";
  if (perCell >= 3500) return "warn";
  return "err";
}

function formatBattText(b){
  if (!b) return "—";
  const mv = b.mv|0;
  const cells = b.cells|0;
  let txt = (mv/1000).toFixed(2)+"V";
  if (cells) txt += " ("+cells+"S)";
  if (b.cut) txt += " CUT";
  return txt;
}

function updateBatteryChip(){
  const chip = $("battChip");
  if (!chip) return;
  if (!cur.meta || !cur.meta.batt){
    show(chip, false);
    return;
  }
  show(chip, true);
  const b = cur.meta.batt;
  chip.classList.remove("ok","warn","err","cut");
  const cls = classifyBatt(b);
  if (b.cut) chip.classList.add("cut");
  if (cls) chip.classList.add(cls);
  $("battTxt").textContent = formatBattText(b);
}

function updateLockUi(){
  if (!cur.meta) return;
  const lk = cur.meta.cfgLock || {hasPwd:false, locked:false};
  const status   = $("cfgLockStatus");
  const rowSet   = $("rowCfgLockSet");
  const rowMng   = $("rowCfgLockManage");
  const overlay  = $("lockOver");
  if (overlay) show(overlay, !!lk.locked);
  if (status){
    status.textContent = lk.hasPwd ? I[lang].cfgLockOnUnlocked : I[lang].cfgLockOff;
  }
  if (rowSet) show(rowSet, !lk.hasPwd);
  if (rowMng) show(rowMng, !!lk.hasPwd);
}

function updateStats(){
  if (!cur.meta) return;
  const total   = (cur.meta.shotsTotal   != null ? cur.meta.shotsTotal   : (cur.meta.shots|0)) | 0;
  const session = (cur.meta.shotsSession != null ? cur.meta.shotsSession : 0) | 0;

  const sV = $("statShotsSessionVal"); if (sV) sV.textContent = session.toLocaleString();
  const tV = $("statShotsTotalVal");   if (tV) tV.textContent = total.toLocaleString();

  const lubeFill = $("lubeFill");
  const lubeTxt  = $("lubeTxt");
  if (lubeFill && lubeTxt){
    const interval = 10000;
    const into = total % interval;
    const pct = (into / interval) * 100;
    lubeFill.style.width = Math.min(100, pct).toFixed(1) + "%";
    lubeFill.classList.remove("warn","due");
    if (pct >= 95)      lubeFill.classList.add("due");
    else if (pct >= 80) lubeFill.classList.add("warn");
    lubeTxt.textContent = into.toLocaleString() + " / " + interval.toLocaleString();
  }

  const battStat = $("statBatt");
  const battVal  = $("statBattVal");
  if (battStat && battVal){
    if (cur.meta.batt){
      show(battStat, true);
      battVal.classList.remove("ok","warn","err","cut");
      const cls = classifyBatt(cur.meta.batt);
      if (cur.meta.batt.cut) battVal.classList.add("cut");
      if (cls) battVal.classList.add(cls);
      battVal.textContent = formatBattText(cur.meta.batt);
    } else {
      show(battStat, false);
    }
  }
}

function updateHeader(){
  if (!cur.meta) return;
  const variant = cur.meta.variant || (IS_PRO ? "—" : "Starter");
  const ssid = cur.meta.ssid || ("BCGA_FCU_" + (IS_PRO ? "PRO" : "STR"));
  const sol = (cur.slot && cur.slot.solenoids===1) ? "S8PA" : "D8PA";
  $("hdrName").textContent = ssid.replace(/_/g," ");
  $("hdrSub").textContent = "FCU "+variant+" • "+sol;
  $("ftVer").textContent = "v"+(cur.meta.fw||"");
  $("ftVar").textContent = variant;
  if (IS_PRO) updateBatteryChip();
  updateStats();
  updateLockUi();
}

function recalcRofTheo(){
  if (!cur.slot) { $("rofTheoVal").textContent = "— rps"; return; }
  const ms = (k)=> Math.max(2, Math.min(80, parseInt($(k+"N").value||"0",10)));
  const dbMs = ()=> Math.max(0.5, Math.min(15, (parseInt($("dbN").value||"0",10) || 0) / 10));
  const isS = cur.slot.solenoids === 1;
  const sum = isS ? (ms("dp") + ms("dr"))
                  : (ms("dn") + ms("dr") + ms("dp") + dbMs());
  if (!sum) { $("rofTheoVal").textContent = "— rps"; return; }
  const rps = 1000 / sum;
  $("rofTheoVal").textContent = rps.toFixed(1) + " rps";
}

function refreshSolenoidUI(){
  if (!cur.slot) return;
  const isS = cur.slot.solenoids===1;
  $("solS").classList.toggle("on", isS);
  $("solD").classList.toggle("on", !isS);
  show($("rowDn"), !isS);
  show($("rowDl"), !isS);
  show($("mos2Btn"), !isS);
  show($("rowMosSwap"), !isS);
  recalcRofTheo();
  updateHeader();
}

function refreshSelectorUI(){
  if (!cur.slot) return;
  if (cur.slot.sel3pos) cur.slot.selMode = 1;       // 3-pos always implies Hall
  $("sel3pos").checked = !!cur.slot.sel3pos;
  show($("rowPos3"), !!cur.slot.sel3pos);
  const selModeEl = $("selMode");
  if (selModeEl){
    selModeEl.value = cur.slot.selMode;
    // Lock the dropdown to Hall when 3-pos is active — switching back would
    // make the third position unreachable on a digital switch.
    selModeEl.disabled = !!cur.slot.sel3pos;
  }
  refreshCalUI();
}

function refreshCalUI(){
  if (!cur.slot) return;
  const trigHall = cur.slot.trigMode===1;
  const selHall  = cur.slot.selMode===1;
  show($("secCalTrig"), trigHall);
  show($("secCalSel"),  selHall);
  show($("secSens"),    trigHall);
  $("calTrigVals").textContent = "low="+cur.slot.hallTrigLow+" high="+cur.slot.hallTrigHigh;
  $("calSelVals").textContent  = "low1="+cur.slot.hallSelLow1+" low2="+cur.slot.hallSelLow2;
  $("curPoint").textContent = cur.slot.hallTrigHigh;
  if (trigHall) startSensPoll(); else stopSensPoll();
}

function applySlotToUi(s){
  cur.slot = s;
  $("slotName").value = s.name||"";
  fillModeOpts($("selPos1"));
  fillModeOpts($("selPos2"));
  fillModeOpts($("selPos3"));
  $("selPos1").value = s.selPos1;
  $("selPos2").value = s.selPos2;
  $("selPos3").value = s.selPos3;
  $("trigMode").value  = s.trigMode;
  $("sel3pos").checked = !!s.sel3pos;
  $("mosfetSwap").checked = !!s.mosfetSwap;
  $("invertTrig").checked = !!s.invertTrig;
  $("silent").checked     = !!s.silent;
  for (const k of ["dn","dr","dp","db"]){
    $(k+"R").value = s[k]; $(k+"N").value = s[k];
  }
  $("rofN").value = s.rof;
  $("semiRofN").value = s.semiRofMs || 0;
  refreshStepperUi();
  refreshSolenoidUI();
  refreshSelectorUI();
}

async function loadMeta(){
  cur.meta = await jget("/load");
  if (cur.meta.lang) lang = (cur.meta.lang==="br") ? "br" : "en";
  fillSlotPicker(cur.meta.slots|0, cur.meta.lastSlot|0);
  cur.i = cur.meta.lastSlot|0;
  applyLang();
  updateHeader();
}

async function loadSlot(i){
  const r = await jget("/getslot?i="+i);
  if (!r.ok) { toast("ERR "+(r.error||""), true); return; }
  cur.i = i;
  applySlotToUi(r.slot);
}

function readSlotFromUi(){
  const pair = (k)=> Math.max(2, Math.min(80, parseInt($(k+"N").value||"0",10)));
  const dbUnits = ()=> Math.max(5, Math.min(150, parseInt($("dbN").value||"0",10)));
  return {
    name: $("slotName").value || ("Slot "+(cur.i+1)),
    solenoids: $("solD").classList.contains("on") ? 2 : 1,
    trigMode: parseInt($("trigMode").value,10),
    selMode:  cur.slot.selMode,
    sel3pos:  $("sel3pos").checked,
    selPos1:  parseInt($("selPos1").value,10),
    selPos2:  parseInt($("selPos2").value,10),
    selPos3:  parseInt($("selPos3").value,10),
    dn: pair("dn"), dr: pair("dr"), dp: pair("dp"), db: dbUnits(),
    rof:   Math.max(0, Math.min(50, parseInt($("rofN").value||"0",10))),
    semiRofMs: Math.max(0, Math.min(500, parseInt($("semiRofN").value||"0",10))),
    is: Math.max(0, Math.min(600, (cur.slot && cur.slot.is|0) || 0)),
    ip: Math.max(0, Math.min(5,   (cur.slot && cur.slot.ip|0) || 0)),
    hallTrigLow:  cur.slot.hallTrigLow,
    hallTrigHigh: cur.slot.hallTrigHigh,
    hallSelLow1:  cur.slot.hallSelLow1,
    hallSelLow2:  cur.slot.hallSelLow2,
    mosfetSwap: $("mosfetSwap").checked,
    invertTrig: $("invertTrig").checked,
    silent:     $("silent").checked,
    makeActive: true
  };
}

async function saveSlot(){
  const body = readSlotFromUi();
  const r = await jpost("/save?i="+cur.i, body);
  if (r.ok) { toast(I[lang].saved); cur.slot = body; updateHeader(); }
  else toast(I[lang].saveFail+": "+(r.error||""), true);
}

async function resetSlot(){
  if (!confirm(I[lang].resetConfirm)) return;
  const r = await jpost("/reset?i="+cur.i, {});
  if (r.ok && r.slot) {
    applySlotToUi(r.slot);
    toast(I[lang].resetOk);
  } else {
    toast(I[lang].saveFail+": "+(r.error||""), true);
  }
}

function bindPair(k){
  const r = $(k+"R"), n = $(k+"N");
  const sync = (src, dst) => {
    dst.value = src.value;
    if (cur.slot) cur.slot[k] = parseInt(src.value || "0", 10);
    recalcRofTheo();
    // IP display depends on the current DP value, so refresh the stepper too.
    if (k === "dp") refreshStepper("ip");
  };
  r.addEventListener("input", ()=> sync(r, n));
  n.addEventListener("input", ()=> sync(n, r));
}

const STEPPERS = {
  is: { step: 5, min: 0, max: 600, fmt: v => v === 0 ? I[lang].off : v + " s" },
  ip: {
    step: 1, min: 0, max: 5,
    // IP is a multiplier on DP — show count and the resulting boost (in ms)
    // computed from whatever DP the user has dialled in.
    fmt: v => {
      if (v === 0) return I[lang].off;
      const dp = (cur.slot && cur.slot.dp) ? (cur.slot.dp|0) : 0;
      return v + "× (+" + (v * dp) + " ms)";
    }
  }
};

function refreshStepper(k){
  const cfg = STEPPERS[k];
  const valEl = $(k+"Val");
  if (!valEl) return;
  const v = (cur.slot && cur.slot[k] != null) ? (cur.slot[k]|0) : cfg.min;
  valEl.textContent = cfg.fmt(v);
  valEl.classList.toggle("off", v === 0);
}

function refreshStepperUi(){
  for (const k of Object.keys(STEPPERS)) refreshStepper(k);
}

function bindStepper(k){
  const cfg = STEPPERS[k];
  document.querySelectorAll('[data-step="'+k+'"]').forEach(btn=>{
    btn.addEventListener("click", ()=>{
      if (!cur.slot) return;
      const dir = parseInt(btn.dataset.dir, 10);
      const cur_v = (cur.slot[k]|0);
      const nv = Math.max(cfg.min, Math.min(cfg.max, cur_v + dir * cfg.step));
      cur.slot[k] = nv;
      refreshStepper(k);
    });
  });
}

function closeHelp(){
  if (openHelp){ openHelp.remove(); openHelp = null; }
}
function openHelpFor(btn){
  closeHelp();
  const k = btn.dataset.h;
  const txt = (I[lang].help && I[lang].help[k]) || k;
  const pop = document.createElement("div");
  pop.className = "popover";
  pop.textContent = txt;
  document.body.appendChild(pop);
  const r = btn.getBoundingClientRect();
  let left = r.left + window.scrollX;
  let top  = r.bottom + window.scrollY + 6;
  pop.style.left = "0px"; pop.style.top = "0px";
  const w = pop.offsetWidth;
  const maxLeft = window.scrollX + document.documentElement.clientWidth - w - 8;
  if (left > maxLeft) left = maxLeft;
  if (left < 8) left = 8;
  pop.style.left = left+"px";
  pop.style.top  = top+"px";
  openHelp = pop;
}
document.addEventListener("click", (e)=>{
  const t = e.target;
  if (t && t.classList && t.classList.contains("help")){
    e.stopPropagation();
    if (openHelp && openHelp._anchor === t){ closeHelp(); return; }
    openHelpFor(t);
    if (openHelp) openHelp._anchor = t;
    return;
  }
  if (openHelp && !openHelp.contains(t)) closeHelp();
});

function startSensPoll(){
  stopSensPoll();
  sensPollT = setInterval(async ()=>{
    if ($("secSens").classList.contains("hide")) return;
    try {
      const r = await jget("/halllive?ch=t");
      if (r.ok) $("sensLive").textContent = r.value;
    } catch(e){}
  }, 300);
}
function stopSensPoll(){ if (sensPollT){ clearInterval(sensPollT); sensPollT=null; } }

async function saveSensitivity(){
  try {
    const r = await jget("/halllive?ch=t");
    if (!r.ok) { toast("ERR", true); return; }
    const v = r.value|0;
    cur.slot.hallTrigHigh = v;
    if (cur.slot.hallTrigLow >= v - 20) cur.slot.hallTrigLow = Math.max(0, v - 200);
    const sv = await jpost("/save?i="+cur.i, {
      hallTrigHigh: cur.slot.hallTrigHigh,
      hallTrigLow:  cur.slot.hallTrigLow
    });
    if (sv.ok) { toast(I[lang].pointSaved + v); refreshCalUI(); }
    else toast(I[lang].saveFail, true);
  } catch(e) { toast(String(e), true); }
}

let calMode = null;
let calStep = 0;
let calBuf  = [];
let livePollT = null;

function startLivePoll(ch){
  stopLivePoll();
  livePollT = setInterval(async ()=>{
    try {
      const r = await jget("/halllive?ch="+ch);
      if (r.ok) $("liveVal").textContent = r.value;
    } catch(e){}
  }, 250);
}
function stopLivePoll(){ if (livePollT){ clearInterval(livePollT); livePollT=null; } }

function setStepDots(active, total){
  total = total || 3;
  $("step1").classList.toggle("on", active>=1);
  $("step2").classList.toggle("on", active>=2 && total>=2);
  $("step3").classList.toggle("on", active>=3 && total>=3);
  show($("step2"), total>=2);
  show($("step3"), total>=3);
}

function calClose(){ stopLivePoll(); show($("calOver"), false); calMode=null; }

function calOpenNoise(target){
  calMode = (target === "s") ? "selNoise" : "trigNoise";
  calStep = 1;
  const sec = (target === "s") ? I[lang].calSelSec : I[lang].calTrigSec;
  $("calTitle").textContent = I[lang].calNoise + " — " + sec;
  $("calMsg").textContent = I[lang].help.noise;
  $("calNext").textContent = I[lang].calBtn;
  $("liveVal").textContent = "—";
  setStepDots(1, 1);
  show($("calOver"), true);
}

function calOpenTrig(){
  calMode = "trig"; calStep = 1; calBuf = [];
  $("calTitle").textContent = I[lang].calTrig + " — " + I[lang].calTrigSec;
  $("calMsg").textContent = I[lang].calStep1;
  $("calNext").textContent = I[lang].capture;
  setStepDots(1, 3);
  show($("calOver"), true);
  startLivePoll("t");
}

function calOpenSel(){
  calMode = "sel"; calStep = 1; calBuf = [];
  $("calTitle").textContent = I[lang].calSel + " — " + I[lang].calSelSec;
  $("calMsg").textContent = I[lang].calStepSel + " 1.";
  $("calNext").textContent = I[lang].capture;
  setStepDots(1, 3);
  show($("calOver"), true);
  startLivePoll("s");
}

async function calNextNoise(){
  const isSel = (calMode === "selNoise");
  const ch    = isSel ? "s" : "t";
  $("calMsg").textContent = I[lang].noiseRunning;
  $("calNext").disabled = true;
  try {
    const r = await jpost("/noisecal", {ch:ch});
    if (r.ok) {
      const m = r.margin|0;
      if (isSel) {
        noiseMarginSel = m;
        $("calSelNoiseVals").textContent = "± " + m;
      } else {
        noiseMargin = m;
        $("calTrigNoiseVals").textContent = "± " + m;
      }
      $("calMsg").textContent = I[lang].noiseOk + m;
      $("calNext").textContent = I[lang].cancel;
      $("calNext").onclick = calClose;
    } else {
      toast(r.error||"err", true);
    }
  } catch(e){ toast(String(e), true); }
  finally { $("calNext").disabled = false; }
}

function calNextTrig(){
  const v = parseInt($("liveVal").textContent||"0",10);
  if (calStep===1){
    calBuf[0] = v;
    calStep = 2;
    setStepDots(2, 3);
    $("calMsg").textContent = I[lang].calStep2;
  } else if (calStep===2){
    calBuf[1] = v;
    calStep = 3;
    setStepDots(3, 3);
    let lo = Math.min(calBuf[0], calBuf[1]);
    let hi = Math.max(calBuf[0], calBuf[1]);
    const margin = Math.max(noiseMargin, 50);
    let lo2 = lo + margin;
    let hi2 = hi - margin;
    if (lo2 >= hi2) {
      $("calMsg").textContent = I[lang].calRangeErr;
      return;
    }
    cur.slot.hallTrigLow  = lo2;
    cur.slot.hallTrigHigh = hi2;
    $("calMsg").textContent = I[lang].calStep3 + " (low="+lo2+" high="+hi2+")";
    $("calNext").textContent = I[lang].saveCal;
  } else {
    jpost("/save?i="+cur.i, {hallTrigLow:cur.slot.hallTrigLow, hallTrigHigh:cur.slot.hallTrigHigh})
      .then(r=>{ toast(r.ok?I[lang].saved:I[lang].saveFail, !r.ok); refreshCalUI(); calClose(); });
  }
}

function calNextSel(){
  const v = parseInt($("liveVal").textContent||"0",10);
  if (calStep <= 3){
    calBuf[calStep-1] = v;
    calStep++;
    if (calStep <= 3){
      setStepDots(calStep, 3);
      $("calMsg").textContent = I[lang].calStepSel + " " + calStep + ".";
      return;
    }
    const sorted = calBuf.slice(0,3).sort((a,b)=>a-b);
    cur.slot.hallSelLow1 = Math.round((sorted[0]+sorted[1])/2);
    cur.slot.hallSelLow2 = Math.round((sorted[1]+sorted[2])/2);
    $("calMsg").textContent = I[lang].calStep3 + " (low1="+cur.slot.hallSelLow1+" low2="+cur.slot.hallSelLow2+")";
    $("calNext").textContent = I[lang].saveCal;
  } else {
    jpost("/save?i="+cur.i, {
      hallSelLow1: cur.slot.hallSelLow1,
      hallSelLow2: cur.slot.hallSelLow2
    }).then(r=>{ toast(r.ok?I[lang].saved:I[lang].saveFail, !r.ok); refreshCalUI(); calClose(); });
  }
}

function calNext(){
  if (calMode === "trigNoise" || calMode === "selNoise") return calNextNoise();
  if (calMode === "trig") return calNextTrig();
  if (calMode === "sel")  return calNextSel();
}

let trigTestT = null;
let trigTestLastEvt = 0;
let trigTestLastActivity = 0;
function trigTestStop(){
  if (trigTestT){ clearInterval(trigTestT); trigTestT = null; }
  $("trigTestBtn").textContent = I[lang].trigTestStart;
  $("trigTestDot").classList.remove("on");
  $("trigTestMsg").textContent = I[lang].trigTestDone;
}
function trigTestStart(){
  if (trigTestT) return;
  trigTestLastEvt = 0;
  trigTestLastActivity = Date.now();
  $("trigTestBtn").textContent = I[lang].trigTestStop;
  $("trigTestMsg").textContent = "…";
  trigTestT = setInterval(async ()=>{
    try {
      const r = await jget("/trigstate");
      if (!r.ok) return;
      $("trigTestDot").classList.toggle("on", !!r.pressed);
      if (r.events !== trigTestLastEvt){
        trigTestLastEvt = r.events;
        trigTestLastActivity = Date.now();
      }
      if (Date.now() - trigTestLastActivity > 15000){
        trigTestStop();
      }
    } catch(e){}
  }, 150);
}

window.addEventListener("DOMContentLoaded", async ()=>{
  ["dn","dr","dp","db"].forEach(bindPair);
  ["is","ip"].forEach(bindStepper);

  $("langBR").addEventListener("click", ()=>{ lang="br"; jpost("/setlang",{lang:"br"}); applyLang(); refreshSelectorUI(); });
  $("langEN").addEventListener("click", ()=>{ lang="en"; jpost("/setlang",{lang:"en"}); applyLang(); refreshSelectorUI(); });

  $("solS").addEventListener("click", ()=>{ if(!cur.slot) return; cur.slot.solenoids=1; refreshSolenoidUI(); });
  $("solD").addEventListener("click", ()=>{ if(!cur.slot) return; cur.slot.solenoids=2; refreshSolenoidUI(); });

  $("trigMode").addEventListener("change", e=>{ if(!cur.slot) return; cur.slot.trigMode = parseInt(e.target.value,10); refreshCalUI(); });
  $("selMode").addEventListener("change", e=>{
    if(!cur.slot) return;
    cur.slot.selMode = parseInt(e.target.value,10);
    refreshCalUI();
  });
  $("sel3pos").addEventListener("change", e=>{
    if(!cur.slot) return;
    cur.slot.sel3pos = e.target.checked ? 1 : 0;
    if (cur.slot.sel3pos) cur.slot.selMode = 1;
    refreshSelectorUI();
  });

  $("btnSave").addEventListener("click", saveSlot);
  $("btnReset").addEventListener("click", resetSlot);
  $("btnReload").addEventListener("click", async ()=>{ await loadMeta(); await loadSlot(cur.i); toast(I[lang].reloaded); });

  $("buzzPlay").addEventListener("click", async ()=>{
    const v = parseInt($("buzzPick").value,10);
    const r = await jpost("/test", {buzz:v});
    if (r.ok) toast(I[lang].buzzed);
  });
  $("mos1Btn").addEventListener("click", async ()=>{
    const r = await jpost("/test", {mos:1});
    toast(r.ok ? I[lang].mosOk : (I[lang].mosFail+": "+(r.error||"")), !r.ok);
  });
  $("mos2Btn").addEventListener("click", async ()=>{
    const r = await jpost("/test", {mos:2});
    toast(r.ok ? I[lang].mosOk : (I[lang].mosFail+": "+(r.error||"")), !r.ok);
  });
  $("trigTestBtn").addEventListener("click", ()=>{
    if (trigTestT) trigTestStop(); else trigTestStart();
  });

  $("setPwdBtn").addEventListener("click", async ()=>{
    const p = $("wifiPwd").value || "";
    if (p.length < 8) { toast(I[lang].pwdBad, true); return; }
    const r = await jpost("/setpwd", {pwd:p});
    toast(r.ok ? I[lang].pwdOk : (r.error||"err"), !r.ok);
    if (r.ok) $("wifiPwd").value = "";
  });

  $("sensSaveBtn").addEventListener("click", saveSensitivity);

  $("cfgLockSetBtn").addEventListener("click", async ()=>{
    const pwd = $("cfgLockPwd").value || "";
    if (pwd.length < 4) { toast(I[lang].cfgLockTooShort, true); return; }
    const r = await jpost("/cfglock", {action:"set", pwd});
    if (r.ok){
      $("cfgLockPwd").value = "";
      cur.meta.cfgLock = {hasPwd:r.hasPwd, locked:r.locked};
      updateLockUi();
      toast(I[lang].cfgLockOk);
    } else toast(r.error||"err", true);
  });
  $("cfgLockLockBtn").addEventListener("click", async ()=>{
    const r = await jpost("/cfglock", {action:"lock"});
    if (r.ok){
      cur.meta.cfgLock = {hasPwd:r.hasPwd, locked:r.locked};
      updateLockUi();
    } else toast(r.error||"err", true);
  });
  $("cfgLockClearBtn").addEventListener("click", async ()=>{
    if (!confirm(I[lang].cfgLockClearConfirm)) return;
    const r = await jpost("/cfglock", {action:"clear"});
    if (r.ok){
      cur.meta.cfgLock = {hasPwd:r.hasPwd, locked:r.locked};
      updateLockUi();
      toast(I[lang].cfgLockCleared);
    } else toast(r.error||"err", true);
  });
  $("lockUnlockBtn").addEventListener("click", async ()=>{
    const pwd = $("lockPwdInput").value || "";
    const r = await jpost("/cfglock", {action:"unlock", pwd});
    if (r.ok && !r.locked){
      $("lockPwdInput").value = "";
      cur.meta.cfgLock = {hasPwd:r.hasPwd, locked:r.locked};
      updateLockUi();
      toast(I[lang].cfgLockUnlocked);
      try { await loadSlot(cur.i); } catch(e){}
    } else toast(I[lang].cfgLockBadPwd, true);
  });

  $("calTrigNoiseBtn").addEventListener("click", ()=>calOpenNoise("t"));
  $("calSelNoiseBtn").addEventListener("click",  ()=>calOpenNoise("s"));
  $("calTrigBtn").addEventListener("click", calOpenTrig);
  $("calSelBtn").addEventListener("click", calOpenSel);
  $("calCancel").addEventListener("click", calClose);
  $("calNext").addEventListener("click", ()=>{
    if (calNext._suppress) { calNext._suppress = false; return; }
    calNext();
  });

  await loadMeta();
  await loadSlot(cur.i);

  // Periodic /load poll keeps stats (shots, battery on PRO) live without a manual reload.
  // PRO refreshes at 5 s for tighter battery readout; STR can use 10 s since it only needs shots.
  const pollMs = IS_PRO ? 5000 : 10000;
  setInterval(async ()=>{ try { cur.meta = await jget("/load"); updateHeader(); } catch(e){} }, pollMs);
});
