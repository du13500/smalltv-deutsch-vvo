#!/usr/bin/env python3
"""Generate compact original 5x7 Latin, Greek and Cyrillic glyphs. No downloaded font assets."""
from pathlib import Path
import unicodedata as ud
# Five-row letter bodies leave two rows for accents. Each integer is a 5-bit row.
bodies = {
'A':[14,17,31,17,17], 'C':[14,17,16,17,14], 'D':[30,17,17,17,30],
'E':[31,16,30,16,31], 'G':[14,16,23,17,14], 'H':[17,17,31,17,17],
'I':[14,4,4,4,14], 'J':[7,2,2,18,12], 'K':[17,18,28,18,17],
'L':[16,16,16,16,31], 'N':[17,25,21,19,17], 'O':[14,17,17,17,14],
'R':[30,17,30,18,17], 'S':[15,16,14,1,30], 'T':[31,4,4,4,4],
'U':[17,17,17,17,14], 'W':[17,17,21,21,10], 'Y':[17,10,4,4,4],
'Z':[31,2,4,8,31],
'a':[14,1,15,17,15], 'c':[14,16,16,16,14], 'd':[1,1,15,17,15],
'e':[14,17,31,16,14], 'g':[15,17,15,1,14], 'h':[16,16,30,17,17],
'i':[4,0,12,4,14], 'j':[2,0,2,18,12], 'k':[16,18,28,18,17],
'l':[12,4,4,4,14], 'n':[30,17,17,17,17], 'o':[14,17,17,17,14],
'r':[22,25,16,16,16], 's':[15,16,14,1,30], 't':[8,30,8,9,6],
'u':[17,17,17,19,13], 'w':[17,17,21,21,10], 'y':[17,17,15,1,14],
'z':[31,2,4,8,31],
}
bodies.update({'B':[30,17,30,17,30],'F':[31,16,30,16,16],'M':[17,27,21,17,17],'P':[30,17,30,16,16],'Q':[14,17,17,19,15],'V':[17,17,17,10,4],'X':[17,10,4,10,17], 'b':[16,16,30,17,30],'f':[6,8,28,8,8],'m':[0,26,21,21,21],'p':[30,17,30,16,16],'q':[15,17,15,1,1],'v':[17,17,17,10,4],'x':[17,10,4,10,17]})
accents={0x300:[8,4],0x301:[2,4],0x302:[4,10],0x303:[10,5],0x304:[14,0],0x306:[17,14],0x307:[4,0],0x308:[10,0],0x30A:[6,6],0x30B:[5,10],0x30C:[10,4]}
glyphs={};compositions=[]
for cp in list(range(0xC0,0x250))+list(range(0x1E00,0x1F00)):
 d=ud.normalize('NFD',chr(cp))
 if len(d)!=2 or d[0] not in bodies:continue
 base,mark=d[0],ord(d[1]);body=bodies[base][:]
 if mark in accents:
  if base=='i':body=[0,12,4,4,14]
  if base=='j':body=[0,2,2,18,12]
  rows=accents[mark]+body
 elif mark in (0x326,0x327):rows=body+[4,8]
 elif mark==0x323:rows=body+[0,4]
 elif mark==0x331:rows=body+[0,31]
 elif mark==0x328:rows=body+[2,3]
 else:continue
 glyphs[cp]=rows;compositions.append((ord(base),mark,cp))
