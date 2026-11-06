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
print('Release files, version, web JavaScript and generated font: OK')
