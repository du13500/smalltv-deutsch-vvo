#!/usr/bin/env python3
"""Validate the actual release source and its embedded web script."""
from pathlib import Path
import subprocess,tempfile,re
root=Path(__file__).resolve().parents[1]
s=(root/'src/main.cpp').read_text()
version=re.search(r'#define FW_VERSION "([^"]+)"',(root/'src/Version.h').read_text()).group(1)
assert '<span class="chip">v)HTML" FW_VERSION R"HTML(</span>' in s, 'Web UI shows FW_VERSION'
for name in ['README.md','LICENSE','CONTRIBUTING.md','THIRD_PARTY_SERVICES.md','ROADMAP.md','TESTPLAN.md','CHANGELOG.md']:
 assert (root/name).is_file(),name
html=''.join(re.findall(r'R"HTML\((.*?)\)HTML"',s,re.S))
assert 'navigator.geolocation' not in html and 'alert(' not in html
assert 'entwickelt von du13500' in html
zone_select=re.search(r'<select id="timeZone">(.*?)</select>',html,re.S).group(1)
ui_zones=re.findall(r'<option value="([^"]+)"',zone_select)
header_zones=re.findall(r'PROGMEM\s*=\s*"([A-Za-z_]+(?:/[A-Za-z_]+)*)";', (root/'src/TimeZones.h').read_text())
assert ui_zones==header_zones, 'UI and firmware time-zone choices differ'
assert len(ui_zones)==50
readme=(root/'README.md').read_text()
# README shows the build badge of the shared workflow and names this version.
assert 'https://github.com/du13500/pixelstop/actions/workflows/build.yml/badge.svg' in readme, 'Build badge missing'
assert version in readme, 'README does not mention the current version'
ids=re.findall(r'\bid="([^"${]+)"',html)
assert len(ids)==len(set(ids)), 'Duplicate fixed HTML ID'
# Extract every inline script, rather than treating a stored boolean as a test.
with tempfile.TemporaryDirectory() as tmp:
 for i,js in enumerate(re.findall(r'<script>(.*?)</script>',html,re.S)):
  p=Path(tmp)/f'webui-{i}.js';p.write_text(js);subprocess.run(['node','--check',str(p)],check=True)
# Ensure generated font matches its reproducible source.
font=(root/'src/PixelFont.h').read_bytes()
subprocess.run(['python3',str(root/'tools/generate_font.py')],check=True)
assert (root/'src/PixelFont.h').read_bytes()==font,'Regenerate PixelFont.h'
for p in root.glob('src/*'):
 assert '0.4.5' not in p.read_text(),str(p)

# English web interface: generated header is current and every German text has a translation.
import sys,importlib.util
spec=importlib.util.spec_from_file_location('webui_en',root/'tools/webui_en.py');W=importlib.util.module_from_spec(spec);spec.loader.exec_module(W)
assert W.js() in (root/'src/webui_en.h').read_text(), 'src/webui_en.h is outdated: run tools/webui_en.py'
from html.parser import HTMLParser
body=re.sub(r'<script>.*?</script>|<style>.*?</style>','',html,flags=re.S)
js=''.join(re.findall(r'<script>(.*?)</script>',html,re.S))
need=set()
class _P(HTMLParser):
 def handle_data(self,d):
  d=' '.join(d.split())
  if d and re.search('[A-Za-zÄÖÜäöü]{2,}',d):need.add(d)
 def handle_starttag(self,t,a):
  for k,v in a:
   if k in('placeholder','title','aria-label') and v:need.add(v)
_P().feed(body)
for m in re.findall(r'`((?:[^`\\]|\\.)*)`',js):
 for t in re.findall(r'>([^<>$]*[A-Za-zÄÖÜäöü]{3,}[^<>$]*)<',m):need.add(' '.join(t.split()))
 for t in re.findall(r'placeholder="([^"$]+)"',m):need.add(t)
for l in re.findall(r"'((?:[^'\\\n]|\\.)*)'",js):
 if re.search(r'[A-Za-zÄÖÜäöüß]{3,}',l) and (' ' in l.strip() or re.search('[äöüÄÖÜß]',l)) and '<' not in l and 'http' not in l and '${' not in l:need.add(l)
neutral=re.compile(r'^(\d+ (s|min|km)|[A-Z][a-z]+/[A-Za-z_/]+|UTC|GitHub|PixelStop|v|[A-Za-z. /-]+ ·|Deutsch|English| show|hint |toast |SSID|Bus|Hostname|[.#][A-Za-z0-9_.\- #,>:]+)$')
known=set(W.EXACT)|set(W.FRAGMENTS)|set(W.LINES)
missing=[n for n in sorted(need) if n not in ui_zones and not neutral.match(n) and n not in known and not any(n.startswith(p) for p in W.PREFIX)]
assert not missing, 'Missing English web texts: '+repr(missing)
print('Release files, version, web JavaScript and generated font, English web texts: OK')