# Preserve v0.4.5's German glyphs exactly; add letters without canonical decomposition.
glyphs.update({
0xE4:[10,0,14,1,15,17,15],0xF6:[10,0,14,17,17,17,14],0xFC:[10,0,17,17,17,19,13],
0xC4:[10,14,17,31,17,17,17],0xD6:[10,14,17,17,17,17,14],0xDC:[10,17,17,17,17,17,14],
0xDF:[14,17,30,17,17,30,16],0xB0:[6,9,9,6,0,0,0],
0xC6:[15,20,20,31,20,20,23],0xE6:[0,0,26,5,15,20,15],
0x152:[14,21,20,22,20,21,14],0x153:[0,0,10,21,23,20,10],
0xD8:[14,19,21,21,21,25,14],0xF8:[0,0,14,19,21,25,14],
0x141:[16,16,18,28,16,16,31],0x142:[12,4,6,12,4,4,14],
0xD0:[14,9,9,29,9,9,14],0xF0:[10,4,10,1,15,17,14],
0x110:[14,9,9,29,9,9,14],0x111:[2,7,2,14,18,18,14],
0xDE:[16,30,17,17,30,16,16],0xFE:[16,16,30,17,17,30,16],
0x126:[17,31,17,31,17,17,17],0x127:[16,28,16,30,17,17,17],
0x131:[0,0,12,4,4,4,14],0x138:[0,0,17,18,28,18,17],
0x14A:[30,17,17,17,17,1,2],0x14B:[0,0,30,17,17,1,2],
0x166:[31,4,4,14,4,4,4],0x167:[8,30,8,28,8,9,6],
0x17F:[6,8,8,28,8,8,8],0x1E9E:[14,17,22,17,21,19,16],
0x2665:[0,10,31,31,14,4,0],
})

