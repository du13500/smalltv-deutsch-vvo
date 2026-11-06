#!/usr/bin/env python3
"""Destination abbreviations, town removal and crop end for the departure board."""
from pathlib import Path
import subprocess,tempfile
root=Path(__file__).resolve().parents[1]
test=r'''
#include "DestinationFormat.h"
#include <cassert>
#include <cstdio>
#include <string>
static std::string f(const char* in,bool drop=true,bool abbr=true,const char* town="Dresden"){
 char out[96];formatDestination(in,town,drop,abbr,out,sizeof out);return out;}
int main(){
 assert(f("Dresden Bühlau")=="Bühlau");
 assert(f("Dresden, Bühlau")=="Bühlau");
 assert(f("Dresden")=="Dresden");
 assert(f("Dresdener Heide")=="Dresdener Heide");
 assert(f("Dresden Bühlau",false)=="Dresden Bühlau");
 assert(f("Leipzig Hauptbahnhof")=="Leipzig Hbf");
 assert(f("Dresden Hauptbahnhof")=="Hbf");
 assert(f("Bahnhof Neustadt")=="Bf Neustadt");
 assert(f("Hauptbahnhof",true,false)=="Hauptbahnhof");
 assert(f("Bautzner Straße")=="Bautzner Str.");
 assert(f("Hauptstraße")=="Hauptstr.");
 assert(f("Bergstrasse/West")=="Bergstr./West");
 assert(f("Albertplatz")=="Albertplatz");
 assert(f("Platz der Einheit")=="Pl. der Einheit");
 assert(f("Sankt Marien")=="St. Marien");
 assert(f("Messe Dresden")=="Messe Dresden");
 assert(f("Pirna Bahnhof",false)=="Pirna Bf");
 assert(f("Pirna, Bahnhof",true,true,"Pirna")=="Bf");
 assert(f(nullptr)=="");
 char small[8];formatDestination("Hauptbahnhof Süd","",false,true,small,sizeof small);assert(std::string(small)=="Hauptba");
 assert(destCropEnd("Leipzig Hbf",8)==7);
 assert(destCropEnd("Meißen T",9)==9);
 assert(destCropEnd("A, B",2)==1);
 puts("Destinations: town removal, abbreviations, word boundaries and crop end OK");
}
'''
with tempfile.TemporaryDirectory() as tmp:
 p=Path(tmp);(p/'t.cpp').write_text(test)
 subprocess.run(['g++','-std=c++11','-Wall','-Wextra','-Werror','-I',str(root/'src'),str(p/'t.cpp'),'-o',str(p/'t')],check=True)
 subprocess.run([str(p/'t')],check=True)
