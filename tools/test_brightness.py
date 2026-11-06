#!/usr/bin/env python3
"""Exercise the actual brightness function, including night transitions."""
from pathlib import Path
import re,subprocess,tempfile
root=Path(__file__).resolve().parents[1]
s=(root/'src/main.cpp').read_text()
setup=s[s.index('void setup()'):s.index('void loop()')]
assert setup.index('enablePhaseLockedWaveform();')<setup.index('analogWriteRange(255);')<setup.index('applyBrightness();')
a=s.index('static bool nightWindow(');b=s.index('static bool modeEnabled(',a)
code=s[a:b].replace('time(nullptr)','fakeNow')
with tempfile.TemporaryDirectory() as tmp:
 p=Path(tmp);f=p/'test.cpp'
 f.write_text(r'''#include <cstdint>
#include <ctime>
#include <cassert>
#define TFT_BL 5
struct {uint8_t brightness=1,nightBrightness=1;bool nightEnabled=false;uint16_t nightStart=1320,nightEnd=420;}cfg;
static time_t fakeNow=0;static int writes=0,last=-1;
void analogWrite(int pin,int duty){assert(pin==5);writes++;last=duty;}
'''+code+r'''
int main(){
 applyBrightness();assert(writes==1&&last==253);
 for(int i=0;i<100;i++)applyBrightness();assert(writes==1);
 cfg.brightness=20;applyBrightness();assert(writes==2&&last==204);
 cfg.nightEnabled=true;fakeNow=1735772400; // 2025-01-01 23:00 UTC
 applyBrightness();assert(writes==3&&last==253);applyBrightness();assert(writes==3);
 fakeNow+=8*3600;applyBrightness();assert(writes==4&&last==204);
 cfg.brightness=100;applyBrightness();assert(writes==5&&last==0);
 cfg.brightness=120;applyBrightness();assert(writes==5);
}
''')
 subprocess.run(['g++','-std=c++11',str(f),'-o',str(p/'test')],check=True)
 subprocess.run([str(p/'test')],check=True,env={'TZ':'UTC'})
print('Actual brightness: first PWM, unchanged values, night transitions and clamping OK')
