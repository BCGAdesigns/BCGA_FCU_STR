// BCGA FCU V2 — web_ui.h
// Single-page dashboard served on GET /. Stored in PROGMEM as raw string.
//
// Section order:
//   - Header: gear logo, name, S8PA/D8PA tag, battery chip, BR/EN
//   - Slot picker: 3 buttons at the top (always clickable)
//   - 1) Tipo de disparo (S8PA/D8PA) — help texts: Jack/Backdraft vs F2/Pulsar
//   - 2) Timings (DN/DR/DP/DL em ms; DN+DL somem em S8PA) + ROF teórico
//   - 3) Seletor (Pos1/Pos2; toggle 3-pos auto-ativa Hall e revela Pos3)
//   - 4) Entrada (Sw/Hall do gatilho, inverter gatilho, swap MOS, silent)
//   - 5) Sensibilidade (só Hall trigger): captura de ponto único
//   - 6) Calibração do gatilho (só Hall trigger): ruído + solto/pressionado
//   - 7) Calibração do seletor (só Hall sel): ruído + 3 posições
//   - 8) Diagnóstico (buzzer + Test MOS — 2s, respeita mosfetSwap)
//   - 9) WiFi (trocar senha)
// Toolbar: Resetar slot · Recarregar · Salvar.
// Footer: link bcgaairsoft.com.br + versão + variante.

#pragma once

#include <Arduino.h>

