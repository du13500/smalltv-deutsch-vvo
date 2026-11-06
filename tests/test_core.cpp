#include "../src/PixelFont.h"
#include "../src/FlightRoute.h"
#include "../src/AircraftPolicy.h"
#include "../src/RadarPolicy.h"
#include "../src/WeatherVisual.h"
#include "../src/TimeZones.h"
#include <assert.h>
#include <string>
#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
static std::vector<uint32_t> decode(const std::string& s){
  size_t i=0;uint32_t cp=0;std::vector<uint32_t> v;
  while(nextDisplayCodepoint(s.data(),s.size(),i,cp))v.push_back(cp);
  return v;
}
int main(){
 assert(!metadataTypeMatches("","A3","ST75"));
 assert(metadataTypeMatches("","A3","A21N"));
 assert(metadataTypeMatches("B39M","A3","B39M"));
 assert(!metadataTypeMatches("A21N","A3","ST75"));
 assert(!metadataTypeMatches("","","A21N"));
 assert(!metadataTypeMatches("","A1","A21N"));
  assert(decode("é")==decode("e\xCC\x81"));
  assert(decode("č")==decode("c\xCC\x8C"));
  assert(decode("ą")==decode("a\xCC\xA8"));
  for(auto s:{"♥","❤","❤️","💛","💚","💙","💜","🖤","🤍","🤎","🩷","🩵","🩶","💔","💞","❤️‍🔥","❤️‍🩹"})assert(decode(s)==std::vector<uint32_t>{0x2665});
  assert(decode("🤏")==std::vector<uint32_t>{0x1F90F});
  assert(decode("A❤️B").size()==3);
  assert(decode("?")[0]=='?');assert(specialGlyph('?')==nullptr);
  assert(decode("🚀")[0]==0x1F680);assert(specialGlyph(0x1F680)==nullptr);
  auto broken=decode(std::string("\xE2" "A"));assert(broken.size()==2&&broken[0]==0xFFFD&&broken[1]=='A');
  assert(decode("\xC0\xAF")[0]==0xFFFD);assert(decode("\xED\xA0\x80")[0]==0xFFFD);
  assert(decode("\xF4\x90\x80\x80")[0]==0xFFFD);
  for(uint32_t cp:decode("éèêëçČčŘřŠšŽžĚěŮůŇňŤťĎďĄąĆćĘęŁłŃńŚśŹźŻżÖöÜüÄäßØøÆæŒœ"))assert(specialGlyph(cp));
  for(uint32_t cp:decode("ȘșȚțİıĞğŐőŰűĲĳĿŀƏəΑθήναΘεσσαλονίκηΐΰМоскваКиївЇїҐґЄєЎўЂђЉљЊњЋћЏџҚқӘәӨөҮүҰұҢңҒғҺһ“”‘’…№€"))assert(specialGlyph(cp));
  for(uint32_t cp=0x400;cp<=0x45F;cp++)assert(specialGlyph(cp));
  for(uint32_t cp=0xC0;cp<=0x17F;cp++)if(cp!=0xD7&&cp!=0xF7)assert(specialGlyph(cp));
  assert(decode("ЁЙЇάΐ")==decode("Е\xCC\x88" "И\xCC\x86" "І\xCC\x88" "α\xCC\x81" "ι\xCC\x88\xCC\x81"));
  auto small=radarTlsBudget(512,4096),medium=radarTlsBudget(1024,4096),large=radarTlsBudget(4096,4096);
  assert(small.block==medium.block&&medium.block==large.block);
  assert(small.total<medium.total&&medium.total<large.total);
  assert(radarTlsFits(25248,11368,512,4096)); // Felix's blocked v0.4.6 screenshot
  assert(!radarTlsFits(small.total-1,20000,512,4096));
  assert(!radarTlsFits(30000,small.block-1,512,4096));
  assert(!radarExpired(30999,1000,true)&&radarExpired(31000,1000,true));
  assert(!radarExpired(50000,1000,false));
  assert(radarExpired(29990,0xFFFFFFF6U,true)); // millis wraparound
  for(int code:{0,1,2,3,45,48,51,53,55,56,57,61,63,65,66,67,71,73,75,77,80,81,82,85,86,95,96,99}){
    auto w=weatherVisual(code);assert(w.label&&w.intensity<=3);
    for(uint32_t cp:decode(w.label))assert(cp<128||specialGlyph(cp));
    assert(decode(w.label).size()*6<=224);
  }
  assert(weatherVisual(51).intensity==1&&weatherVisual(55).intensity==3);
  assert(weatherVisual(45).motif==WX_FOG&&weatherVisual(48).motif==WX_FOG);
  assert(weatherVisual(96).motif==WX_THUNDER_HAIL&&weatherVisual(99).intensity==3);
  assert(weatherVisual(50).motif==WX_CLOUD); // unsupported codes are never invented rain
  assert(timeZoneIndex("bad/zone")==-1&&timeZoneIndex(nullptr)==-1);
  auto localHour=[](const char* zone,time_t epoch){setenv("TZ",timeZoneRule(timeZoneIndex(zone)),1);tzset();struct tm t={};localtime_r(&epoch,&t);return t.tm_hour;};
  assert(localHour("Europe/Berlin",1768478400)==13); // 2026-01-15 12:00 UTC
  assert(localHour("Europe/Berlin",1784116800)==14); // 2026-07-15 12:00 UTC
  assert(localHour("America/New_York",1768478400)==7);
  assert(localHour("America/New_York",1784116800)==8);
  assert(localHour("Asia/Tokyo",1768478400)==21);
  assert(localHour("Australia/Sydney",1768478400)==23);
  assert(localHour("Australia/Sydney",1784116800)==22);
  FlightRoute r={};assert(parseFlightRoute("LEJ-BLR-HKG",r));assert(r.count==3&&!strcmp(r.airports[1],"BLR"));
  assert(parseFlightRoute(" lej - blr - hkg ",r));assert(r.count==3&&!strcmp(r.airports[0],"LEJ"));
  assert(parseFlightRoute("OSL-IST",r));assert(r.count==2);
  assert(parseFlightRoute("AAA-BBB-CCC-DDD-EEE-FFF-GGG-HHH-III",r));assert(r.count==8&&r.truncated);
  for(auto s:{"", "unknown", "LEJ", "LEJ-", "LEJ- ", "LEJ--HKG", "LEJ-N/A-HKG", "LEJ-BL1-HKG", "LEJJ-HKG", "-HKG"}){assert(!parseFlightRoute(s,r));assert(r.count==0);}
  puts("Latin/Greek/Cyrillic, UTF-8/hearts, routes, radar memory/expiry, weather and time zones: OK");
}
