#include "Arduino.h"
#include "web_server.h"

const char* htmlPage = R"rawliteral(
<!DOCTYPE html><html><head><meta name="viewport" content="width=device-width,initial-scale=1">
<style>body{font-family:system-ui,sans-serif;text-align:center;padding:20px;background:#f4f4f4}
.state{font-size:28px;font-weight:bold;margin:10px 0}
.timer{font-size:36px;font-weight:bold;font-variant-numeric:tabular-nums;margin:10px 0;color:#28a745}
.progress{width:100%;max-width:300px;margin:10px auto;height:20px;background:#ddd;border-radius:10px;overflow:hidden}
.bar{height:100%;width:0%;background:linear-gradient(90deg,#28a745,#20c997);transition:width 0.5s}
.btn{padding:12px 24px;margin:6px;font-size:16px;border:none;border-radius:8px;color:#fff;cursor:pointer}
.btn-start{background:#28a745}.btn-stop{background:#dc3545}.btn-fix{background:#ffc107;color:#000;display:none}
.btn-mock{background:#6c757d}
.mock-panel{background:#fff;border:1px solid #ccc;border-radius:8px;padding:15px;margin:15px auto;max-width:300px;text-align:left;display:none}
.mock-panel h3{margin:0 0 10px;font-size:16px}
.mock-row{display:flex;justify-content:space-between;align-items:center;margin:8px 0}
.mock-toggle{width:44px;height:24px;background:#ccc;border-radius:12px;position:relative;cursor:pointer;transition:0.3s}
.mock-toggle.active{background:#28a745}
.mock-toggle::after{content:'';position:absolute;top:2px;left:2px;width:20px;height:20px;background:#fff;border-radius:50%;transition:0.3s}
.mock-toggle.active::after{left:22px}
.error-msg{color:#dc3545;font-weight:bold;margin:5px 0}
.test-banner{background:#ffc107;color:#000;padding:8px;margin:10px auto;max-width:300px;border-radius:6px;font-weight:bold;display:none}
</style></head><body>
<h2>ESP32 Dishwasher</h2>
<div class="test-banner" id="testBanner">TEST MODE ACTIVE</div>
<div class="state" id="stepInfo">Ready</div>
<div class="timer" id="timer">--:--</div><div class="progress"><div class="bar" id="prog"></div></div>
<div id="sensors">Low: - | High: -</div>
<div id="errorMsg" class="error-msg" style="display:none;"></div>
<select id="cycle"><option value="0">Quick</option><option value="1" selected>Normal</option><option value="2">Deep</option><option value="3">Steam</option></select><br>
<button class="btn btn-start" onclick="fetch('/start?c='+document.getElementById('cycle').value)">START</button>
<button class="btn btn-stop" onclick="fetch('/stop')">STOP</button>
<button class="btn btn-fix" id="fixBtn" onclick="fetch('/fix')">FIX ISSUE</button>
<button class="btn btn-mock" onclick="toggleMockPanel()">TEST MODE</button>

<div class="mock-panel" id="mockPanel">
  <h3>Sensor Mock Controls</h3>
  <div class="mock-row"><span>Enable Mock</span><div class="mock-toggle" id="mockToggle" onclick="toggleMock()"></div></div>
  <div class="mock-row"><span>Water LOW</span><div class="mock-toggle" id="lowToggle" onclick="toggleSensor('low')"></div></div>
  <div class="mock-row"><span>Water HIGH</span><div class="mock-toggle" id="highToggle" onclick="toggleSensor('high')"></div></div>
</div>

<script>
function fmt(s){if(s<=0)return"--:--";const m=Math.floor(s/60),r=s%60;return m+":"+(r<10?"0":"")+r}
function toggleMockPanel(){document.getElementById('mockPanel').style.display=document.getElementById('mockPanel').style.display==='block'?'none':'block'}

// Instant toggle without page reload
function toggleMock(){
  const t=document.getElementById('mockToggle');
  t.classList.toggle('active');
  fetch('/mock/enable?state='+(t.classList.contains('active')?'1':'0'));
}
function toggleSensor(s){
  const t=s==='low'?document.getElementById('lowToggle'):document.getElementById('highToggle');
  t.classList.toggle('active');
  fetch('/mock/set?sensor='+s+'&state='+(t.classList.contains('active')?'1':'0'));
}

// Background poller keeps UI in sync with ESP32
setInterval(()=>{
  fetch('/status?t='+Date.now()).then(r=>r.json()).then(d=>{
    document.getElementById('stepInfo').innerText=d.step+(d.total>0?' ('+(d.idx+1)+'/'+d.total+')':'');
    document.getElementById('timer').innerText=fmt(d.timeLeft);
    document.getElementById('prog').style.width=d.pct+'%';
    document.getElementById('sensors').innerText='Low: '+(d.low?'OK':'-')+' | High: '+(d.high?'OK':'-');
    
    const errBox=document.getElementById('errorMsg'), fixBtn=document.getElementById('fixBtn'), testBanner=document.getElementById('testBanner');
    if(d.state==='ERROR'||(d.error&&d.error.length>0)){errBox.innerText=d.error;errBox.style.display='block';fixBtn.style.display='inline-block'}else{errBox.style.display='none';fixBtn.style.display='none'}
    
    testBanner.style.display=d.mock?'block':'none';
    document.getElementById('mockToggle').className='mock-toggle'+(d.mock?' active':'');
    document.getElementById('lowToggle').className='mock-toggle'+(d.mockLow?' active':'');
    document.getElementById('highToggle').className='mock-toggle'+(d.mockHigh?' active':'');
  });
},1000);
</script></body></html>
)rawliteral";

void setupWebServer(WebServer &server) {
  server.on("/", [&server](){ server.send(200, "text/html", htmlPage); });

  server.on("/start", [&server](){
    if (server.hasArg("c")) currentCycleIdx = server.arg("c").toInt();
    if (currentCycleIdx < 0 || currentCycleIdx >= CYCLE_COUNT) currentCycleIdx = 1;
    if (currentState == IDLE || currentState == ERROR) {
      currentState = FILLING; stepStart = millis(); currentStepIdx = 0;
      activeError = NO_ERROR;
      Serial.print("Starting: "); Serial.println(AVAILABLE_CYCLES[currentCycleIdx].name);
    }
    server.send(200, "text/plain", "OK");
  });

  server.on("/stop", [&server](){ currentState = IDLE; currentStepIdx = 0; server.send(200, "text/plain", "STOPPED"); });
  server.on("/reset", [&server](){ clearError(); server.send(200, "text/plain", "RESET"); });
  
  server.on("/fix", [&server](){
    activeError = NO_ERROR; currentState = IDLE; currentStepIdx = 0;
    digitalWrite(PIN_INPUT_SOLENOID, LOW); digitalWrite(PIN_DRAIN_SOLENOID, LOW);
    digitalWrite(PIN_PUMP, LOW); digitalWrite(PIN_HEATER, LOW);
    Serial.println("[TEST] Error forced cleared via UI.");
    server.send(200, "text/plain", "FIXED");
  });

  server.on("/mock/enable", [&server](){
    mockSensors = server.arg("state") == "1";
    server.send(200, "text/plain", "OK");
  });
  server.on("/mock/set", [&server](){
    String sensor = server.arg("sensor");
    bool state = server.arg("state") == "1";
    if (sensor == "low") mockWaterLow = state;
    else if (sensor == "high") mockWaterHigh = state;
    server.send(200, "text/plain", "OK");
  });

  server.on("/status", [&server](){
    server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
    
    const DishCycle& cycle = AVAILABLE_CYCLES[currentCycleIdx];
    const CycleStep* step = (currentState != IDLE && currentStepIdx < cycle.stepCount) ? &cycle.steps[currentStepIdx] : nullptr;
    
    unsigned long timeLeft = 0, pct = 0;
    if (step && step->durationMs > 0) {
      timeLeft = getStepTimeRemaining(*step, stepStart);
      unsigned long elapsed = millis() - stepStart;
      pct = (elapsed >= step->durationMs) ? 100 : (elapsed * 100 / step->durationMs);
    } else if (step) { pct = 50; }

    String errorMsg = "";
    switch(activeError) {
      case ERROR_LOW_WATER: errorMsg = "Low water during heating. Check tank."; break;
      case ERROR_FILL_TIMEOUT: errorMsg = "Fill timeout. Check water supply/valve."; break;
      case ERROR_DRAIN_TIMEOUT: errorMsg = "Drain timeout. Check hose/pump."; break;
      default: break;
    }

    String json = "{\"state\":\"" + String(stateToString(currentState)) + "\",";
    json += "\"step\":\"" + String(step ? stepToString(step->type) : "Ready") + "\",";
    json += "\"idx\":" + String(currentStepIdx) + ",\"total\":" + String(cycle.stepCount) + ",";
    json += "\"timeLeft\":" + String(timeLeft) + ",\"pct\":" + String(pct) + ",";
    json += "\"error\":\"" + errorMsg + "\",";
    json += "\"low\":" + String(waterLow ? "true" : "false") + ",\"high\":" + String(waterHigh ? "true" : "false") + ",";
    json += "\"mock\":" + String(mockSensors ? "true" : "false") + ",";
    json += "\"mockLow\":" + String(mockWaterLow ? "true" : "false") + ",";
    json += "\"mockHigh\":" + String(mockWaterHigh ? "true" : "false") + "}";
    
    server.send(200, "application/json", json);
  });
}