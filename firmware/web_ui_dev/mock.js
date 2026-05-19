// BCGA FCU dev mock — intercepts /load /getslot /save etc when running outside the ESP.
// Only activates on file:// or localhost; on the device this script isn't shipped (build_web_ui.py strips it).

(function(){
  const isFileProto = location.protocol === "file:";
  const isLocalHost = /^(localhost|127\.0\.0\.1|\[::1\])$/i.test(location.hostname);
  if (!isFileProto && !isLocalHost) return;

  const VARIANT = document.body.classList.contains("variant-pro") ? "Pro" : "Starter";
  const SSID = "BCGA_FCU_" + (VARIANT === "Pro" ? "PRO" : "STR");
  const FW = "dev-mock-1.0";
  const SLOT_COUNT = 3;
  const STORAGE_KEY = "bcga_mock_state_" + (VARIANT === "Pro" ? "pro" : "str");

  function defaultSlot(i){
    return {
      name: "Slot " + (i+1),
      solenoids: 2,
      trigMode: 0, selMode: 0, sel3pos: 0,
      selPos1: 1, selPos2: 2, selPos3: 0,
      dn: 24, dr: 24, dp: 24, db: 5,
      rof: 0, semiRofMs: 0,
      is: 30, ip: 1,
      hallTrigLow: 800, hallTrigHigh: 2400,
      hallSelLow1: 1200, hallSelLow2: 2600,
      mosfetSwap: 0, invertTrig: 0, silent: 0
    };
  }

  function loadState(){
    try {
      const raw = localStorage.getItem(STORAGE_KEY);
      if (raw) return JSON.parse(raw);
    } catch(e){}
    const slots = [];
    for (let i=0;i<SLOT_COUNT;i++) slots.push(defaultSlot(i));
    // Seed shotsTotal mid-lube-cycle so the progress bar is interesting on first load.
    return { lastSlot: 0, lang: "en", slots, pwd: "12345678", shotsTotal: 8500, shotsSession: 0, cfgPwd: "", cfgLocked: false };
  }
  function saveState(){ try { localStorage.setItem(STORAGE_KEY, JSON.stringify(state)); } catch(e){} }

  let state = loadState();
  let trigPressed = false;
  let trigEvents = 0;
  let hallTick = 0;

  document.addEventListener("keydown", e=>{
    if (e.key === " " || e.code === "Space") {
      if (!trigPressed) {
        // count one "shot" per press (semi behaviour); good enough for a demo.
        state.shotsSession = (state.shotsSession|0) + 1;
        state.shotsTotal   = (state.shotsTotal|0) + 1;
        saveState();
      }
      trigPressed = true; trigEvents++;
    }
  });
  document.addEventListener("keyup", e=>{
    if (e.key === " " || e.code === "Space") trigPressed = false;
  });

  function reply(obj, delayMs){
    const ms = delayMs == null ? 30 : delayMs;
    return new Promise(r => setTimeout(()=>r(jsonResponse(obj)), ms));
  }
  function jsonResponse(obj){
    const body = JSON.stringify(obj);
    return new Response(body, { status: 200, headers: { "content-type": "application/json" } });
  }

  function pickQuery(url, key){
    try {
      const u = new URL(url, location.origin);
      const v = u.searchParams.get(key);
      return v == null ? null : parseInt(v, 10);
    } catch(e){ return null; }
  }

  async function readBody(req){
    if (!req || !req.text) return {};
    try { const t = await req.text(); return t ? JSON.parse(t) : {}; }
    catch(e){ return {}; }
  }

  const realFetch = window.fetch.bind(window);

  window.fetch = async function(input, init){
    const url = typeof input === "string" ? input : (input && input.url) || "";
    const method = ((init && init.method) || (input && input.method) || "GET").toUpperCase();
    const path = (() => { try { return new URL(url, location.origin).pathname; } catch(e){ return url; } })();

    if (!path || path[0] !== "/") return realFetch(input, init);

    // Mirror firmware behaviour: when locked, slot writes are 401.
    if (state.cfgPwd && state.cfgLocked && method === "POST" &&
        (path === "/save" || path === "/setslot" || path === "/reset")){
      return reply({ ok:false, error:"locked" });
    }

    if (path === "/load" && method === "GET"){
      const meta = {
        slots: SLOT_COUNT,
        lastSlot: state.lastSlot|0,
        fw: FW,
        variant: VARIANT,
        ssid: SSID,
        lang: state.lang || "en",
        shotsTotal:   state.shotsTotal|0,
        shotsSession: state.shotsSession|0,
        shots:        (state.shotsTotal|0) + (state.shotsSession|0)
      };
      if (VARIANT === "Pro"){
        const mv = 11400 + Math.floor(Math.random()*200);
        meta.batt = { mv, cells: 3, low: mv < 10500, cut: mv < 9900 };
      }
      meta.cfgLock = { hasPwd: !!state.cfgPwd, locked: !!(state.cfgPwd && state.cfgLocked) };
      return reply(meta);
    }

    if (path === "/getslot" && method === "GET"){
      const i = pickQuery(url, "i") || 0;
      const s = state.slots[i] || defaultSlot(i);
      return reply({ ok:true, slot: s });
    }

    if (path === "/halllive" && method === "GET"){
      hallTick++;
      const base = trigPressed ? 2500 : 900;
      const noise = Math.floor(Math.random()*40) - 20;
      return reply({ ok:true, value: base + noise }, 10);
    }

    if (path === "/trigstate" && method === "GET"){
      return reply({ ok:true, pressed: trigPressed, events: trigEvents }, 10);
    }

    if (path === "/save" && method === "POST"){
      const i = pickQuery(url, "i") || 0;
      const body = await readBody(init && init.body ? { text: ()=>Promise.resolve(init.body) } : null);
      Object.assign(state.slots[i] = state.slots[i] || defaultSlot(i), body);
      saveState();
      return reply({ ok:true });
    }

    if (path === "/setslot" && method === "POST"){
      const i = pickQuery(url, "i") || 0;
      state.lastSlot = i;
      saveState();
      return reply({ ok:true });
    }

    if (path === "/reset" && method === "POST"){
      const i = pickQuery(url, "i") || 0;
      state.slots[i] = defaultSlot(i);
      saveState();
      return reply({ ok:true, slot: state.slots[i] });
    }

    if (path === "/test" && method === "POST"){
      return reply({ ok:true }, 80);
    }

    if (path === "/noisecal" && method === "POST"){
      return reply({ ok:true, margin: 30 + Math.floor(Math.random()*40) }, 800);
    }

    if (path === "/setpwd" && method === "POST"){
      const body = await readBody(init && init.body ? { text: ()=>Promise.resolve(init.body) } : null);
      const p = (body && body.pwd) || "";
      if (p.length < 8) return reply({ ok:false, error: "too short" });
      state.pwd = p; saveState();
      return reply({ ok:true });
    }

    if (path === "/setlang" && method === "POST"){
      const body = await readBody(init && init.body ? { text: ()=>Promise.resolve(init.body) } : null);
      state.lang = (body && body.lang === "br") ? "br" : "en";
      saveState();
      return reply({ ok:true });
    }

    if (path === "/cfglock"){
      const status = ()=> ({ ok:true, hasPwd: !!state.cfgPwd, locked: !!(state.cfgPwd && state.cfgLocked) });
      if (method === "GET") return reply(status());
      const body = await readBody(init && init.body ? { text: ()=>Promise.resolve(init.body) } : null);
      const action = (body && body.action) || "";
      const pwd    = (body && body.pwd)    || "";
      const locked = !!(state.cfgPwd && state.cfgLocked);
      if (action === "set"){
        if (state.cfgPwd && locked) return reply({ ok:false, error:"locked" });
        if (pwd.length < 4) return reply({ ok:false, error:"pwd too short" });
        state.cfgPwd = pwd; state.cfgLocked = false;
      } else if (action === "lock"){
        if (!state.cfgPwd) return reply({ ok:false, error:"no pwd set" });
        state.cfgLocked = true;
      } else if (action === "unlock"){
        if (!state.cfgPwd) return reply({ ok:false, error:"no pwd set" });
        if (state.cfgPwd !== pwd) return reply({ ok:false, error:"bad pwd" });
        state.cfgLocked = false;
      } else if (action === "clear"){
        if (locked && state.cfgPwd !== pwd) return reply({ ok:false, error:"bad pwd" });
        state.cfgPwd = ""; state.cfgLocked = false;
      } else {
        return reply({ ok:false, error:"bad action" });
      }
      saveState();
      return reply(status());
    }

    return realFetch(input, init);
  };

  console.log("[BCGA mock] active — variant=" + VARIANT + " — press Space to simulate trigger");
})();
