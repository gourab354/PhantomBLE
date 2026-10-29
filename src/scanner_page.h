#ifndef SCANNER_PAGE_H
#define SCANNER_PAGE_H

#include <Arduino.h>

const char SCANNER_PAGE[] PROGMEM = R"rawliteral(
<!DOCTYPE html><html lang="en"><head>
<meta charset="UTF-8"><meta name="viewport" content="width=device-width,initial-scale=1">
<title>PhantomBLE Scanner</title>
<style>
*{margin:0;padding:0;box-sizing:border-box}
body{background:#0d1117;color:#c9d1d9;font-family:-apple-system,'Segoe UI',sans-serif;max-width:640px;margin:0 auto;padding:16px}
h1{color:#00ff88;font-size:24px;text-align:center;padding:20px 0 5px}
.sub{text-align:center;color:#8b949e;font-size:13px;margin-bottom:20px}
.bar{display:flex;gap:8px;margin-bottom:12px;flex-wrap:wrap;align-items:center}
.btn{padding:10px 18px;border:none;border-radius:8px;cursor:pointer;font-size:14px;font-weight:600;transition:.2s}
.btn:active{transform:scale(.96)}
.b1{background:#00ff88;color:#000}.b1:hover{background:#00cc6a}.b1:disabled{background:#21262d;color:#484f58;cursor:default}
.b2{background:#f85149;color:#fff}.b3{background:#21262d;color:#c9d1d9;text-decoration:none;display:inline-flex;align-items:center}
.b4{background:#1f6feb;color:#fff;padding:7px 14px;font-size:12px}
.b4:hover{background:#1958c4}
.st{padding:12px;border-radius:8px;margin-bottom:12px;font-size:13px;background:#161b22;border:1px solid #30363d}
.st.on{border-color:#00ff88;color:#00ff88}
.cnt{color:#8b949e;font-size:13px;margin-bottom:8px}
.card{background:#161b22;border:1px solid #30363d;border-radius:12px;padding:14px;margin-bottom:8px;display:flex;justify-content:space-between;align-items:center;transition:.2s}
.card:hover{border-color:#388bfd}
.card.picked{border-color:#00ff88;background:#0d2818}
.ci h3{font-size:14px;color:#e6edf3;margin-bottom:4px;word-break:break-all}
.ci .m{font-size:12px;color:#8b949e;line-height:1.8}
.ci .m span{margin-right:10px}
.bg{display:inline-block;padding:2px 8px;border-radius:4px;font-size:11px;font-weight:600;margin-right:4px}
.bg-t{background:#1a1a2e;color:#58a6ff}
.bg-s{background:#1a2e1a;color:#3fb950}
.rb{display:inline-block;width:50px;height:6px;background:#21262d;border-radius:3px;overflow:hidden;vertical-align:middle}
.rf{height:100%;border-radius:3px}
select{background:#21262d;color:#c9d1d9;border:1px solid #30363d;padding:8px 12px;border-radius:8px;font-size:13px}
</style></head><body>
<h1>PhantomBLE Scanner</h1>
<p class="sub">Discover and Clone Nearby BLE Devices</p>
<div class="bar">
<button class="btn b1" id="sb" onclick="go()">Scan</button>
<button class="btn b2" id="xb" onclick="halt()" style="display:none">Stop</button>
<select id="dur"><option value="5">5s</option><option value="10" selected>10s</option><option value="15">15s</option><option value="20">20s</option></select>
<a href="/" class="btn b3">Dashboard</a>
</div>
<div class="st" id="st">Ready. Press Scan to discover nearby BLE peripherals.</div>
<div class="cnt" id="cnt"></div>
<div id="dl"></div>
<script>
var T=null,S=false;
function go(){
 fetch('/api/scan',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify({action:'start',duration:+document.getElementById('dur').value})})
 .then(function(r){return r.json()}).then(function(){
  S=true;ss('sb',true,'Scanning...');document.getElementById('xb').style.display='';
  document.getElementById('st').className='st on';si('Scanning for nearby BLE devices...');
  document.getElementById('dl').innerHTML='';T=setInterval(poll,1500);
 }).catch(function(e){si('Failed: '+e);});
}
function halt(){
 fetch('/api/scan',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify({action:'stop'})}).then(function(){done();});
}
function done(){
 S=false;if(T){clearInterval(T);T=null;}
 ss('sb',false,'Scan');document.getElementById('xb').style.display='none';
 document.getElementById('st').className='st';poll();
}
function poll(){
 fetch('/api/scan/results').then(function(r){return r.json()}).then(function(d){
  if(!d.scanning&&S)done();
  var devs=d.devices||[];
  document.getElementById('cnt').textContent=devs.length+' device(s) found';
  if(d.scanning)si('Scanning... ('+devs.length+' found)');
  else if(devs.length>0)si('Scan complete. Select a device to clone its identity.');
  else if(d.done)si('Scan complete. No devices found.');
  render(devs);
 });
}
function render(devs){
 devs.sort(function(a,b){return b.rssi-a.rssi});
 var h='';
 for(var i=0;i<devs.length;i++){
  var d=devs[i];
  var p=Math.min(100,Math.max(5,(d.rssi+100)*2));
  var c=p>60?'#3fb950':p>30?'#d29922':'#f85149';
  var n=d.name||'(Unknown)';
  var tb=d.appearance?'<span class="bg bg-t">'+tn(d.appearance)+'</span>':'';
  var sb=d.services?'<span class="bg bg-s">'+d.services.split(',')[0]+'</span>':'';
  h+='<div class="card" id="d'+i+'"><div class="ci"><h3>'+tb+sb+esc(n)+'</h3><div class="m">'
   +'<span>'+d.address+'</span>'
   +'<span>'+d.rssi+'dBm <span class="rb"><span class="rf" style="width:'+p+'%;background:'+c+'"></span></span></span>'
   +(d.appearance?'<span>0x'+d.appearance.toString(16).toUpperCase()+'</span>':'')
   +'</div></div>'
   +'<button class="btn b4" onclick="clone('+i+')">Clone</button></div>';
 }
 document.getElementById('dl').innerHTML=h;
}
function clone(i){
 fetch('/api/scan/clone',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify({index:i})})
 .then(function(r){return r.json()}).then(function(d){
  if(d.ok){
   si('Cloned as "'+d.name+'" - Now advertising!');
   document.getElementById('st').className='st on';
   var el=document.getElementById('d'+i);if(el)el.className='card picked';
  }else si('Failed: '+(d.error||'unknown'));
 });
}
function tn(a){var m={961:'Keyboard',962:'Mouse',963:'Joystick',964:'Gamepad',967:'Pen',832:'HRM',320:'TV',640:'AppleTV',512:'Tag'};return m[a]||('0x'+a.toString(16));}
function si(t){document.getElementById('st').textContent=t;}
function ss(id,dis,txt){var e=document.getElementById(id);e.disabled=dis;e.textContent=txt;}
function esc(s){var d=document.createElement('div');d.textContent=s;return d.innerHTML;}
</script></body></html>
)rawliteral";

const size_t SCANNER_PAGE_LEN = sizeof(SCANNER_PAGE) - 1;

#endif // SCANNER_PAGE_H