# Modern Greek and Cyrillic alphabets. Original pixel designs, same 6x8 cell.
# Cyrillic lowercase retains its own forms instead of transliterating to Latin.
greek_upper={
'Α':[14,17,17,31,17,17,17], 'Β':[30,17,17,30,17,17,30],
'Γ':[31,16,16,16,16,16,16], 'Δ':[4,10,10,17,17,17,31],
'Ε':[31,16,16,30,16,16,31], 'Ζ':[31,1,2,4,8,16,31],
'Η':[17,17,17,31,17,17,17], 'Θ':[14,17,17,31,17,17,14],
'Ι':[14,4,4,4,4,4,14], 'Κ':[17,18,20,24,20,18,17],
'Λ':[4,10,10,17,17,17,17], 'Μ':[17,27,21,21,17,17,17],
'Ν':[17,25,25,21,19,19,17], 'Ξ':[31,0,0,14,0,0,31],
'Ο':[14,17,17,17,17,17,14], 'Π':[31,17,17,17,17,17,17],
'Ρ':[30,17,17,30,16,16,16], 'Σ':[31,16,8,4,8,16,31],
'Τ':[31,4,4,4,4,4,4], 'Υ':[17,17,10,4,4,4,4],
'Φ':[4,14,21,21,21,14,4], 'Χ':[17,17,10,4,10,17,17],
'Ψ':[21,21,21,14,4,4,4], 'Ω':[14,17,17,17,10,10,27],
}
greek_lower={
'α':[0,0,13,18,18,18,13], 'β':[6,9,9,14,9,9,22],
'γ':[0,0,17,17,10,4,4], 'δ':[6,8,6,9,17,17,14],
'ε':[0,0,14,16,12,16,14], 'ζ':[31,2,4,8,16,14,1],
'η':[0,0,22,25,17,17,1], 'θ':[14,17,17,31,17,17,14],
'ι':[0,0,4,4,4,4,6], 'κ':[0,0,17,18,28,18,17],
'λ':[8,4,4,10,10,17,17], 'μ':[0,0,17,17,17,30,16],
'ν':[0,0,17,17,18,20,8], 'ξ':[14,16,12,16,14,1,6],
'ο':[0,0,14,17,17,17,14], 'π':[0,0,31,10,10,10,11],
'ρ':[0,14,17,17,30,16,16], 'σ':[0,0,15,18,18,18,12],
'τ':[0,0,31,4,4,4,6], 'υ':[0,0,17,17,17,17,14],
'φ':[0,4,14,21,21,14,4], 'χ':[0,0,17,10,4,10,17],
'ψ':[0,0,21,21,21,14,4], 'ω':[0,0,17,17,21,21,10],
'ς':[0,0,14,16,16,14,1],
}
cyrillic_upper={
'А':[14,17,17,31,17,17,17], 'Б':[31,16,16,30,17,17,30],
'В':[30,17,17,30,17,17,30], 'Г':[31,16,16,16,16,16,16],
'Д':[6,10,10,10,17,31,17], 'Е':[31,16,16,30,16,16,31],
'Ж':[21,21,14,4,14,21,21], 'З':[14,17,1,6,1,17,14],
'И':[17,17,19,21,25,17,17], 'Й':[10,4,17,19,21,25,17],
'К':[17,18,20,24,20,18,17], 'Л':[7,9,9,9,9,17,17],
'М':[17,27,21,21,17,17,17], 'Н':[17,17,17,31,17,17,17],
'О':[14,17,17,17,17,17,14], 'П':[31,17,17,17,17,17,17],
'Р':[30,17,17,30,16,16,16], 'С':[14,17,16,16,16,17,14],
'Т':[31,4,4,4,4,4,4], 'У':[17,17,17,15,1,17,14],
'Ф':[4,14,21,21,21,14,4], 'Х':[17,17,10,4,10,17,17],
'Ц':[18,18,18,18,18,31,1], 'Ч':[17,17,17,15,1,1,1],
'Ш':[21,21,21,21,21,21,31], 'Щ':[21,21,21,21,21,31,1],
'Ъ':[24,8,8,14,9,9,14], 'Ы':[17,17,17,25,21,21,25],
'Ь':[16,16,16,30,17,17,30], 'Э':[14,17,1,7,1,17,14],
'Ю':[18,21,21,29,21,21,18], 'Я':[15,17,17,15,5,9,17],
}
cyrillic_lower={
'а':[0,0,14,1,15,17,15], 'б':[3,12,16,30,17,17,14],
'в':[0,0,30,17,30,17,30], 'г':[0,0,31,16,16,16,16],
'д':[0,0,6,10,10,31,17], 'е':[0,0,14,17,31,16,14],
'ж':[0,0,21,14,4,14,21], 'з':[0,0,30,1,6,1,30],
'и':[0,0,17,19,21,25,17], 'й':[10,4,17,19,21,25,17],
'к':[0,0,17,18,28,18,17], 'л':[0,0,7,9,9,17,17],
'м':[0,0,17,27,21,17,17], 'н':[0,0,17,17,31,17,17],
'о':[0,0,14,17,17,17,14], 'п':[0,0,31,17,17,17,17],
'р':[0,0,30,17,30,16,16], 'с':[0,0,14,16,16,16,14],
'т':[0,0,31,4,4,4,4], 'у':[0,0,17,17,15,1,14],
'ф':[4,14,21,21,14,4,4], 'х':[0,0,17,10,4,10,17],
'ц':[0,0,18,18,18,31,1], 'ч':[0,0,17,17,15,1,1],
'ш':[0,0,21,21,21,21,31], 'щ':[0,0,21,21,21,31,1],
'ъ':[0,0,24,8,14,9,14], 'ы':[0,0,17,17,25,21,25],
'ь':[0,0,16,16,30,17,30], 'э':[0,0,14,1,7,1,14],
'ю':[0,0,18,21,29,21,18], 'я':[0,0,15,17,15,5,9],
}
extra={
'Ђ':[28,8,14,9,9,9,2], 'ђ':[8,28,8,14,9,9,2],
'Є':[14,17,16,28,16,17,14], 'є':[0,0,14,16,28,16,14],
'Ѕ':[15,16,16,14,1,1,30], 'ѕ':[0,0,15,16,14,1,30],
'І':[14,4,4,4,4,4,14], 'і':[4,0,12,4,4,4,14],
'Ј':[7,2,2,2,2,18,12], 'ј':[2,0,6,2,2,18,12],
'Љ':[8,24,24,30,25,25,30], 'љ':[0,0,8,24,30,25,30],
'Њ':[20,20,20,30,21,21,22], 'њ':[0,0,20,20,30,21,22],
'Ћ':[28,8,14,9,9,9,9], 'ћ':[8,28,8,14,9,9,9],
'Џ':[17,17,17,17,17,31,4], 'џ':[0,0,17,17,17,31,4],
'Ґ':[1,31,16,16,16,16,16], 'ґ':[0,1,31,16,16,16,16],
'Ә':[14,17,1,15,17,17,14], 'ә':[0,0,14,1,15,17,14],
'Ғ':[31,16,16,28,16,16,16], 'ғ':[0,0,31,16,28,16,16],
'Қ':[18,20,24,20,18,18,1], 'қ':[0,0,18,20,24,18,1],
'Ң':[18,18,18,30,18,18,1], 'ң':[0,0,18,18,30,18,1],
'Ө':[14,17,17,31,17,17,14], 'ө':[0,0,14,17,31,17,14],
'Ұ':[17,17,10,4,14,4,4], 'ұ':[0,0,17,10,4,14,4],
'Ү':[17,17,10,4,4,4,4], 'ү':[0,0,17,10,4,4,4],
'Һ':[16,16,16,30,17,17,17], 'һ':[16,16,16,30,17,17,17],
'Җ':[21,21,14,4,14,21,1], 'җ':[0,0,21,14,4,21,1],
'Ӏ':[14,4,4,4,4,4,14], 'ӏ':[4,4,4,4,4,4,4],
'Ə':[14,17,1,15,17,17,14], 'ə':[0,0,14,1,15,17,14],
'Ĳ':[23,5,5,5,5,21,10], 'ĳ':[5,0,13,5,5,21,10],
'Ŀ':[16,16,16,20,16,16,31], 'ŀ':[12,4,4,5,4,4,14],
'ĸ':[0,0,17,18,28,18,17], 'ŉ':[16,0,6,9,9,9,9],
'‘':[4,8,8,0,0,0,0], '’':[4,4,8,0,0,0,0],
'‚':[0,0,0,0,4,4,8], '“':[10,20,20,0,0,0,0],
'”':[10,10,20,0,0,0,0], '„':[0,0,0,0,10,10,20],
'–':[0,0,0,31,0,0,0], '—':[0,0,0,31,0,0,0],
'…':[0,0,0,0,0,0,21], '«':[0,0,10,20,10,0,0],
'»':[0,0,10,5,10,0,0], '№':[16,24,20,18,17,3,3],
'€':[7,8,30,8,30,8,7], '£':[6,9,8,28,8,8,31],
}
for group in (greek_upper,greek_lower,cyrillic_upper,cyrillic_lower,extra):
 for ch,rows in group.items():glyphs[ord(ch)]=rows
