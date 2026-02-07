/*
  html_templates.h - Embedded HTML/CSS/JS for web dashboard and setup pages
  DarkLight Cover Calibrator - ESP32-S3 Port

  (c) Copyright Nathan Woelfle 2020-present day. All Rights Reserved.
  Creative Commons Attribution-NonCommercial 4.0 International License
*/

#ifndef HTML_TEMPLATES_H
#define HTML_TEMPLATES_H

// Common CSS style block
inline const char* getStyleCSS() {
  return R"rawliteral(
<style>
  * { box-sizing: border-box; margin: 0; padding: 0; }
  body { font-family: 'Segoe UI', Tahoma, sans-serif; background: #1a1a2e; color: #e0e0e0; }
  .container { max-width: 800px; margin: 0 auto; padding: 16px; }
  h1 { color: #e94560; margin-bottom: 8px; font-size: 1.5em; }
  h2 { color: #c0c0c0; margin: 16px 0 8px; font-size: 1.1em; border-bottom: 1px solid #333; padding-bottom: 4px; }
  .nav { display: flex; gap: 12px; margin-bottom: 16px; }
  .nav a { color: #e94560; text-decoration: none; padding: 6px 12px; border: 1px solid #e94560; border-radius: 4px; }
  .nav a:hover, .nav a.active { background: #e94560; color: white; }
  .card { background: #16213e; border-radius: 8px; padding: 16px; margin-bottom: 12px; }
  .status-grid { display: grid; grid-template-columns: repeat(auto-fit, minmax(200px, 1fr)); gap: 12px; }
  .status-item { display: flex; justify-content: space-between; align-items: center; padding: 8px; background: #0f3460; border-radius: 4px; }
  .status-label { font-size: 0.9em; color: #a0a0a0; }
  .status-value { font-weight: bold; font-size: 1.1em; }
  .indicator { display: inline-block; width: 10px; height: 10px; border-radius: 50%; margin-right: 6px; }
  .ind-green { background: #4caf50; }
  .ind-yellow { background: #ff9800; }
  .ind-red { background: #f44336; }
  .ind-blue { background: #2196f3; }
  .ind-gray { background: #666; }
  .btn-group { display: flex; flex-wrap: wrap; gap: 8px; margin-top: 8px; }
  .btn { padding: 8px 16px; border: none; border-radius: 4px; cursor: pointer; font-size: 0.9em; color: white; }
  .btn-primary { background: #e94560; }
  .btn-primary:hover { background: #c73550; }
  .btn-success { background: #4caf50; }
  .btn-success:hover { background: #388e3c; }
  .btn-warning { background: #ff9800; }
  .btn-warning:hover { background: #e68900; }
  .btn-danger { background: #f44336; }
  .btn-danger:hover { background: #d32f2f; }
  .btn-info { background: #2196f3; }
  .btn-info:hover { background: #1976d2; }
  .btn:disabled { background: #555; cursor: not-allowed; }
  .form-group { margin-bottom: 12px; }
  .form-group label { display: block; margin-bottom: 4px; color: #a0a0a0; font-size: 0.9em; }
  .form-group input, .form-group select { width: 100%; padding: 8px; background: #0f3460; border: 1px solid #333; border-radius: 4px; color: #e0e0e0; font-size: 0.9em; }
  .form-row { display: grid; grid-template-columns: 1fr 1fr; gap: 12px; }
  .msg { padding: 8px 12px; border-radius: 4px; margin-top: 8px; display: none; }
  .msg-ok { background: #1b5e20; color: #a5d6a7; }
  .msg-err { background: #b71c1c; color: #ef9a9a; }
  .slider-container { display: flex; align-items: center; gap: 12px; }
  .slider-container input[type=range] { flex: 1; }
  .slider-value { min-width: 40px; text-align: center; }
  .version { text-align: center; color: #555; font-size: 0.8em; margin-top: 16px; }
</style>
)rawliteral";
}

// Dashboard page
inline const char* getDashboardHTML() {
  return R"rawliteral(
<!DOCTYPE html>
<html><head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>DarkLight CoverCalibrator</title>
%STYLE%
</head><body>
<div class="container">
  <h1>DarkLight Cover Calibrator</h1>
  <div class="nav">
    <a href="/" class="active">Dashboard</a>
    <a href="/setup">Setup</a>
    <a href="/update">Update</a>
  </div>

  <div class="card">
    <h2>Cover</h2>
    <div class="status-grid">
      <div class="status-item">
        <span class="status-label">State</span>
        <span class="status-value"><span id="coverInd" class="indicator ind-gray"></span><span id="coverState">--</span></span>
      </div>
    </div>
    <div id="coverControls" class="btn-group">
      <button class="btn btn-success" onclick="sendCmd('opencover')">Open</button>
      <button class="btn btn-primary" onclick="sendCmd('closecover')">Close</button>
      <button class="btn btn-danger" onclick="sendCmd('haltcover')">Halt</button>
    </div>
  </div>

  <div class="card">
    <h2>Calibrator</h2>
    <div class="status-grid">
      <div class="status-item">
        <span class="status-label">State</span>
        <span class="status-value"><span id="calInd" class="indicator ind-gray"></span><span id="calState">--</span></span>
      </div>
      <div class="status-item">
        <span class="status-label">Brightness</span>
        <span class="status-value"><span id="brightness">--</span> / <span id="maxBright">--</span></span>
      </div>
    </div>
    <div id="calSlider" class="slider-container" style="margin-top:8px">
      <input type="range" id="brightSlider" min="0" max="255" value="0" oninput="document.getElementById('brightVal').textContent=this.value">
      <span class="slider-value" id="brightVal">0</span>
      <button class="btn btn-info" onclick="setBrightness()">Set</button>
    </div>
    <div id="calControls" class="btn-group">
      <button class="btn btn-success" onclick="lightOn()">Light On</button>
      <button class="btn btn-danger" onclick="sendCmd('lightoff')">Light Off</button>
    </div>
  </div>

  <div class="card">
    <h2>Heater</h2>
    <div class="status-grid">
      <div class="status-item">
        <span class="status-label">State</span>
        <span class="status-value"><span id="heatInd" class="indicator ind-gray"></span><span id="heatState">--</span></span>
      </div>
      <div class="status-item">
        <span class="status-label">Heater Temp</span>
        <span class="status-value"><span id="heatTemp">--</span>&deg;C</span>
      </div>
      <div class="status-item">
        <span class="status-label">Outside Temp</span>
        <span class="status-value"><span id="outTemp">--</span>&deg;C</span>
      </div>
      <div class="status-item">
        <span class="status-label">Humidity</span>
        <span class="status-value"><span id="humidity">--</span>%</span>
      </div>
      <div class="status-item">
        <span class="status-label">Dew Point</span>
        <span class="status-value"><span id="dewPoint">--</span>&deg;C</span>
      </div>
      <div class="status-item">
        <span class="status-label">Heater PWM</span>
        <span class="status-value"><span id="heatPWM">--</span></span>
      </div>
    </div>
    <div id="heaterControls" class="btn-group">
      <button class="btn btn-info" onclick="sendCmd('autoheat')">Auto</button>
      <button class="btn btn-warning" onclick="sendCmd('manualheat')">Manual</button>
      <button class="btn btn-primary" onclick="sendCmd('heatonclose')">Heat-on-Close</button>
      <button class="btn btn-danger" onclick="sendCmd('heateroff')">Off</button>
    </div>
  </div>

  <p class="version">DarkLight CoverCalibrator <span id="fwVersion">--</span></p>
</div>

<script>
const coverStates = ['Not Present','Closed','Moving','Open','Unknown','Error'];
const calStates = ['Not Present','Off','Not Ready','Ready','Unknown','Error'];
const heatStates = ['Not Present','Off','Auto','On','Unknown','Error','Set'];
const coverColors = ['ind-gray','ind-green','ind-yellow','ind-green','ind-yellow','ind-red'];
const calColors = ['ind-gray','ind-gray','ind-yellow','ind-green','ind-yellow','ind-red'];
const heatColors = ['ind-gray','ind-gray','ind-blue','ind-green','ind-yellow','ind-red','ind-blue'];

function updateStatus() {
  fetch('/api/status').then(r=>r.json()).then(d=>{
    document.getElementById('coverState').textContent = coverStates[d.coverState] || '?';
    document.getElementById('coverInd').className = 'indicator ' + (coverColors[d.coverState]||'ind-gray');
    document.getElementById('calState').textContent = calStates[d.calState] || '?';
    document.getElementById('calInd').className = 'indicator ' + (calColors[d.calState]||'ind-gray');
    document.getElementById('brightness').textContent = d.brightness;
    document.getElementById('maxBright').textContent = d.maxBrightness;
    document.getElementById('brightSlider').max = d.maxBrightness;
    document.getElementById('heatState').textContent = heatStates[d.heaterState] || '?';
    document.getElementById('heatInd').className = 'indicator ' + (heatColors[d.heaterState]||'ind-gray');
    document.getElementById('heatTemp').textContent = d.heaterTemp!=null?d.heaterTemp.toFixed(1):'--';
    document.getElementById('outTemp').textContent = d.outsideTemp!=null?d.outsideTemp.toFixed(1):'--';
    document.getElementById('humidity').textContent = d.humidity!=null?d.humidity.toFixed(1):'--';
    document.getElementById('dewPoint').textContent = d.dewPoint!=null?d.dewPoint.toFixed(1):'--';
    document.getElementById('heatPWM').textContent = d.heaterPWM!=null?d.heaterPWM:'--';
    document.getElementById('fwVersion').textContent = d.version || '--';

    // Disable controls when device not present (state 0)
    setControlsEnabled('coverControls', d.coverState !== 0);
    setControlsEnabled('calControls', d.calState !== 0);
    setControlsEnabled('calSlider', d.calState !== 0);
    setControlsEnabled('heaterControls', d.heaterState !== 0);
  }).catch(e=>{});
}

function setControlsEnabled(containerId, enabled) {
  var el = document.getElementById(containerId);
  if (!el) return;
  var btns = el.querySelectorAll('button');
  var inputs = el.querySelectorAll('input');
  for (var i = 0; i < btns.length; i++) btns[i].disabled = !enabled;
  for (var i = 0; i < inputs.length; i++) inputs[i].disabled = !enabled;
  el.style.opacity = enabled ? '1' : '0.4';
}

function sendCmd(cmd) {
  fetch('/api/cmd?action='+cmd, {method:'POST'}).then(()=>setTimeout(updateStatus,300));
}

function setBrightness() {
  var v = document.getElementById('brightSlider').value;
  fetch('/api/cmd?action=lighton&brightness='+v, {method:'POST'}).then(()=>setTimeout(updateStatus,300));
}

function lightOn() {
  var v = document.getElementById('brightSlider').value;
  if (v == 0) v = document.getElementById('maxBright').textContent;
  fetch('/api/cmd?action=lighton&brightness='+v, {method:'POST'}).then(()=>setTimeout(updateStatus,300));
}

setInterval(updateStatus, 2000);
updateStatus();
</script>
</body></html>
)rawliteral";
}

// Setup page
inline const char* getSetupHTML() {
  return R"rawliteral(
<!DOCTYPE html>
<html><head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>DarkLight Setup</title>
%STYLE%
</head><body>
<div class="container">
  <h1>DarkLight Setup</h1>
  <div class="nav">
    <a href="/">Dashboard</a>
    <a href="/setup" class="active">Setup</a>
    <a href="/update">Update</a>
  </div>

  <div class="card">
    <h2>WiFi Configuration</h2>
    <div class="form-group">
      <label>SSID</label>
      <input type="text" id="wifiSSID" placeholder="Your WiFi network">
    </div>
    <div class="form-group">
      <label>Password</label>
      <input type="password" id="wifiPass" placeholder="WiFi password">
    </div>
    <button class="btn btn-primary" onclick="saveWifi()">Save WiFi</button>
    <div id="wifiMsg" class="msg"></div>
  </div>

  <div class="card" id="servoPosCard">
    <h2>Servo Position</h2>
    <div class="status-grid" style="margin-bottom:12px">
      <div class="status-item">
        <span class="status-label">Current Position</span>
        <span class="status-value"><span id="servoPos">--</span>&deg;</span>
      </div>
      <div class="status-item">
        <span class="status-label">Open Angle</span>
        <span class="status-value"><span id="openAngleDisp">--</span>&deg;</span>
      </div>
      <div class="status-item">
        <span class="status-label">Close Angle</span>
        <span class="status-value"><span id="closeAngleDisp">--</span>&deg;</span>
      </div>
    </div>
    <div class="btn-group">
      <button class="btn btn-primary" onclick="nudge(-10)" style="font-size:1.1em">&laquo; 10</button>
      <button class="btn btn-primary" onclick="nudge(-1)" style="font-size:1.1em">&lsaquo; 1</button>
      <button class="btn btn-primary" onclick="nudge(1)" style="font-size:1.1em">1 &rsaquo;</button>
      <button class="btn btn-primary" onclick="nudge(10)" style="font-size:1.1em">10 &raquo;</button>
    </div>
    <div class="btn-group">
      <button class="btn btn-success" onclick="setAsOpen()">Set Current as Open</button>
      <button class="btn btn-warning" onclick="setAsClose()">Set Current as Close</button>
    </div>
    <div id="posMsg" class="msg"></div>
  </div>

  <div class="card" id="servoConfigCard">
    <h2>Servo Configuration</h2>
    <div class="form-row">
      <div class="form-group">
        <label>Range Min (degrees)</label>
        <input type="number" id="rangeMin" min="0" max="270">
      </div>
      <div class="form-group">
        <label>Range Max (degrees)</label>
        <input type="number" id="rangeMax" min="0" max="270">
      </div>
    </div>
    <div class="form-row">
      <div class="form-group">
        <label>Open Angle</label>
        <input type="number" id="servoOpen" min="0" max="270">
      </div>
      <div class="form-group">
        <label>Close Angle</label>
        <input type="number" id="servoClose" min="0" max="270">
      </div>
    </div>
    <div class="form-row">
      <div class="form-group">
        <label>Min Pulse Width (us)</label>
        <input type="number" id="servoMinPW" min="500" max="2500">
      </div>
      <div class="form-group">
        <label>Max Pulse Width (us)</label>
        <input type="number" id="servoMaxPW" min="500" max="2500">
      </div>
    </div>
    <div class="form-group">
      <label>Move Time (ms)</label>
      <input type="number" id="moveTime" min="1000" max="10000">
    </div>
    <button class="btn btn-primary" onclick="saveServo()">Save Servo</button>
    <div id="servoMsg" class="msg"></div>
  </div>

  <div class="card" id="lightCard">
    <h2>Light Configuration</h2>
    <div class="form-row">
      <div class="form-group">
        <label>Max Brightness Steps</label>
        <select id="maxBright">
          <option value="1">1 (on/off)</option>
          <option value="5">5</option>
          <option value="17">17</option>
          <option value="51">51</option>
          <option value="85">85</option>
          <option value="255">255 (8-bit)</option>
          <option value="1023">1023 (10-bit)</option>
        </select>
      </div>
      <div class="form-group">
        <label>Stabilize Time (ms)</label>
        <input type="number" id="stabTime" min="0" max="10000">
      </div>
    </div>
    <button class="btn btn-primary" onclick="saveLight()">Save Light</button>
    <div id="lightMsg" class="msg"></div>
  </div>

  <div class="card" id="heaterCard">
    <h2>Heater Configuration</h2>
    <div class="form-row">
      <div class="form-group">
        <label>Delta Point (&deg;C above dew)</label>
        <input type="number" id="deltaPoint" min="0" max="20" step="0.5">
      </div>
      <div class="form-group">
        <label>Shutoff Time (minutes)</label>
        <input type="number" id="shutoffMin" min="1" max="180">
      </div>
    </div>
    <button class="btn btn-primary" onclick="saveHeater()">Save Heater</button>
    <div id="heaterMsg" class="msg"></div>
  </div>

  <div class="card">
    <h2>Device</h2>
    <button class="btn btn-warning" onclick="if(confirm('Restart device?'))fetch('/api/restart',{method:'POST'})">Restart</button>
  </div>

  <p class="version">DarkLight CoverCalibrator</p>
</div>

<script>
function showMsg(id, ok, text) {
  var el = document.getElementById(id);
  el.textContent = text;
  el.className = 'msg ' + (ok ? 'msg-ok' : 'msg-err');
  el.style.display = 'block';
  setTimeout(function(){ el.style.display='none'; }, 3000);
}

function loadSettings() {
  fetch('/api/settings').then(r=>r.json()).then(d=>{
    document.getElementById('wifiSSID').value = d.wifiSSID || '';
    document.getElementById('servoOpen').value = d.servoOpen;
    document.getElementById('servoClose').value = d.servoClose;
    document.getElementById('servoMinPW').value = d.servoMinPW;
    document.getElementById('servoMaxPW').value = d.servoMaxPW;
    document.getElementById('moveTime').value = d.moveTime;
    document.getElementById('rangeMin').value = d.rangeMin;
    document.getElementById('rangeMax').value = d.rangeMax;
    document.getElementById('servoPos').textContent = d.servoPos;
    document.getElementById('openAngleDisp').textContent = d.servoOpen;
    document.getElementById('closeAngleDisp').textContent = d.servoClose;
    document.getElementById('maxBright').value = d.maxBright;
    document.getElementById('stabTime').value = d.stabTime;
    document.getElementById('deltaPoint').value = d.deltaPoint;
    document.getElementById('shutoffMin').value = Math.round(d.shutoffTime / 60000);
  });
}

function nudge(dir) {
  fetch('/api/servo/nudge?dir='+dir, {method:'POST'}).then(r=>r.json()).then(d=>{
    if (d.ok) {
      document.getElementById('servoPos').textContent = d.pos;
      document.getElementById('openAngleDisp').textContent = d.open;
      document.getElementById('closeAngleDisp').textContent = d.close;
    }
  }).catch(()=>{});
}

function setAsOpen() {
  fetch('/api/servo/setopen', {method:'POST'}).then(r=>r.json()).then(d=>{
    if (d.ok) {
      document.getElementById('openAngleDisp').textContent = d.open;
      document.getElementById('servoOpen').value = d.open;
      showMsg('posMsg', true, 'Open angle set to ' + d.open + '\u00B0');
    }
  }).catch(()=>showMsg('posMsg', false, 'Request failed'));
}

function setAsClose() {
  fetch('/api/servo/setclose', {method:'POST'}).then(r=>r.json()).then(d=>{
    if (d.ok) {
      document.getElementById('closeAngleDisp').textContent = d.close;
      document.getElementById('servoClose').value = d.close;
      showMsg('posMsg', true, 'Close angle set to ' + d.close + '\u00B0');
    }
  }).catch(()=>showMsg('posMsg', false, 'Request failed'));
}

function postSettings(endpoint, data, msgId) {
  fetch('/api/' + endpoint, {
    method: 'POST',
    headers: {'Content-Type':'application/x-www-form-urlencoded'},
    body: Object.entries(data).map(([k,v])=>k+'='+encodeURIComponent(v)).join('&')
  }).then(r=>r.json()).then(d=>{
    showMsg(msgId, d.ok, d.ok ? 'Saved!' : (d.error||'Error'));
  }).catch(()=>showMsg(msgId, false, 'Request failed'));
}

function saveWifi() {
  postSettings('wifi', {
    ssid: document.getElementById('wifiSSID').value,
    pass: document.getElementById('wifiPass').value
  }, 'wifiMsg');
}

function saveServo() {
  postSettings('servo', {
    open: document.getElementById('servoOpen').value,
    close: document.getElementById('servoClose').value,
    minpw: document.getElementById('servoMinPW').value,
    maxpw: document.getElementById('servoMaxPW').value,
    movetime: document.getElementById('moveTime').value,
    rangemin: document.getElementById('rangeMin').value,
    rangemax: document.getElementById('rangeMax').value
  }, 'servoMsg');
}

function saveLight() {
  postSettings('light', {
    maxbright: document.getElementById('maxBright').value,
    stabtime: document.getElementById('stabTime').value
  }, 'lightMsg');
}

function saveHeater() {
  postSettings('heater', {
    delta: document.getElementById('deltaPoint').value,
    shutoff: parseInt(document.getElementById('shutoffMin').value) * 60000
  }, 'heaterMsg');
}

function setCardEnabled(cardId, enabled) {
  var el = document.getElementById(cardId);
  if (!el) return;
  var btns = el.querySelectorAll('button');
  var inputs = el.querySelectorAll('input, select');
  for (var i = 0; i < btns.length; i++) btns[i].disabled = !enabled;
  for (var i = 0; i < inputs.length; i++) inputs[i].disabled = !enabled;
  el.style.opacity = enabled ? '1' : '0.4';
}

function checkDevicePresence() {
  fetch('/api/status').then(r=>r.json()).then(d=>{
    setCardEnabled('servoPosCard', d.coverState !== 0);
    setCardEnabled('servoConfigCard', d.coverState !== 0);
    setCardEnabled('lightCard', d.calState !== 0);
    setCardEnabled('heaterCard', d.heaterState !== 0);
  }).catch(()=>{});
}

loadSettings();
checkDevicePresence();
</script>
</body></html>
)rawliteral";
}

#endif // HTML_TEMPLATES_H