const char WEB_UI_HTML[] PROGMEM = R"HTML(<!doctype html>
<html lang="pt-BR">
<head>
<meta charset="utf-8" />
<meta name="viewport" content="width=device-width,initial-scale=1,viewport-fit=cover" />
<title>BCGA FCU</title>
<style>
:root{--bg:#0b0f1a;--panel:#161b22;--panel2:#1c2230;--text:#e6edf3;--mute:#8b95a5;--accent:#00bcd4;--accent2:#089aaa;--ok:#10b981;--warn:#f59e0b;--err:#ef4444;--line:#243044}
*{box-sizing:border-box}
html,body{margin:0;padding:0;background:var(--bg);color:var(--text);font:14px/1.4 -apple-system,Segoe UI,Roboto,Helvetica,Arial,sans-serif}
header{position:sticky;top:0;background:linear-gradient(180deg,#0b0f1a 70%,rgba(11,15,26,.85));padding:10px 14px;border-bottom:1px solid var(--line);z-index:5;display:flex;align-items:center;gap:10px;flex-wrap:wrap}
header .brand{display:flex;align-items:center;gap:10px;flex:1;min-width:0}
header .brand svg{flex:0 0 auto}
header .brand .txt{display:flex;flex-direction:column;line-height:1.1;min-width:0}
header .brand .name{font-size:14px;font-weight:700;color:#fff;letter-spacing:.5px}
header .brand .sub{font-size:11px;color:var(--mute)}
.battChip{display:inline-flex;align-items:center;gap:6px;padding:4px 10px;border-radius:99px;font-size:12px;font-weight:600;background:var(--panel2);border:1px solid var(--line);color:var(--mute)}
.battChip.ok{background:rgba(16,185,129,.15);color:var(--ok);border-color:var(--ok)}
.battChip.warn{background:rgba(245,158,11,.15);color:var(--warn);border-color:var(--warn)}
.battChip.err{background:rgba(239,68,68,.15);color:var(--err);border-color:var(--err)}
.battChip.cut{text-decoration:line-through;opacity:.6}
.lang{display:flex;gap:6px}
.lang button{background:var(--panel2);border:1px solid var(--line);color:var(--mute);padding:4px 10px;border-radius:99px;cursor:pointer;font:inherit}
.lang button.on{color:var(--accent);border-color:var(--accent)}
main{padding:14px;max-width:760px;margin:0 auto;padding-bottom:100px}
.slotPick{display:flex;gap:8px;justify-content:center;margin:0 0 12px}
.slot-btn{flex:0 0 auto;width:60px;height:48px;border-radius:10px;border:1px solid var(--line);background:var(--panel);color:var(--mute);font-weight:700;font-size:16px;cursor:pointer;font:inherit}
.slot-btn.on{background:var(--accent);color:#001318;border-color:var(--accent);box-shadow:0 0 0 2px rgba(0,188,212,.25)}
section{background:var(--panel);border:1px solid var(--line);border-radius:12px;padding:12px 14px 10px;margin-bottom:12px}
section h2{margin:0 0 10px;font-size:13px;text-transform:uppercase;letter-spacing:1px;color:var(--accent);display:flex;align-items:center;gap:10px}
section h2 .num{display:inline-flex;align-items:center;justify-content:center;width:22px;height:22px;border-radius:50%;background:var(--accent);color:#001318;font-weight:800;font-size:12px}
.row{display:flex;align-items:center;gap:10px;margin:8px 0;flex-wrap:wrap}
.row label{flex:1;min-width:120px;color:var(--mute);display:flex;align-items:center;gap:6px}
.row .ctrl{flex:2;min-width:160px;display:flex;align-items:center;gap:8px}
input[type=text],input[type=number],input[type=password],select{background:var(--panel2);color:var(--text);border:1px solid var(--line);border-radius:6px;padding:8px 10px;font:inherit;width:100%}
input[type=range]{width:100%;accent-color:var(--accent)}
.btn{background:var(--accent);color:#001318;border:none;padding:10px 14px;border-radius:8px;font-weight:600;cursor:pointer;font:inherit}
.btn:hover{background:var(--accent2)}
.btn.ghost{background:transparent;color:var(--accent);border:1px solid var(--accent)}
.btn.danger{background:var(--err);color:#fff}
.btn.warn{background:transparent;color:var(--warn);border:1px solid var(--warn)}
.btn.sm{padding:6px 10px;font-size:13px}
.pair{display:grid;grid-template-columns:1fr 90px;gap:8px;align-items:center}
.toolbar{position:fixed;left:0;right:0;bottom:0;padding:10px 14px;background:rgba(11,15,26,.95);border-top:1px solid var(--line);display:flex;gap:10px;justify-content:space-between;align-items:center;z-index:4;flex-wrap:wrap}
.toolbar .grp{display:flex;gap:10px}
.toast{position:fixed;left:50%;bottom:90px;transform:translateX(-50%);background:var(--ok);color:#001a10;padding:8px 14px;border-radius:99px;font-weight:600;opacity:0;transition:opacity .25s;pointer-events:none;z-index:10;max-width:90%}
.toast.show{opacity:1}
.toast.err{background:var(--err);color:#fff}
.muted{color:var(--mute);font-size:12px}
.divider{height:1px;background:var(--line);margin:10px 0}
.radio-row{display:flex;gap:10px;flex-wrap:wrap}
.radio-card{flex:1 1 140px;background:var(--panel2);border:1px solid var(--line);border-radius:10px;padding:10px;cursor:pointer;text-align:center;font-weight:600}
.radio-card.on{border-color:var(--accent);color:var(--accent);box-shadow:0 0 0 1px var(--accent) inset}
.radio-card .small{display:block;font-size:11px;color:var(--mute);margin-top:4px;font-weight:400}
.hide{display:none!important}
.rofTheoBox{display:flex;align-items:center;justify-content:space-between;padding:8px 10px;border:1px dashed var(--line);border-radius:8px;background:var(--panel2);margin:6px 0 0}
.rofTheoBox .v{font-weight:700;color:var(--accent);font-variant-numeric:tabular-nums}
footer{padding:18px 12px 24px;text-align:center;color:var(--mute);font-size:12px}
footer a{color:var(--accent);text-decoration:none}
.help{display:inline-flex;align-items:center;justify-content:center;width:18px;height:18px;border-radius:50%;background:var(--panel2);border:1px solid var(--line);color:var(--mute);font-size:11px;font-weight:700;cursor:pointer;padding:0;font:inherit}
.help:hover{color:var(--accent);border-color:var(--accent)}
.popover{position:absolute;background:var(--panel2);border:1px solid var(--accent);border-radius:8px;padding:8px 10px;font-size:12px;color:var(--text);max-width:260px;z-index:30;box-shadow:0 4px 12px rgba(0,0,0,.4)}
.overlay{position:fixed;inset:0;background:rgba(0,0,0,.7);display:flex;align-items:center;justify-content:center;z-index:20;padding:14px}
.overlay .box{background:var(--panel);border:1px solid var(--line);border-radius:12px;padding:18px;max-width:420px;width:100%}
.overlay h3{margin:0 0 8px;color:var(--accent);font-size:16px}
.live{font-size:42px;font-weight:800;text-align:center;margin:14px 0;color:var(--accent);font-variant-numeric:tabular-nums}
.live .lbl{display:block;font-size:11px;color:var(--mute);font-weight:400;text-transform:uppercase;letter-spacing:1px}
.steps{display:flex;gap:6px;justify-content:center;margin:8px 0 12px}
.dot{width:8px;height:8px;border-radius:50%;background:var(--line)}
.dot.on{background:var(--accent)}
.toggle{display:flex;align-items:center;gap:8px}
</style>
</head>
<body>

<header>
  <div class="brand">
    <svg width="32" height="32" viewBox="0 0 64 64" aria-hidden="true">
      <path fill="#fff" fill-rule="evenodd" d="M28 2 L36 2 L37 9 L43 11 L48 6 L54 12 L49 17 L51 23 L58 24 L58 32 L51 33 L49 39 L54 44 L48 50 L43 45 L37 47 L36 54 L28 54 L27 47 L21 45 L16 50 L10 44 L15 39 L13 33 L6 32 L6 24 L13 23 L15 17 L10 12 L16 6 L21 11 L27 9 Z M32 18 a10 10 0 1 0 0.01 0 Z"/>
    </svg>
    <div class="txt">
      <div class="name" id="hdrName">BCGA FCU</div>
      <div class="sub" id="hdrSub">— • —</div>
    </div>
  </div>
  <div id="battChip" class="battChip hide"><span id="battTxt">—</span></div>
  <div class="lang">
    <button id="langBR" class="on" type="button">BR</button>
    <button id="langEN" type="button">EN</button>
  </div>
</header>

<main>

  <div class="slotPick" id="slotPick"></div>

  <section>
    <h2><span class="num">1</span><span data-i="solSec">Tipo de disparo</span></h2>
    <div class="row">
      <label data-i="slotName">Nome do slot</label>
      <div class="ctrl"><input type="text" id="slotName" maxlength="15" /></div>
    </div>
    <div class="radio-row">
      <div class="radio-card" id="solS"><div>S8PA</div><span class="small" data-i="solS">1 solenoide (Jack, Backdraft)</span></div>
      <div class="radio-card" id="solD"><div>D8PA</div><span class="small" data-i="solD">2 solenoides (F2, Pulsar)</span></div>
    </div>
  </section>

  <section>
    <h2><span class="num">2</span><span data-i="timSec">Timings (ms)</span></h2>
    <div class="row" id="rowDn">
      <label>DN <button class="help" data-h="dn" type="button">?</button></label>
      <div class="ctrl"><div class="pair" style="width:100%"><input type="range" id="dnR" min="2" max="80" /><input type="number" id="dnN" min="2" max="80" /></div></div>
    </div>
    <div class="row" id="rowDr">
      <label>DR <button class="help" data-h="dr" type="button">?</button></label>
      <div class="ctrl"><div class="pair" style="width:100%"><input type="range" id="drR" min="2" max="80" /><input type="number" id="drN" min="2" max="80" /></div></div>
    </div>
    <div class="row" id="rowDp">
      <label>DP <button class="help" data-h="dp" type="button">?</button></label>
      <div class="ctrl"><div class="pair" style="width:100%"><input type="range" id="dpR" min="2" max="80" /><input type="number" id="dpN" min="2" max="80" /></div></div>
    </div>
    <div class="row" id="rowDl">
      <label>DL <button class="help" data-h="dl" type="button">?</button></label>
      <div class="ctrl"><div class="pair" style="width:100%"><input type="range" id="dlR" min="2" max="80" /><input type="number" id="dlN" min="2" max="80" /></div></div>
    </div>
    <div class="row">
      <label data-i="rofLbl">ROF máx (rps) <button class="help" data-h="rof" type="button">?</button></label>
      <div class="ctrl"><input type="number" id="rofN" min="0" max="50" /></div>
    </div>
    <div class="rofTheoBox">
      <span data-i="rofTheoLbl">ROF teórico <button class="help" data-h="rofTheo" type="button">?</button></span>
      <span class="v" id="rofTheoVal">— rps</span>
    </div>
  </section>

  <section>
    <h2><span class="num">3</span><span data-i="selSec">Seletor</span></h2>
    <div class="row"><label data-i="pos1">Posição 1 <button class="help" data-h="pos" type="button">?</button></label><div class="ctrl"><select id="selPos1"></select></div></div>
    <div class="row"><label data-i="pos2">Posição 2 <button class="help" data-h="pos" type="button">?</button></label><div class="ctrl"><select id="selPos2"></select></div></div>
    <div class="row">
      <label data-i="sel3">Seletor de 3 posições <button class="help" data-h="sel3" type="button">?</button></label>
      <div class="ctrl"><label class="toggle"><input type="checkbox" id="sel3pos" /> <span data-i="sel3H">habilitar Pos 3 (ativa Hall)</span></label></div>
    </div>
    <div class="row" id="rowPos3"><label data-i="pos3">Posição 3</label><div class="ctrl"><select id="selPos3"></select></div></div>
  </section>

  <section>
    <h2><span class="num">4</span><span data-i="inSec">Entrada</span></h2>
    <div class="row"><label data-i="trigIn">Gatilho <button class="help" data-h="trigMode" type="button">?</button></label><div class="ctrl"><select id="trigMode"><option value="0" data-i="sw">Microswitch</option><option value="1" data-i="hl">Hall (analog)</option></select></div></div>
    <div class="row"><label data-i="invTrig">Inverter gatilho</label><div class="ctrl"><label class="toggle"><input type="checkbox" id="invertTrig" /></label></div></div>
    <div class="row" id="rowMosSwap"><label data-i="mosSwap">Trocar MOS 1 ↔ 2 <button class="help" data-h="mosSwap" type="button">?</button></label><div class="ctrl"><label class="toggle"><input type="checkbox" id="mosfetSwap" /> <span data-i="mosSwapH">corrigir fiação trocada</span></label></div></div>
    <div class="row"><label data-i="silent">Modo silencioso <button class="help" data-h="silent" type="button">?</button></label><div class="ctrl"><label class="toggle"><input type="checkbox" id="silent" /> <span data-i="silentH">não tocar buzzer no uso</span></label></div></div>
  </section>

  <section id="secSens" class="hide">
    <h2><span class="num">5</span><span data-i="sensSec">Sensibilidade do gatilho</span> <button class="help" data-h="sens" type="button">?</button></h2>
    <div class="muted" data-i="sensHelp">Puxe o gatilho até onde quer que dispare e clique em Salvar.</div>
    <div class="live"><span class="lbl" data-i="liveAdc">Leitura ADC</span><span id="sensLive">0</span></div>
    <div class="row">
      <label data-i="curPoint">Ponto atual</label>
      <div class="ctrl"><span class="muted" id="curPoint">—</span></div>
    </div>
    <div class="row">
      <div></div>
      <div class="ctrl"><button class="btn" id="sensSaveBtn" type="button" data-i="savePoint">Salvar ponto</button></div>
    </div>
  </section>

  <section id="secCalTrig" class="hide">
    <h2><span class="num">6</span><span data-i="calTrigSec">Calibração do gatilho</span></h2>
    <div class="muted" data-i="calTrigHelp">Comece pelo ruído (faz 1 disparo de teste). Não puxe o gatilho.</div>
    <div class="row" style="margin-top:10px">
      <label data-i="calNoise">Ruído <button class="help" data-h="noise" type="button">?</button></label>
      <div class="ctrl"><button class="btn ghost sm" id="calTrigNoiseBtn" type="button" data-i="calBtn">Calibrar</button>
      <span class="muted" id="calTrigNoiseVals">±—</span></div>
    </div>
    <div class="row">
      <label data-i="calTrig">Gatilho <button class="help" data-h="cal" type="button">?</button></label>
      <div class="ctrl"><button class="btn ghost sm" id="calTrigBtn" type="button" data-i="calBtn">Calibrar</button>
      <span class="muted" id="calTrigVals">low=— high=—</span></div>
    </div>
  </section>

  <section id="secCalSel" class="hide">
    <h2><span class="num">7</span><span data-i="calSelSec">Calibração do seletor</span></h2>
    <div class="muted" data-i="calSelHelp">Comece pelo ruído (faz 1 disparo de teste). Não toque no seletor.</div>
    <div class="row" style="margin-top:10px">
      <label data-i="calNoise">Ruído <button class="help" data-h="noise" type="button">?</button></label>
      <div class="ctrl"><button class="btn ghost sm" id="calSelNoiseBtn" type="button" data-i="calBtn">Calibrar</button>
      <span class="muted" id="calSelNoiseVals">±—</span></div>
    </div>
    <div class="row">
      <label data-i="calSel">Seletor (3 pos) <button class="help" data-h="cal" type="button">?</button></label>
      <div class="ctrl"><button class="btn ghost sm" id="calSelBtn" type="button" data-i="calBtn">Calibrar</button>
      <span class="muted" id="calSelVals">—</span></div>
    </div>
  </section>

  <section>
    <h2><span class="num">8</span><span data-i="diagSec">Diagnóstico</span></h2>
    <div class="row">
      <label data-i="buzz">Buzzer <button class="help" data-h="buzz" type="button">?</button></label>
      <div class="ctrl">
        <select id="buzzPick">
          <option value="1">Boot</option>
          <option value="2" data-i="bzMode">Trocar modo</option>
          <option value="3" data-i="bzSave">Salvar OK</option>
          <option value="4" data-i="bzWifi">WiFi ON</option>
          <option value="5" data-i="bzLow">Bateria baixa</option>
          <option value="6" data-i="bzCut">Bateria CUT</option>
          <option value="7" data-i="bzTest">Teste</option>
        </select>
        <button class="btn sm" id="buzzPlay" type="button" data-i="play">Tocar</button>
      </div>
    </div>
    <div class="row">
      <label data-i="mosTest">Teste MOSFET (2s) <button class="help" data-h="mosTest" type="button">?</button></label>
      <div class="ctrl">
        <button class="btn danger sm" id="mos1Btn" type="button">SOL 1 (Poppet)</button>
        <button class="btn danger sm" id="mos2Btn" type="button">SOL 2 (Nozzle)</button>
      </div>
    </div>
  </section>

  <section>
    <h2><span class="num">9</span><span data-i="wifiSec">WiFi</span></h2>
    <div class="row">
      <label data-i="newPwd">Nova senha <button class="help" data-h="pwd" type="button">?</button></label>
      <div class="ctrl"><input type="password" id="wifiPwd" minlength="8" maxlength="32" placeholder="********" /></div>
    </div>
    <div class="row"><div></div><div class="ctrl"><button class="btn ghost" id="setPwdBtn" type="button" data-i="changePwd">Trocar senha</button></div></div>
  </section>

</main>

<div class="toolbar">
  <button class="btn warn" id="btnReset" type="button" data-i="resetSlot">Resetar slot</button>
  <div class="grp">
    <button class="btn ghost" id="btnReload" type="button" data-i="reload">Recarregar</button>
    <button class="btn" id="btnSave" type="button" data-i="save">Salvar slot</button>
  </div>
</div>

<footer>
  <a href="https://bcgaairsoft.com.br" target="_blank" rel="noopener">bcgaairsoft.com.br</a>
  &nbsp;•&nbsp; <span id="ftVer">v—</span>
  &nbsp;•&nbsp; <span id="ftVar">—</span>
</footer>

<div id="toast" class="toast">OK</div>

<div id="calOver" class="overlay hide">
  <div class="box">
    <h3 id="calTitle">Calibração</h3>
    <div class="steps"><div class="dot on" id="step1"></div><div class="dot" id="step2"></div><div class="dot" id="step3"></div></div>
    <div class="muted" id="calMsg">—</div>
    <div class="live"><span class="lbl" data-i="liveAdc">Leitura ADC</span><span id="liveVal">0</span></div>
    <div style="display:flex;gap:8px;justify-content:flex-end;flex-wrap:wrap">
      <button class="btn ghost" id="calCancel" type="button" data-i="cancel">Cancelar</button>
      <button class="btn" id="calNext" type="button" data-i="capture">Capturar</button>
    </div>
  </div>
</div>

<script>
"use strict";
const I = {
  br:{
    slotName:"Nome do slot",
    solSec:"Tipo de disparo",solS:"1 solenoide (Jack, Backdraft)",solD:"2 solenoides (F2, Pulsar)",
    timSec:"Timings (ms)",rofLbl:"ROF máx (rps)",rofTheoLbl:"ROF teórico",
    selSec:"Seletor",pos1:"Posição 1",pos2:"Posição 2",pos3:"Posição 3",
    sel3:"Seletor de 3 posições",sel3H:"habilitar Pos 3 (ativa Hall)",
    inSec:"Entrada",trigIn:"Gatilho",sw:"Microswitch",hl:"Hall (analog)",
    invTrig:"Inverter gatilho",
    mosSwap:"Trocar MOS 1 ↔ 2",mosSwapH:"corrigir fiação trocada",
    silent:"Modo silencioso",silentH:"não tocar buzzer no uso",
    sensSec:"Sensibilidade do gatilho",sensHelp:"Puxe o gatilho até onde quer que dispare e clique em Salvar.",
    curPoint:"Ponto atual",savePoint:"Salvar ponto",liveAdc:"Leitura ADC",
    calTrigSec:"Calibração do gatilho",calTrigHelp:"Comece pelo ruído (faz 1 disparo de teste). Não puxe o gatilho.",
    calSelSec:"Calibração do seletor",calSelHelp:"Comece pelo ruído (faz 1 disparo de teste). Não toque no seletor.",
    calNoise:"Ruído",calTrig:"Gatilho",calSel:"Seletor (3 pos)",calBtn:"Calibrar",
    diagSec:"Diagnóstico",buzz:"Buzzer",bzMode:"Trocar modo",bzSave:"Salvar OK",bzWifi:"WiFi ON",bzLow:"Bateria baixa",bzCut:"Bateria CUT",bzTest:"Teste",play:"Tocar",mosTest:"Teste MOSFET (2s)",
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
      dl:"Delay (ms) após o disparo, aguardando a BB sair do cano antes do próximo tiro. Só D8PA. Padrão 10 ms.",
      rof:"Limite de cadência em tiros por segundo (rps). 0 = sem limite (cadência só pelos timings).",
      rofTheo:"Cadência teórica em rps calculada a partir dos timings: 1000 ÷ (soma dos delays).",
      pos:"Modo de fogo quando o seletor está nesta posição.",
      sel3:"Habilita uma terceira posição do seletor. Requer sensor Hall (a opção de 3 stops digital não existe). Ao ligar, o gatilho do seletor passa automaticamente para Hall.",
      trigMode:"Tipo do sensor do gatilho: microswitch digital ou Hall analógico.",
      mosSwap:"Se você soldou nozzle e poppet nos pinos trocados, marque para corrigir via software (D8PA).",
      silent:"Não toca confirmação do buzzer durante o uso normal.",
      sens:"Puxe o gatilho até o ponto onde quer disparar e clique Salvar — o ADC vira o ponto de disparo.",
      noise:"Faz 1 disparo completo medindo o ruído elétrico no Hall. Use ANTES de calibrar gatilho para definir a margem da deadband.",
      cal:"Captura os valores ADC do gatilho/seletor solto e pressionado.",
      mosTest:"Pulsa o MOSFET por 2 segundos. Use só para conferir a fiação — sem gas/munição.",
      buzz:"Toca um padrão de aviso. Útil para testar o buzzer.",
      pwd:"Senha do AP WiFi. Mínimo 8 caracteres."
    }
  },
  en:{
    slotName:"Slot name",
    solSec:"Firing type",solS:"1 solenoid (Jack, Backdraft)",solD:"2 solenoids (F2, Pulsar)",
    timSec:"Timings (ms)",rofLbl:"ROF cap (rps)",rofTheoLbl:"Theoretical ROF",
    selSec:"Selector",pos1:"Position 1",pos2:"Position 2",pos3:"Position 3",
    sel3:"3-position selector",sel3H:"enable Pos 3 (auto-Hall)",
    inSec:"Input",trigIn:"Trigger",sw:"Microswitch",hl:"Hall (analog)",
    invTrig:"Invert trigger",
    mosSwap:"Swap MOS 1 ↔ 2",mosSwapH:"fix swapped wiring",
    silent:"Silent mode",silentH:"skip buzzer while in use",
    sensSec:"Trigger sensitivity",sensHelp:"Pull trigger to the desired fire point and click Save.",
    curPoint:"Current point",savePoint:"Save point",liveAdc:"ADC reading",
    calTrigSec:"Trigger calibration",calTrigHelp:"Start with noise (fires 1 test cycle). Don't pull the trigger.",
    calSelSec:"Selector calibration",calSelHelp:"Start with noise (fires 1 test cycle). Don't touch the selector.",
    calNoise:"Noise",calTrig:"Trigger",calSel:"Selector (3 pos)",calBtn:"Calibrate",
    diagSec:"Diagnostics",buzz:"Buzzer",bzMode:"Mode change",bzSave:"Save OK",bzWifi:"WiFi ON",bzLow:"Low battery",bzCut:"Battery CUT",bzTest:"Test",play:"Play",mosTest:"MOSFET test (2s)",
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
      dl:"Delay (ms) after the shot, waiting for the BB to exit the barrel before the next shot. D8PA only. Default 10 ms.",
      rof:"Rate cap in rounds per second (rps). 0 = unlimited (cadence ruled by timings only).",
      rofTheo:"Theoretical cadence in rps from timings: 1000 ÷ (sum of delays).",
      pos:"Fire mode when the selector is in this position.",
      sel3:"Enables a third selector position. Requires Hall sensor (no 3-stop digital). Toggling on auto-switches the selector input to Hall.",
      trigMode:"Trigger sensor type: digital microswitch or analog Hall.",
      mosSwap:"If you soldered nozzle/poppet to the wrong pins, check this to fix in software (D8PA).",
      silent:"Skips buzzer confirmations during normal use.",
      sens:"Pull trigger to the point where you want it to fire and click Save — that ADC becomes the fire point.",
      noise:"Fires 1 full cycle while sampling the Hall ADC. Run BEFORE calibrating the trigger to set the deadband margin.",
      cal:"Captures ADC values for released/pressed trigger or each selector position.",
      mosTest:"Pulses the MOSFET for 2 seconds. Use only to check wiring — no gas/ammo.",
      buzz:"Plays a warning pattern. Useful to test the buzzer.",
      pwd:"WiFi AP password. Minimum 8 characters."
    }
  }
};
let lang = "br";
let cur = { i:0, slot:null, meta:null };
let noiseMargin    = 0;    // last captured trigger noise margin (ADC counts)
let noiseMarginSel = 0;    // last captured selector noise margin (ADC counts)
let openHelp = null;       // currently open help popover element
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

function updateBatteryChip(){
  const chip = $("battChip");
  if (!cur.meta || !cur.meta.pro || !cur.meta.batt){
    show(chip, false);
    return;
  }
  show(chip, true);
  const b = cur.meta.batt;
  const mv = b.mv|0;
  const cells = b.cells|0;
  let txt = (mv/1000).toFixed(2)+"V";
  if (cells) txt += " ("+cells+"S)";
  chip.classList.remove("ok","warn","err","cut");
  if (b.cut) {
    chip.classList.add("cut","err");
    txt += " CUT";
  } else if (b.low) {
    chip.classList.add("err");
  } else if (cells > 0) {
    const perCell = mv / cells;
    if (perCell >= 3700)      chip.classList.add("ok");
    else if (perCell >= 3500) chip.classList.add("warn");
    else                       chip.classList.add("err");
  }
  $("battTxt").textContent = txt;
}

function updateHeader(){
  if (!cur.meta) return;
  const variant = cur.meta.variant || "—";
  const ssid = cur.meta.ssid || ("BCGA_FCU_"+(variant==="Pro"?"PRO":"STR"));
  const sol = (cur.slot && cur.slot.solenoids===1) ? "S8PA" : "D8PA";
  $("hdrName").textContent = ssid.replace(/_/g," ");
  $("hdrSub").textContent = "FCU "+variant+" • "+sol;
  $("ftVer").textContent = "v"+(cur.meta.fw||"");
  $("ftVar").textContent = variant;
  updateBatteryChip();
}

function recalcRofTheo(){
  if (!cur.slot) { $("rofTheoVal").textContent = "— rps"; return; }
  const pair = (k)=> Math.max(2, Math.min(80, parseInt($(k+"N").value||"0",10)));
  const isS = cur.slot.solenoids === 1;
  const sum = isS ? (pair("dp") + pair("dr"))
                  : (pair("dn") + pair("dr") + pair("dp") + pair("dl"));
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
  show($("mos2Btn"), !isS && cur.meta && cur.meta.pro);
  show($("rowMosSwap"), !isS && cur.meta && cur.meta.pro);
  recalcRofTheo();
  updateHeader();
}

function refreshSelectorUI(){
  if (!cur.slot) return;
  // 3-pos selector requires Hall — toggling sel3pos auto-flips selMode.
  if (cur.slot.sel3pos) cur.slot.selMode = 1;
  $("sel3pos").checked = !!cur.slot.sel3pos;
  show($("rowPos3"), !!cur.slot.sel3pos);
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
  for (const k of ["dn","dr","dp","dl"]){
    $(k+"R").value = s[k]; $(k+"N").value = s[k];
  }
  $("rofN").value = s.rof;
  refreshSolenoidUI();
  refreshSelectorUI();
}

async function loadMeta(){
  cur.meta = await jget("/load");
  if (cur.meta.lang) lang = (cur.meta.lang==="en") ? "en" : "br";
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
  return {
    name: $("slotName").value || ("Slot "+(cur.i+1)),
    solenoids: $("solD").classList.contains("on") ? 2 : 1,
    trigMode: parseInt($("trigMode").value,10),
    selMode:  cur.slot.selMode,
    sel3pos:  $("sel3pos").checked,
    selPos1:  parseInt($("selPos1").value,10),
    selPos2:  parseInt($("selPos2").value,10),
    selPos3:  parseInt($("selPos3").value,10),
    dn: pair("dn"), dr: pair("dr"), dp: pair("dp"), dl: pair("dl"),
    rof:   Math.max(0, Math.min(50, parseInt($("rofN").value||"0",10))),
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
  r.addEventListener("input", ()=> { n.value = r.value; recalcRofTheo(); });
  n.addEventListener("input", ()=> { r.value = n.value; recalcRofTheo(); });
}

// ===== Help popover =====
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

// ===== Sensitivity live poll =====
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

// ===== Hall calibration wizard =====
let calMode = null;        // "trig" | "sel" | "trigNoise" | "selNoise"
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
  // target: "t" (trigger) or "s" (selector)
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
  // Selector cal: always 3 positions (capture all even when sel3pos is off).
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

// ===== wire-up =====
window.addEventListener("DOMContentLoaded", async ()=>{
  ["dn","dr","dp","dl"].forEach(bindPair);

  $("langBR").addEventListener("click", ()=>{ lang="br"; jpost("/setlang",{lang:"br"}); applyLang(); refreshSelectorUI(); });
  $("langEN").addEventListener("click", ()=>{ lang="en"; jpost("/setlang",{lang:"en"}); applyLang(); refreshSelectorUI(); });

  $("solS").addEventListener("click", ()=>{ if(!cur.slot) return; cur.slot.solenoids=1; refreshSolenoidUI(); });
  $("solD").addEventListener("click", ()=>{ if(!cur.slot) return; cur.slot.solenoids=2; refreshSolenoidUI(); });

  $("trigMode").addEventListener("change", e=>{ if(!cur.slot) return; cur.slot.trigMode = parseInt(e.target.value,10); refreshCalUI(); });
  $("sel3pos").addEventListener("change", e=>{
    if(!cur.slot) return;
    cur.slot.sel3pos = e.target.checked ? 1 : 0;
    if (cur.slot.sel3pos) cur.slot.selMode = 1;   // Hall obrigatório
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

  $("setPwdBtn").addEventListener("click", async ()=>{
    const p = $("wifiPwd").value || "";
    if (p.length < 8) { toast(I[lang].pwdBad, true); return; }
    const r = await jpost("/setpwd", {pwd:p});
    toast(r.ok ? I[lang].pwdOk : (r.error||"err"), !r.ok);
    if (r.ok) $("wifiPwd").value = "";
  });

  $("sensSaveBtn").addEventListener("click", saveSensitivity);

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
  setInterval(async ()=>{ try { cur.meta = await jget("/load"); updateHeader(); } catch(e){} }, 5000);
});
</script>
</body>
</html>
)HTML";