# NFC and decomposed forms must occupy one display cell (e.g. Ё, Й, Ї, ά, ΐ).
accent_bodies={ord(k):v for k,v in {'Α':bodies['A'],'Ε':bodies['E'],'Η':bodies['H'],'Ι':bodies['I'],'Ο':bodies['O'],'Υ':bodies['Y'],'Ω':[14,17,17,10,27],'Е':bodies['E'],'Г':[31,16,16,16,16],'К':bodies['K'],'И':[17,19,21,25,17],'У':[17,17,15,1,14]}.items()}
for length in (2,3):
 for cp in list(range(0x370,0x400))+list(range(0x400,0x460)):
  d=ud.normalize('NFD',chr(cp))
  if len(d)!=length:continue
  prefix=ud.normalize('NFC',d[:-1]);mark=ord(d[-1])
  if len(prefix)!=1 or ord(prefix) not in glyphs or mark not in accents:continue
  base=ord(prefix);rows=glyphs[base][:]
  if length==2:rows=accents[mark]+accent_bodies.get(base,rows[-5:])
  else:rows=[rows[i]|accents[mark][i] for i in range(2)]+rows[-5:]
  # Hand-authored й/Й stay exactly as designed.
  if cp not in (0x419,0x439):glyphs[cp]=rows
  compositions.append((base,mark,cp))
for cp,rows in glyphs.items():
 assert len(rows)==7 and all(0<=row<=31 for row in rows),hex(cp)
compositions=sorted(set(compositions))

