const {webkit,chromium}=require('playwright');
const fs=require('fs');const path=require('path');const assert=require('assert');
const source=fs.readFileSync(path.join(__dirname,'../src/main.cpp'),'utf8');
const version=(fs.readFileSync(path.join(__dirname,'../src/Version.h'),'utf8').match(/FW_VERSION "([^"]+)"/)||[])[1];
const html=[...source.matchAll(/R"HTML\(([\s\S]*?)\)HTML"/g)].map(m=>m[1]).join('').replace(/(<span class="chip">v)(<\/span>)/,`$1${version}$2`);
const fixture={ssid:'Testnetz',hostname:'pixelstop',timeZone:'Europe/Berlin',stopName:'Pohlandplatz',stopId:'fixture',weatherCity:'Dresden',weatherLat:51,weatherLon:13,planePlace:'Dresden',planeLat:51,planeLon:13,nightStart:1320,nightEnd:420,nightEnabled:true,brightness:70,nightBrightness:12,rotationMask:1,rotationSec:[30,20,20,15,20,20,20,20,20],departureCount:4,departurePoll:30,weatherPoll:600,planeRadiusKm:25,planePoll:10,planePlaceToggleSec:10,trackerStatusPollMin:15,vvoMotMask:127,countdowns:[]};
(async()=>{
 const engine=process.env.SMALLTV_BROWSER==='chromium'?chromium:webkit;
 const browser=await engine.launch({headless:true,...(process.env.SMALLTV_WEBKIT_EXECUTABLE?{executablePath:process.env.SMALLTV_WEBKIT_EXECUTABLE}:{})});
 try {
  for(const width of [320,375,390,430,560,561,768,820,1024]){
   const context=await browser.newContext({viewport:{width,height:1000},isMobile:true,hasTouch:true});
   const page=await context.newPage();
   let posted=null;const config={...fixture};
   await page.route('http://smalltv.test/**',r=>{
    if(r.request().method()==='POST'){posted=r.request().postDataJSON();Object.assign(config,posted);return r.fulfill({contentType:'application/json',body:JSON.stringify({ok:true,reboot:false})});}
    return r.fulfill({contentType:r.request().url().endsWith('/')?'text/html':'application/json',body:r.request().url().endsWith('/')?html:JSON.stringify(r.request().url().includes('/api/config')?config:{ok:true})});
   });
   await page.goto('http://smalltv.test/');
   await page.waitForFunction(()=>document.getElementById('nightStart').value==='22:00');
   assert.strictEqual(await page.locator('#timeZone').inputValue(),'Europe/Berlin');
   assert(!(await page.locator('#geoAdvanced').evaluate(e=>e.open)));
   assert(await page.locator('#serviceInfo').evaluate(e=>e.previousElementSibling.textContent.includes('Firmware-Update')));
   assert(!(await page.locator('#wetter').textContent()).includes('Ortsauswahl über diesen Browser'));
   assert(!(await page.locator('#flugradar').textContent()).includes('Ortsauswahl über diesen Browser'));

   assert.deepStrictEqual(await page.locator('#modeRows .modeLeft span').allTextContents(),['Abfahrten','Wetter','Flugradar','Flug-Tracker','Uhr','Countdown 1','Countdown 2','Countdown 3','Countdown 4']);
   assert.strictEqual(await page.locator('#brightness').getAttribute('min'),'1');
   assert.strictEqual(await page.locator('#nightBrightness').getAttribute('min'),'1');
   assert.strictEqual(await page.locator('#timeZone option').count(),50);
   const tzOrder=await page.evaluate(()=>{
    const cards=[...document.querySelectorAll('#anzeige .card')];
    return cards.map(c=>c.querySelector('h2')?.textContent);
   });
   assert(tzOrder.indexOf('Zeitzone')<tzOrder.indexOf('Nachtmodus'));
   const layout=await page.evaluate(()=>{
    const a=document.getElementById('nightStart'),b=document.getElementById('nightEnd');
    const rect=e=>{const r=e.getBoundingClientRect();return {left:r.left,right:r.right,top:r.top,bottom:r.bottom,width:r.width,height:r.height}};
    return {a:rect(a),b:rect(b),card:rect(a.closest('.card')),overflow:document.documentElement.scrollWidth>innerWidth};
   });
   for(const box of [layout.a,layout.b]){assert(box.left>=layout.card.left+15&&box.right<=layout.card.right-15,JSON.stringify({width,layout}));assert(box.height<=46);}
   if(width<=560)assert(layout.b.top>=layout.a.bottom+5);
   else {assert(layout.b.left-layout.a.right>=9);assert(Math.abs(layout.a.width-layout.b.width)<1);}
   assert(!layout.overflow,`Page overflow at ${width}`);
   await page.locator('[data-p="flugradar"]').click();
   assert(await page.locator('#planePlaceToggleSec').isDisabled());
   await page.locator('#planeShowPlace').check();assert(await page.locator('#planePlaceToggleSec').isEnabled());
   await page.locator('#planePlaceToggleSec').selectOption('0');
   assert(await page.locator('#planeShowAge').isChecked(),'Age defaults to enabled for older settings');
   await page.locator('#planeShowAge').uncheck();
   const style=await page.evaluate(()=>{
    const h=getComputedStyle(document.querySelector('#flugradar .subhead'));
    const l=getComputedStyle(document.querySelector('#flugradar label'));
    return {head:[h.color,h.fontSize,h.fontWeight],label:[l.color,l.fontSize,l.fontWeight]};
   });
   assert.deepStrictEqual(style.head,style.label);
   if(process.env.SMALLTV_SCREENSHOTS&&[390,820].includes(width)){
    await page.locator('[data-p="anzeige"]').click();
    await page.locator('#nightStart').scrollIntoViewIfNeeded();
    await page.screenshot({path:path.join(process.env.SMALLTV_SCREENSHOTS,`night-${width}.png`)});
   }
   await page.locator('[data-p="anzeige"]').click();
   await page.locator('#timeZone').selectOption('Asia/Tokyo');
   await page.getByRole('button',{name:'Einstellungen speichern',exact:true}).click();
   await page.waitForFunction(()=>document.querySelector('.toast')?.textContent.includes('gespeichert'));
   assert(posted&&posted.timeZone==='Asia/Tokyo','Selected zone missing in save payload');
   assert.strictEqual(posted.planePlaceToggleSec,0,'No-switch option missing in save payload');
   assert.strictEqual(posted.planeShowAge,false,'Disabled age missing in save payload');
   await page.waitForFunction(()=>document.getElementById('timeZone').value==='Asia/Tokyo');
   assert(!(await page.locator('#planeShowAge').isChecked()),'Disabled age restored incorrectly');
   if(width===390){
    await page.locator('[data-p="wetter"]').click();
    const requests=[];let releaseFirst;
    await page.route('http://smalltv.test/api/geo/**',()=>{throw new Error('Geo must not use the ESP8266 HTTP endpoints')});
    await page.route('https://nominatim.openstreetmap.org/reverse**',async r=>{
     const u=new URL(r.request().url());requests.push(u);
     assert.match((await r.request().allHeaders()).referer,/\/pixelstop\//,'Browser request must identify the application');
     if(requests.length===1)await new Promise(resolve=>{releaseFirst=resolve});
     const name=requests.length===1?'OLD':'北京';
     return r.fulfill({headers:{'Access-Control-Allow-Origin':'*'},contentType:'application/json',body:JSON.stringify({ok:true,name})});
    });
    await page.evaluate(()=>{setMapPoint('weather',65.69448,343.30078);reversePlace('weather',65.69448,343.30078)});
    assert(Math.abs(Number(await page.locator('#weatherLon').inputValue())+16.69922)<0.00001);
    assert(await page.locator('#saveBtn').isDisabled(),'Save must be blocked during debounce');
    const beforePost=JSON.stringify(posted);await page.evaluate(()=>save());assert.strictEqual(JSON.stringify(posted),beforePost,'Programmatic save must also respect pending place');
    await page.waitForFunction(()=>geoBusy);await page.waitForTimeout(100);
    assert(await page.locator('#saveBtn').isDisabled(),'Save must be blocked during reverse HTTP');
    assert(requests.length===1&&Number(requests[0].searchParams.get('lon'))>=-180&&Number(requests[0].searchParams.get('lon'))<=180);
    await page.evaluate(()=>{for(let i=0;i<10;i++){setMapPoint('weather',39.9,116.4+i*360);reversePlace('weather',39.9,116.4+i*360)}});
    releaseFirst();
    await page.waitForFunction(()=>document.getElementById('weatherCity').value==='北京');
    assert.strictEqual(requests.length,2,'Rapid picks must coalesce to the last request');
    assert.strictEqual(requests[0].searchParams.get('format'),'jsonv2');
    assert(await page.locator('#saveBtn').isEnabled(),'Save must recover after final name');
    assert.strictEqual(await page.locator('#weatherLon').inputValue(),'116.40000');
    await page.unroute('https://nominatim.openstreetmap.org/reverse**');
    const count=requests.length;await page.evaluate(()=>reversePlace('weather',39.9,116.4));await page.waitForFunction(()=>!geoBusy&&!geoPending.weather);assert.strictEqual(requests.length,count,'Repeat coordinate must use browser cache');
    let releaseObsolete;
    await page.route('https://nominatim.openstreetmap.org/reverse**',async r=>{await new Promise(resolve=>{releaseObsolete=resolve});return r.fulfill({headers:{'Access-Control-Allow-Origin':'*'},contentType:'application/json',body:JSON.stringify({ok:true,name:'OBSOLETE'})})});
    await page.route('https://nominatim.openstreetmap.org/search**',r=>r.fulfill({headers:{'Access-Control-Allow-Origin':'*'},contentType:'application/json',body:JSON.stringify([{name:'Dresden',display_name:'Dresden',lat:'51.05',lon:'13.74'}])}));
    await page.evaluate(()=>{setMapPoint('weather',65,-17);reversePlace('weather',65,-17)});
    await page.waitForFunction(()=>geoBusy);await page.waitForTimeout(100);
    await page.locator('#weatherSearch').fill('Dresden');await page.locator('#weatherSearchBtn').click();
    releaseObsolete();await page.locator('#weatherResults .pickres').click();
    await page.waitForFunction(()=>!geoBusy);
    assert.strictEqual(await page.locator('#weatherCity').inputValue(),'Dresden','Old map reply overwrote selected search result');
    let searches=0;
    await page.unroute('https://nominatim.openstreetmap.org/search**');
    await page.route('https://nominatim.openstreetmap.org/search**',r=>{searches++;assert.strictEqual(new URL(r.request().url()).searchParams.get('accept-language'),'de');return r.fulfill({headers:{'Access-Control-Allow-Origin':'*'},contentType:'application/json',body:JSON.stringify([{namedetails:{name:'Praha','name:de':'Prag'},display_name:'<img src=x onerror=alert(1)>',lat:'50.08',lon:'14.42'}])})});
    await page.locator('#weatherGeoGerman').check();
    await page.locator('#weatherSearch').fill('Prag');await page.locator('#weatherSearchBtn').click();
    await page.locator('#weatherResults .pickres').waitFor();
    assert.strictEqual(await page.locator('#weatherResults img').count(),0,'Provider labels must render as text');
    await page.locator('#weatherResults .pickres').click();assert.strictEqual(await page.locator('#weatherCity').inputValue(),'Prag');
    await page.locator('#weatherSearch').fill('Prag');await page.locator('#weatherSearchBtn').click();await page.waitForFunction(()=>!geoBusy&&!geoPending.weather);
    assert.strictEqual(searches,1,'Repeated text search must use cached response');
    assert(await page.evaluate(()=>localStorage.getItem('smalltv.geoCache').includes('Prag')),'Cache must survive a page reload');
    await page.unroute('https://nominatim.openstreetmap.org/reverse**');
    await page.route('https://nominatim.openstreetmap.org/reverse**',r=>r.fulfill({headers:{'Access-Control-Allow-Origin':'*'},contentType:'application/json',body:JSON.stringify({ok:true,name:''})}));
    await page.evaluate(()=>{setMapPoint('weather',0,-150);reversePlace('weather',0,-150)});
    await page.waitForFunction(()=>document.getElementById('weatherResults').textContent.includes('kein Ortsname verfügbar'));
    assert(await page.locator('#saveBtn').isEnabled(),'Nameless points must remain savable');
    await page.getByRole('button',{name:'Einstellungen speichern',exact:true}).click();
    await page.waitForFunction(()=>document.querySelector('.toast')?.textContent.includes('gespeichert'));
    assert(posted.weatherLat===0&&posted.weatherLon===-150&&posted.weatherCity==='');
    assert(await page.evaluate(()=>Number.isNaN(parseCoord(''))));
    await page.unroute('https://nominatim.openstreetmap.org/reverse**');
    let releaseWeather;
    await page.route('https://nominatim.openstreetmap.org/reverse**',async r=>{
     if(new URL(r.request().url()).searchParams.get('lat')==='10.000000')await new Promise(resolve=>{releaseWeather=resolve});
     return r.fulfill({status:502,headers:{'Access-Control-Allow-Origin':'*'},contentType:'application/json',body:JSON.stringify({ok:false,error:'Fixture error'})});
    });
    await page.evaluate(()=>{reversePlace('weather',10,20);reversePlace('plane',11,21)});
    assert(await page.locator('#saveBtn').isDisabled(),'Both queued map requests must block save');
    await page.waitForFunction(()=>geoBusy);await page.waitForTimeout(100);releaseWeather();
    await page.waitForFunction(()=>!geoBusy&&!geoPending.weather&&!geoPending.plane);
    assert(await page.locator('#saveBtn').isEnabled(),'Reverse errors must release save after both maps finish');
    await page.unroute('https://nominatim.openstreetmap.org/reverse**');
    await context.setOffline(true);
    await page.evaluate(()=>searchPlace('weather'));
    assert.strictEqual(await page.locator('#weatherResults').textContent(),'Internetverbindung erforderlich');
    await page.evaluate(()=>toggleMap('plane'));
    assert.strictEqual(await page.locator('#planeResults').textContent(),'Internetverbindung erforderlich');
    assert(await page.locator('#planeMapWrap').evaluate(e=>e.classList.contains('hidden')));
    await page.evaluate(()=>reversePlace('plane',12,22));
    assert.strictEqual(await page.locator('#planeResults').textContent(),'Internetverbindung erforderlich');
    assert(await page.locator('#saveBtn').isEnabled());
    await context.setOffline(false);
    let limited=0;await page.route('https://nominatim.openstreetmap.org/reverse**',r=>{limited++;return r.fulfill({status:429,headers:{'Access-Control-Allow-Origin':'*','Access-Control-Expose-Headers':'Retry-After','Retry-After':'120'},body:''})});
    await page.evaluate(()=>reversePlace('plane',12,22));await page.waitForFunction(()=>!geoBusy&&!geoPending.plane);
    assert(await page.locator('#planeResults').textContent().then(x=>x.includes('Pause')));
    await page.evaluate(()=>reversePlace('weather',13,23));await page.waitForFunction(()=>!geoBusy&&!geoPending.weather);
    assert.strictEqual(limited,1,'Provider pause must apply to both search/map programs');
    assert(await page.locator('#saveBtn').isEnabled(),'Provider pause must release save');
    await page.locator('[data-p="system"]').click();
    assert(!(await page.locator('#geoAdvanced').evaluate(e=>e.open)),'Advanced settings must start closed');
    await page.locator('#geoAdvanced summary').click();
    await page.locator('#geoProviderUrl').fill('https://geo.example.test/nominatim/');await page.locator('#geoProviderBtn').click();
    assert.strictEqual(await page.evaluate(()=>localStorage.getItem('smalltv.geoProvider')),'https://geo.example.test/nominatim/','Provider must be replaceable without firmware update');
    await page.route('http://smalltv.test/api/log',r=>r.fulfill({contentType:'text/plain',body:'Radar.Position | HTTP 429 | Heap 23000 B'}));
    await page.locator('#logBtn').click();
    await page.waitForFunction(()=>document.getElementById('diagnosticLog').textContent.includes('HTTP 429'));
    await page.locator('#logCopyBtn').click();
    await page.waitForFunction(()=>window.getSelection().toString().includes('HTTP 429'));
    const fonts=await page.evaluate(()=>['logBtn','logCopyBtn','rebootBtn'].map(id=>getComputedStyle(document.getElementById(id)).fontSize));assert.strictEqual(new Set(fonts).size,1);

   }
   console.log(`${engine.name()} ${width}px: time fields, spacing, captions, dependent select and time-zone load/save OK`);
   await context.close();
  }
 } finally {await browser.close();}
})().catch(e=>{console.error(e);process.exitCode=1});
