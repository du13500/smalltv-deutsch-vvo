#!/usr/bin/env python3
"""Compile real display functions against a recorder, including partial radar updates."""
from pathlib import Path
import re,subprocess,tempfile,os
root=Path(__file__).resolve().parents[1]
s=(root/'src/main.cpp').read_text()
def fn(name):
 m=re.search(r'^static [^\n]+\b'+name+r'\([^\n]*\) \{',s,re.M);assert m,name
 i=s.index('{',m.start());depth=1;j=i+1
 while depth:
  if s[j]=='{':depth+=1
  if s[j]=='}':depth-=1
  j+=1
 return s[m.start():j]
def struct(name,file):
 return 'struct '+name+' {'+(root/'src'/file).read_text().split('struct '+name+' {',1)[1].split('};',1)[0]+'};'
pre=r'''
#include <string>
#include <vector>
#include <cassert>
#include <cstring>
#include <cstdio>
#include <cmath>
#include <algorithm>
#include <ctime>
#include "PixelFont.h"
#include "FlightRoute.h"
#include "AircraftLabel.h"
#include "WeatherVisual.h"
#include "DestinationFormat.h"
#define PSTR(x) (x)
#define FPSTR(x) (x)
#define PGM_P const char*
struct String:std::string {
 using std::string::string;
 String(const std::string& s):std::string(s){}
 String(int n):std::string(std::to_string(n)){}
 String(float n,int digits){char b[48];snprintf(b,sizeof(b),"%.*f",digits,(double)n);assign(b);}
 String substring(size_t a,size_t b)const{return substr(a,b-a);}
 int indexOf(const String& q,int start=0)const{auto p=find(q,start);return p==npos?-1:(int)p;}
 int indexOf(char q,int start=0)const{auto p=find(q,start);return p==npos?-1:(int)p;}
};
struct Draw {String text;int y,size,x,color;Draw(String t,int yy,int ss,int xx=-1,int cc=-1):text(t),y(yy),size(ss),x(xx),color(cc){}};
struct Rect {int x,y,w,h,color;};
struct Circle {int x,y,r,color;};
struct Line {int x,y,x2,y2,color;};
std::vector<Draw> draws;std::vector<Rect> rects;std::vector<Circle> circles;std::vector<Line> lines;
String footer;int fullScreens=0;
struct Gfx {
 int x=0,y=0,size=1;
 void fillScreen(int){fullScreens++;} void setTextSize(int n){size=n;} void setTextColor(int){}
 void setCursor(int a,int b){x=a;y=b;} void print(const String& t){draws.push_back({t,y,size});}
 void write(uint8_t){x+=6*size;} void fillRect(int x,int y,int w,int h,int c){rects.push_back({x,y,w,h,c});}
 void fillCircle(int x,int y,int r,int c){circles.push_back({x,y,r,c});}
 void drawLine(int x,int y,int x2,int y2,int c){lines.push_back({x,y,x2,y2,c});}
 void drawPixel(int x,int y,int c){fillRect(x,y,1,1,c);}
 void drawFastHLine(int x,int y,int w,int c){drawLine(x,y,x+w-1,y,c);}
} display;
Gfx* gfx=&display;
const double PI=3.14159265358979323846;
const int C_BLACK=0,C_WHITE=1,C_YELLOW=2,C_GRAY=3,C_RED=4,C_GREEN=5,C_CYAN=6,C_DGRAY=7,TFT_W=240;
void centerText(const String& s,int y,int size,int color){draws.push_back({s,y,size,-1,color});}
void drawFooter(const String& s,bool=true){footer=s;}
void printDisplayTextAt(int,int,const String& s,uint8_t size,uint16_t);
String displayAscii(String s){return s;}
String flightStatusGerman(const char*){return "STATUS";}
using std::min;using std::max;
template<class T>T constrain(T x,T a,T b){return max(a,min(b,x));}
const int COUNTDOWN_DAYS=0;
struct CountdownSlot{String title,titleColor="#ffffff",countColor="#ffffff";uint8_t type=1;};
enum Language:uint8_t{LANG_DE=0,LANG_EN=1};
struct Settings{uint8_t lang=LANG_DE;String stopPlace;String trackedFlightNo,aviationstackKey,planePlace,planeDisplayName,stopName,stopId,weatherDisplayName,weatherCity,timeZone;bool planeShowPlace=false,planeShowAge=true,departureCompactDestinations=false,departureAbbreviate=true,departureDropTown=true,weatherTempColor=false;uint16_t planePlaceToggleSec=10;uint8_t departureCount=5;CountdownSlot countdown[4];} cfg;
static String tr(PGM_P de,PGM_P en){return String(cfg.lang==LANG_EN?en:de);}
#define TR(de,en) tr(PSTR(de),PSTR(en))
String lastCountdownRendered[4];uint8_t departurePage=0;
uint16_t color565(const String&){return C_WHITE;}
String timerValue="00:01:09";
String countdownValue(const CountdownSlot&){return timerValue;}
float displayTempValue(float c){return c;}
uint16_t weatherTempColor565(float){return C_WHITE;}
String weatherUnit(){return "°C";}
const char* weatherLabel(int c){return weatherVisual(c).label;}
String timeHHMM(){return timerValue;}
void drawFooterPageArrow(bool){}
bool planePlacePhase=false;
'''
pre+='\n'+struct('TrackedFlightSnapshot','TrackFlight.h')+'\n'+struct('FlightSnapshot','Flight.h')+'\n'+struct('WeatherSnapshot','Weather.h')+'\n'+struct('VvoDeparture','Vvo.h')
pre+=r'''
void printDisplayTextAt(int x,int y,const String& s,uint8_t size,uint16_t){draws.push_back({s,y,size,x});}
struct VisibleField {String text;int16_t x=0,y=0,w=0,h=0,tx=0,ty=0;uint16_t color=0;uint8_t size=0;bool valid=false;};
VisibleField visibleFields[32];bool viewFresh=true;
WeatherSnapshot weather={};VvoDeparture departures[10]={};uint8_t departureN=0;bool departureError=false;
const WeatherSnapshot& weatherData(){return weather;}
uint8_t vvoCount(){return departureN;}
const VvoDeparture& vvoAt(uint8_t i){return departures[i];}
bool vvoError(){return departureError;}
static bool departureUnserved=false;bool vvoNotServed(){return departureUnserved;}
TrackedFlightSnapshot data={};FlightSnapshot plane={};FlightSnapshot lastPlaneRendered={};bool planeRendered=false;
const TrackedFlightSnapshot& trackedFlightData(){return data;}
const FlightSnapshot& flightData(){return plane;}
uint32_t clockMs=100000,lastOkMs=65000;bool providerPaused=false;
uint32_t millis(){return clockMs;}uint32_t flightLastOkMs(){return lastOkMs;}bool flightProviderPaused(){return providerPaused;}static int lastHttp=0;int flightLastHttp(){return lastHttp;}static bool adsbLolAccessDenied(int c){return c==401||c==403||c==426;}
String trackedFlightDuration(const TrackedFlightSnapshot&){return "01:30";}
void clearDraws(){draws.clear();rects.clear();circles.clear();lines.clear();fullScreens=0;footer="not drawn";}
'''
post=r'''
int main(){
 for(const char* code:{"E170","E175","E75L","E75S","E190","E195","E290","E295","B38M","B39M","A21N"}){
  FlightSnapshot sample={};strcpy(sample.typeCode,code);
  String first=compactType(sample);assert(first==aircraftDisplayLabel(code));
  strcpy(sample.typeName,"A long metadata description with additional variant information");
  assert(compactType(sample)==first);assert(textW(first,2)<=224);
 }
 cfg.planeShowPlace=true;cfg.planeDisplayName="WUNSCHTEXT";cfg.planePlaceToggleSec=0;planePlacePhase=false;clearDraws();drawPlaneHeader();assert(draws.back().text=="WUNSCHTEXT");
 cfg={};
 FlightRoute route={};
 for(const char* text:{"OSL-IST","LEJ-BLR-HKG","AAA-BBB-CCC-DDD","AAA-BBB-CCC-DDD-EEE-FFF-GGG-HHH","AAA-BBB-CCC-DDD-EEE-FFF-GGG-HHH-III"}){
   assert(parseFlightRoute(text,route));clearDraws();
   drawFlightRoute(route,route.airports[0],route.airports[route.count-1],105);
   for(auto d:draws){assert(textW(d.text,d.size)<=224);assert(d.y>=105&&d.y+d.size*7<=133);}
   if(route.count==2){assert(draws.size()==1&&draws[0].text=="OSL > IST"&&draws[0].size==3&&draws[0].y==105);}
 }
 cfg={};clearDraws();drawTrackedFlight();assert(footer=="");
 bool hint=false;for(auto d:draws)if(d.text=="Webinterface eingeben")hint=true;assert(hint);
 cfg.trackedFlightNo="AF1234";footer="not drawn";drawTrackedFlight();assert(footer=="");
 cfg.aviationstackKey="fixture";footer="";drawTrackedFlight();assert(footer=="LADE FLUGDATEN");
 data.error=true;footer="";drawTrackedFlight();assert(footer=="TRACKER FEHLER");
 data.error=false;data.notFound=true;footer="";drawTrackedFlight();assert(footer=="FLUG NICHT GEFUNDEN");
 data.valid=true;data.notFound=false;strcpy(data.origin,"OSL");strcpy(data.destination,"IST");
 strcpy(data.arrActual,"N/A");clearDraws();drawTrackedFlight();assert(footer=="AVIATIONSTACK + ADSB.LOL");
 assert(draws[1].text=="OSL > IST"&&draws[1].y==36&&draws[1].size==3);
 clearDraws();drawTrackedFlight(false);assert(fullScreens==0&&draws.empty()&&rects.empty());
 strcpy(data.depScheduled,"10:00");clearDraws();drawTrackedFlight(false);assert(fullScreens==0&&draws.size()==1);
 data.error=true;footer="";drawTrackedFlight();assert(footer=="LETZTER STAND");
 // Radar: same ICAO -> only changed visible fields; new aircraft -> complete redraw.
 plane.valid=true;strcpy(plane.hex,"abc123");strcpy(plane.callsign,"THY1JU");strcpy(plane.registration,"TC-JRS");
 strcpy(plane.origin,"OSL");strcpy(plane.destination,"IST");plane.distanceKm=12.8f;plane.altitudeFt=33000;plane.altitudeKnown=true;
 clearDraws();drawPlane();assert(fullScreens==1&&footer=="ADSB.LOL LIVE");
 clearDraws();drawPlane(false);assert(fullScreens==0&&rects.empty()&&draws.empty()&&footer=="not drawn");
 plane.distanceKm=9.7f;clearDraws();drawPlane(false);assert(fullScreens==0&&rects.size()==1&&draws.size()==1);
 assert(rects[0].x==8&&rects[0].y==190&&rects[0].w==112&&rects[0].h==24);assert(draws[0].text=="9.7 km");
 plane.altitudeFt=30000;clearDraws();drawPlane(false);assert(fullScreens==0&&rects.size()==1&&rects[0].x==120);
 strcpy(plane.airline,"Turkish Airlines");clearDraws();drawPlane(false);assert(fullScreens==0&&rects.size()==1&&rects[0].y==78);
 parseFlightRoute("OSL-LEJ-IST",plane.route);clearDraws();drawPlane(false);assert(fullScreens==0&&rects.size()==1&&rects[0].y==102);
 plane.error=true;clearDraws();drawPlane(false);assert(fullScreens==0&&footer=="LETZTER STAND");
 strcpy(plane.hex,"def456");clearDraws();drawPlane(false);assert(fullScreens==1);
 plane.valid=false;clearDraws();drawPlane(false);assert(fullScreens==1&&footer=="ABRUF FEHLGESCHLAGEN");
 plane.error=false;clearDraws();drawPlane(false);assert(fullScreens==1&&footer=="ADSB.LOL");
 // Pause ticks touch only the number box; all aircraft content stays frozen.
 plane.valid=true;plane.error=true;providerPaused=true;lastOkMs=65000;clockMs=100000;
 clearDraws();drawPlane(false);assert(footer=="ABRUFPAUSE");assert(8+textW(footer+" ",1)==74);
 viewFresh=false;clockMs=101000;clearDraws();drawPlaneAge();
 assert(fullScreens==0&&rects.size()==1&&rects[0].x==74&&rects[0].y==223&&rects[0].w==48&&rects[0].h==14);
 assert(draws.size()==1&&draws[0].x==74&&draws[0].text=="36s"&&footer=="not drawn");
 clearDraws();drawPlaneAge();assert(draws.empty()&&rects.empty());
 for(uint32_t age:{9,10,99,100,999,1000,4294967}){clockMs=lastOkMs+age*1000UL;clearDraws();drawPlaneAge();assert(fullScreens==0&&rects.size()==1&&rects[0].x==74&&rects[0].w==48);assert(draws.size()==1&&draws[0].x==74&&74+textW(draws[0].text,1)<=194);}
 lastOkMs=0xfffffff0U;clockMs=34984;clearDraws();drawPlaneAge();assert(draws[0].text=="35s");
 providerPaused=false;plane.error=false;clearDraws();drawPlane(false);assert(fullScreens==0&&footer=="ADSB.LOL LIVE");
 clearDraws();drawPlaneAge();assert(rects.empty()&&draws.empty());
 // Successful reception and ordinary stale states count too, directly after label.
 clockMs=lastOkMs+4000;clearDraws();drawPlaneAge();assert(draws.size()==1&&draws[0].text=="4s"&&draws[0].x==92&&rects.size()==1);
 plane.error=true;clockMs=lastOkMs+16000;clearDraws();drawPlane(false);assert(footer=="LETZTER STAND");assert(draws.back().text=="16s"&&draws.back().x==92);
 cfg.planeShowAge=false;clearDraws();drawPlane(false);assert(footer=="LETZTER STAND");for(auto d:draws)assert(d.text!="16s");
 clockMs+=1000;clearDraws();drawPlaneAge();assert(draws.empty()&&rects.empty());
 providerPaused=true;clearDraws();drawPlane(false);assert(footer=="ABRUFPAUSE"&&draws.empty());
 cfg.planeShowAge=true;clearDraws();drawPlane(false);assert(draws.back().text=="17s"&&draws.back().x==74);
 plane.valid=false;clearDraws();drawPlaneAge();assert(draws.empty()&&rects.empty());
 providerPaused=false;plane.valid=true;plane.error=false;
 // Weather motifs: night partly clouded must have a moon; fog has no cloud.
 clearDraws();drawWeatherIcon(2,false);assert(circles.size()==5&&circles[0].color==C_WHITE&&circles[1].color==C_BLACK);
 clearDraws();drawWeatherIcon(45,true);assert(circles.empty()&&rects.size()==4&&lines.empty());
 clearDraws();drawWeatherIcon(51,true);auto drizzle=lines.size();assert(drizzle==2);
 clearDraws();drawWeatherIcon(55,true);assert(lines.size()==6);
 clearDraws();drawWeatherIcon(61,true);assert(lines[0].y2-lines[0].y==10);
 clearDraws();drawWeatherIcon(71,true);assert(lines.size()==8);
 clearDraws();drawWeatherIcon(77,true);assert(circles.size()==5&&circles[3].r==1);
 clearDraws();drawWeatherIcon(96,true);assert(circles.size()==5&&circles[3].r==2&&lines.size()==3);
 for(int code:{0,1,2,3,45,48,51,53,55,56,57,61,63,65,66,67,71,73,75,77,80,81,82,85,86,95,96,99})for(bool day:{false,true}){
   clearDraws();drawWeatherIcon(code,day);
   for(auto q:rects)assert(q.x>=0&&q.x+q.w<=240&&q.y>=22&&q.y+q.h<=88);
   for(auto q:circles)assert(q.x-q.r>=0&&q.x+q.r<240&&q.y-q.r>=22&&q.y+q.r<=87);
   for(auto q:lines)assert(q.x>=0&&q.x2>=0&&q.x<240&&q.x2<240&&q.y>=22&&q.y2>=22&&q.y<=87&&q.y2<=87);
 }
 // Actual glyph renderer: one cell per Greek/Cyrillic letter; width/crop agree.
 String title="Київ Αθήνα";assert(glyphCount(title)==10&&textW(title,2)==120);
 assert(crop("Москва",4)=="Мос.");clearDraws();drawSpecialGlyph(0,0,2,specialGlyph(0x416),C_WHITE);assert(!rects.empty());
 for(auto q:rects)assert(q.x+q.w<=10&&q.y+q.h<=14);
 plane.onGround=true;assert(planeAltitude(plane)=="GND");plane.onGround=false;plane.altitudeKnown=false;assert(planeAltitude(plane)=="N/A");plane.altitudeKnown=true;plane.altitudeFt=0;assert(planeAltitude(plane)=="0 m");
 strcpy(plane.typeName,"Reims-Cessna F172N Skyhawk");strcpy(plane.typeCode,"C172");assert(compactType(plane)=="Cessna F172N");
 strcpy(plane.typeName,"A very long unrecognized manufacturer and model");assert(compactType(plane)=="C172");
 // Weather: an identical refresh draws nothing; changed temperature restores its unit.
 cfg.weatherCity="Dresden";weather.valid=true;weather.code=2;weather.isDay=false;weather.currentC=15;weather.highC=20;weather.lowC=8;weather.rainPct=10;
 clearDraws();drawWeather();assert(fullScreens==1);clearDraws();drawWeather(false);assert(fullScreens==0&&rects.empty()&&draws.empty()&&circles.empty());
 weather.currentC=16;clearDraws();drawWeather(false);assert(fullScreens==0&&draws.size()==2&&draws[0].text=="16");
 weather.rainPct=20;clearDraws();drawWeather(false);assert(fullScreens==0&&draws.size()==1&&draws[0].text=="REGEN 20%");
 weather.error=true;clearDraws();drawWeather(false);assert(fullScreens==0&&footer=="LIVE / LETZTER STAND");
 // Departures: unchanged response stays, minutes alone redraw one field.
 cfg.stopName="S-Bahnhof Reick";cfg.stopPlace="Dresden";
 assert(departureText("Dresden Flughafen",true)=="Flughafen");
 assert(departureText(cfg.stopName.c_str(),false)=="S-Bahnhof Reick");
 cfg.stopPlace="";assert(departureText("Dresden Flughafen",true)=="Flughafen");
 cfg.stopPlace="Pirna";assert(departureText("Pirna Copitz",true)=="Copitz"&&departureText("Dresden Hauptbahnhof",true)=="Dresden Hbf");
 cfg.stopPlace="";cfg.stopName="Freital, Bahnhof";assert(departureText("Freital Deuben",true)=="Deuben");
 cfg.stopId="123";cfg.stopName="Haltestelle";departureN=2;
 strcpy(departures[0].line,"4");strcpy(departures[0].direction,"Laubegast");departures[0].minutes=3;
 strcpy(departures[1].line,"65");strcpy(departures[1].direction,"Heidenau");departures[1].minutes=8;
 clearDraws();drawDepartures();assert(fullScreens==1);clearDraws();drawDepartures(false);assert(fullScreens==0&&draws.empty()&&rects.empty());
 departures[0].minutes=2;clearDraws();drawDepartures(false);assert(fullScreens==0&&draws.size()==1&&draws[0].text=="2");
 departures[0].minutes=0;clearDraws();drawDepartures(false);assert(fullScreens==0);bool now=false;for(auto d:draws)if(d.text=="JETZT")now=true;assert(now);
 departureN=1;clearDraws();drawDepartures(false);assert(fullScreens==1);
 departureN=0;departureUnserved=true;departureError=false;clearDraws();drawDepartures(false);assert(fullScreens==1&&footer=="VVO");
 bool first=false,second=false;for(auto d:draws){if(d.text=="Haltestelle wird"){first=true;assert(d.color==C_GRAY);}if(d.text=="nicht bedient"){second=true;assert(d.color==C_GRAY);}assert(d.text!="nicht erreichbar");}assert(first&&second);
 clearDraws();drawDepartures(false);assert(fullScreens==0&&draws.empty());
 departureUnserved=false;departureError=true;clearDraws();drawDepartures(false);assert(fullScreens==1&&footer=="VVO FEHLER");

 // Timer: one seconds digit changes, same geometry -> one small cleared cell.
 cfg.countdown[0].title="МОСКВА";timerValue="00:01:09";clearDraws();drawCountdown(0);assert(fullScreens==1);
 clearDraws();drawCountdown(0,false);assert(fullScreens==0&&rects.empty()&&draws.empty());
 timerValue="00:01:08";clearDraws();drawCountdown(0,false);assert(fullScreens==0);int black=0;for(auto q:rects)if(q.color==C_BLACK){black++;assert(q.w<=42&&q.h<=80);}assert(black==1);
 timerValue="+00:00:01";clearDraws();drawCountdown(0,false);assert(fullScreens==1);
 // Clock: unchanged minute/date -> no draw; one changed digit clears one cell.
 timerValue="14:22";clearDraws();drawClock();assert(fullScreens==1);clearDraws();drawClock(false);assert(fullScreens==0&&rects.empty()&&draws.empty());
 timerValue="14:23";clearDraws();drawClock(false);assert(fullScreens==0&&rects.size()==1&&draws.size()==1&&draws[0].text=="3");
 puts("Real display: selective radar/new aircraft, stale states, tracker footer, weather motifs and Unicode bounds OK");
}
'''
pre+='\nstatic bool lastPlanePauseRendered=false,lastPlaneAgeRendered=true;\n'
names=['nextCodepoint','glyphCount','drawSpecialGlyph','textW','crop','stopTown','departureText','beginView','visibleText','visibleCenter','planePlaceTitle','drawPlaneHeader','routePart','drawFlightRoute','samePlane','planeDistance','planeAltitude','compactType','planeFooterLabel','drawPlaneAge','drawPlaneFooter','drawPlane','drawTrackedFlight','drawCloud','drawSun','drawMoon','drawSnowflake','drawWeatherIcon','drawSunEventIcon','drawWeather','drawDepartures','segMask','drawSevenDigit','sevenUnit','sevenCharWidth','drawSevenString','drawCountdown','drawClock']
with tempfile.TemporaryDirectory() as t:
 p=Path(t);(p/'test.cpp').write_text(pre+'\n'+'\n'.join(fn(name) for name in names)+'\n'+post)
 subprocess.run(['g++','-std=c++11','-Wall','-Wextra','-Werror','-I'+str(root/'src'),str(p/'test.cpp'),'-o',str(p/'test')],check=True)
 subprocess.run([str(p/'test')],check=True)