header='''// Generated by tools/generate_font.py. Original 5x7 Latin/Greek/Cyrillic pixel glyphs, MIT (Copyright 2026 du13500).
#pragma once
#include <stdint.h>
#include <stddef.h>
#ifdef ARDUINO
#include <pgmspace.h>
#else
#define PROGMEM
#define pgm_read_byte(p) (*(const uint8_t*)(p))
#define pgm_read_dword(p) (*(const uint32_t*)(p))
#endif
struct PixelGlyph { uint32_t cp; uint8_t rows[7]; };
static const uint8_t missingGlyph[7] PROGMEM={31,17,17,17,17,17,31};
static const PixelGlyph pixelGlyphs[] PROGMEM={
'''
for cp,rows in sorted(glyphs.items()):header+=f'  {{0x{cp:04X},{{'+','.join(map(str,rows))+'}}, // '+chr(cp)+'\n'
header+='''};
static inline uint8_t pixelRow(const uint8_t* p) { return pgm_read_byte(p); }
static inline const uint8_t* specialGlyph(uint32_t cp) {
  size_t lo=0,hi=sizeof(pixelGlyphs)/sizeof(pixelGlyphs[0]);
  while(lo<hi){size_t mid=lo+(hi-lo)/2;uint32_t key=pgm_read_dword(&pixelGlyphs[mid].cp);
    if(key==cp)return pixelGlyphs[mid].rows;
    if(key<cp)lo=mid+1;else hi=mid;
  }
  return nullptr;
}
struct PixelComposition { uint32_t base,mark,cp; };
static const PixelComposition pixelCompositions[] PROGMEM={
'''
for b,m,c in compositions:header+=f'  {{0x{b:X},0x{m:X},0x{c:X}}},\n'
header+='''};
// Decode UTF-8 without consuming the ASCII byte following a broken sequence.
static inline bool decodeUtf8(const char* s,size_t n,size_t& i,uint32_t& cp) {
  if(i>=n)return false;
  uint8_t lead=(uint8_t)s[i++];
  if(lead<0x80){cp=lead;return true;}
  uint8_t count=0;uint32_t min=0;
  if(lead>=0xC2&&lead<=0xDF){count=1;cp=lead&31;min=0x80;}
  else if(lead>=0xE0&&lead<=0xEF){count=2;cp=lead&15;min=0x800;}
  else if(lead>=0xF0&&lead<=0xF4){count=3;cp=lead&7;min=0x10000;}
  else {cp=0xFFFD;return true;}
  for(uint8_t j=0;j<count;j++){
    if(i>=n||((uint8_t)s[i]&0xC0)!=0x80){cp=0xFFFD;return true;}
    cp=(cp<<6)|((uint8_t)s[i++]&63);
  }
  if(cp<min||cp>0x10FFFF||(cp>=0xD800&&cp<=0xDFFF))cp=0xFFFD;
  return true;
}
static inline bool heartCodepoint(uint32_t cp) {
  return cp==0x2661||cp==0x2665||cp==0x2764||cp==0x2765||cp==0x2763||
    cp==0x1F49F||(cp>=0x1F493&&cp<=0x1F49E)||cp==0x1F5A4||
    cp==0x1F90D||cp==0x1F90E||(cp>=0x1FA75&&cp<=0x1FA77);
}
static inline bool nextDisplayCodepoint(const char* s,size_t n,size_t& i,uint32_t& cp) {
  // Selectors choose emoji/text presentation; they do not occupy a display cell.
  do {if(!decodeUtf8(s,n,i,cp))return false;} while(cp==0xFE0E||cp==0xFE0F);
  bool heart=heartCodepoint(cp);
  if(heart)cp=0x2665;
  while(i<n){
    size_t next=i;uint32_t mark=0;decodeUtf8(s,n,next,mark);
    if(mark==0xFE0E||mark==0xFE0F){i=next;continue;}
    // Heart on fire / mending heart are rendered as one ordinary pixel heart.
    if(heart&&mark==0x200D){
      size_t end=next;uint32_t joined=0;
      if(decodeUtf8(s,n,end,joined)&&(joined==0x1F525||joined==0x1FA79)){i=end;continue;}
    }
    if(mark<0x300||mark>0x36F)break;
    bool composed=false;
    for(size_t k=0;k<sizeof(pixelCompositions)/sizeof(pixelCompositions[0]);k++){
      const auto& c=pixelCompositions[k];
      if(pgm_read_dword(&c.base)==cp&&pgm_read_dword(&c.mark)==mark){cp=pgm_read_dword(&c.cp);i=next;composed=true;break;}
    }
    if(!composed)break;
  }
  return true;
}
'''
Path(__file__).resolve().parents[1].joinpath('src/PixelFont.h').write_text(header)
print(f'{len(glyphs)} glyphs, {len(compositions)} composed variants')
