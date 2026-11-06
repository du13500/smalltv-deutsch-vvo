#!/usr/bin/env python3
"""Test GeoResponse against the actual ESP8266 3.1.2 transfer routine."""
from pathlib import Path
import os, subprocess, tempfile
root=Path(__file__).resolve().parents[1]
framework=Path(os.environ.get('SMALLTV_FRAMEWORK',str(Path.home()/'.platformio/packages/framework-arduinoespressif8266')))
core=(framework/'cores/esp8266/StreamSend.cpp').read_text()
transfer=core[core.index('size_t\nStream::SendGenericPeekBuffer'):core.index('\nsize_t\nStream::SendGenericRegularUntil')]
source=(root/'src/Geo.cpp').read_text()
buffer=source[source.index('class GeoResponse'):source.index('\nstatic BearSSL::Session')]
stub=r'''
#define FW_USER_AGENT "PixelStop/test"
#include <cassert>
#include <string>
#include <cstring>
#include <cstdint>
#include <algorithm>
#include <memory>
#include <sys/types.h>
static constexpr uint32_t GEO_RECEIVE_RESERVE=3072;
enum {SeekSet};static uint32_t ticks=0;
namespace esp8266 {namespace polledTimeout {
struct oneShotFastMs {using timeType=uint32_t;static constexpr timeType neverExpires=0xffffffff;
 uint32_t start,duration;explicit oneShotFastMs(uint32_t n):start(ticks),duration(n){}
 void reset(){start=ticks;}explicit operator bool()const{return ticks-start>=duration;}};
}}
void optimistic_yield(int){++ticks;}
struct Print {virtual ~Print(){};virtual int availableForWrite(){return 0;}
 virtual bool outputCanTimeout(){return true;}virtual size_t write(uint8_t)=0;
 virtual size_t write(const uint8_t*,size_t)=0;
 size_t write(const char* p,size_t n){return write((const uint8_t*)p,n);}};
struct Stream:Print {enum class Report{Success,TimedOut,ShortOperation,WriteError};Report report=Report::Success;
 virtual int available()=0;virtual int read()=0;virtual int peek()=0;virtual void flush()=0;
 virtual size_t peekAvailable(){return 0;}virtual const char* peekBuffer(){return nullptr;}
 virtual void peekConsume(size_t){}virtual bool inputCanTimeout(){return true;}
 uint32_t getTimeout(){return 6;}void setReport(Report r){report=r;}Report getLastSendReport(){return report;}
 size_t SendGenericPeekBuffer(Print*,ssize_t,int,uint32_t);
};
static size_t fakeHeap=20000;struct FakeESP{size_t getFreeHeap(){return fakeHeap;}} ESP;
enum DiagStage {DS_HEAP,DS_RESPONSE_LIMIT,DS_FILE_ERROR,DS_TRANSFER_ERROR};
struct DiagRequest{int last=-1;void stage(DiagStage s){last=s;}void http(int){}};
static size_t writeLimit=100000,largestWrite=0;static int warmWrites=0;
static bool openAllowed=true,closed=false,removed=false,truncateAllowed=true;
struct File {
 std::shared_ptr<std::string> bytes;size_t pos=0;bool warmed=false;
 explicit operator bool()const{return bool(bytes);}
 size_t write(const uint8_t* p,size_t n){if(!warmed){warmed=true;++warmWrites;fakeHeap-=512;}
  largestWrite=std::max(largestWrite,n);size_t take=std::min(n,writeLimit);bytes->append((const char*)p,take);return take;}
 bool truncate(size_t n){if(!truncateAllowed)return false;bytes->resize(n);return true;}
 int available(){return bytes->size()-pos;}int read(){return available()?(*bytes)[pos++]:-1;}
 int peek(){return available()?(*bytes)[pos]:-1;}void flush(){}
 bool seek(size_t n,int){pos=n;return n<=bytes->size();}void close(){closed=true;bytes.reset();}
};
struct FS {File open(const char* path,const char* mode){assert(std::string(path)=="/geo-response.tmp");assert(std::string(mode)=="w+");File f;if(openAllowed)f.bytes=std::make_shared<std::string>();return f;}
 void remove(const char* path){assert(std::string(path)=="/geo-response.tmp");removed=true;}} LittleFS;
namespace BearSSL {struct WiFiClientSecure{bool stopped=false;void stop(){stopped=true;}};}
struct Feed:Stream {
 std::string body;size_t pos=0;bool starve=false,dropHeap=false;
 size_t peekAvailable()override{if(dropHeap&&pos>=256)fakeHeap=2400;return starve?0:std::min(size_t(1024),body.size()-pos);}
 const char* peekBuffer()override{return body.data()+pos;}void peekConsume(size_t n)override{pos+=n;}
 size_t write(uint8_t)override{return 0;}size_t write(const uint8_t*,size_t)override{return 0;}
 int available()override{return peekAvailable();}int read()override{return -1;}int peek()override{return -1;}void flush()override{}
};
struct HTTPClient {Feed feed;bool ended=false;int declared=-2;
 int getSize(){return declared==-2?(int)feed.body.size():declared;}
 int writeToStream(Stream* s){auto n=feed.SendGenericPeekBuffer(s,getSize(),-1,6);return feed.report==Stream::Report::Success?(int)n:-11;}
 void end(){ended=true;}};
void reset(){fakeHeap=20000;ticks=0;writeLimit=100000;largestWrite=0;warmWrites=0;openAllowed=truncateAllowed=true;closed=removed=false;}
'''
test=r'''
int main(){
 reset();{GeoResponse response;HTTPClient http;BearSSL::WiFiClientSecure client;DiagRequest trace;
  assert(warmWrites==1);http.feed.body=std::string(6169,'x');assert(response.receive(http,client,trace));
  assert(http.ended&&client.stopped&&largestWrite<=256&&http.feed.pos==6169);
  std::string body;while(response.available())body+=char(response.read());assert(body==http.feed.body);}
 assert(closed&&removed);
 // Original bug: Print's zero capacity prevents every write, until timeout.
 reset();{Feed feed;feed.body="body";Feed sink;
  assert(feed.SendGenericPeekBuffer(&sink,4,-1,6)==0);assert(feed.report==Stream::Report::TimedOut&&ticks==6);}
 reset();{GeoResponse response;HTTPClient http;BearSSL::WiFiClientSecure client;DiagRequest trace;
  http.feed.body=std::string(GeoResponse::limit,'x');assert(response.receive(http,client,trace));}
 reset();{GeoResponse response;HTTPClient http;BearSSL::WiFiClientSecure client;DiagRequest trace;
  http.feed.body=std::string(GeoResponse::limit+1,'x');assert(!response.receive(http,client,trace));
  assert(trace.last==DS_RESPONSE_LIMIT&&ticks==0&&http.feed.pos==0);}
 reset();{GeoResponse response;HTTPClient http;BearSSL::WiFiClientSecure client;DiagRequest trace;
  http.declared=-1;http.feed.body=std::string(GeoResponse::limit+1,'x');assert(!response.receive(http,client,trace));assert(trace.last==DS_RESPONSE_LIMIT);}
 reset();{GeoResponse response;HTTPClient http;BearSSL::WiFiClientSecure client;DiagRequest trace;
  http.feed.body="partial";writeLimit=2;assert(!response.receive(http,client,trace));assert(trace.last==DS_FILE_ERROR&&ticks<6);}
 reset();{GeoResponse response;HTTPClient http;BearSSL::WiFiClientSecure client;DiagRequest trace;
  http.feed.body="missing";http.feed.starve=true;assert(!response.receive(http,client,trace));assert(trace.last==DS_TRANSFER_ERROR);}
 reset();{GeoResponse response;HTTPClient http;BearSSL::WiFiClientSecure client;DiagRequest trace;
  http.feed.body="safe refusal";fakeHeap=2400;assert(!response.receive(http,client,trace));assert(trace.last==DS_HEAP&&ticks==0&&http.feed.pos==0);}
 reset();{GeoResponse response;HTTPClient http;BearSSL::WiFiClientSecure client;DiagRequest trace;
  http.feed.body=std::string(1024,'x');http.feed.dropHeap=true;assert(!response.receive(http,client,trace));assert(trace.last==DS_HEAP&&http.feed.pos==256&&ticks<6);}
 reset();openAllowed=false;{GeoResponse response;assert(!response.ready());}
 reset();truncateAllowed=false;{GeoResponse response;assert(!response.ready());}
}
'''
with tempfile.TemporaryDirectory() as tmp:
 p=Path(tmp);(p/'test.cpp').write_text(stub+transfer+buffer+test)
 subprocess.run(['g++','-std=c++11','-Wall','-Wextra','-Werror',str(p/'test.cpp'),'-o',str(p/'test')],check=True)
 subprocess.run([str(p/'test')],check=True)
assert source.count('response.receive(http,client,trace)')==1
assert source.count('deserializeJson(doc, response,')==2
helper=source.split('static int receiveGeo(',1)[1].split('\nstatic String urlEncode',1)[0]
assert 'JsonDocument' not in helper
assert helper.index('http.begin(client,url)')<helper.index('url=String();')<helper.index('geoHeapReady(op,true)')<helper.index('http.GET()')
assert helper.index('http.collectHeaders')<helper.index('geoHeapReady(op,true)')
for name,op in [('geoSearchPlaces','D_GEO_SEARCH'),('geoReversePlace','D_GEO_REVERSE')]:
 part=source.split('bool '+name+'(',1)[1]
 assert 'BearSSL::WiFiClientSecure client' not in part
 assert 'HTTPClient http' not in part
 assert part.index('GeoResponse response;')<part.index('receiveGeo(url,response,trace,'+op+')')<part.index('DS_GEO_RELEASE')<part.index('deserializeJson')
 assert 'response.close()' in part[part.index('deserializeJson'):part.index('if (err) return false;')]
print('Real ESP8266 transfer: old timeout reproduced; bounded writes, heap refusal, failures and cleanup OK')

# Execute the actual production receive helper. stop() deliberately does NOT
# release the TLS stack, matching the framework: only object destruction does.
lifetime_stub=stub.replace('DS_TRANSFER_ERROR};','DS_TRANSFER_ERROR,DS_GEO_CONNECT,DS_GEO_RECEIVE};')
lifetime_stub=lifetime_stub.replace('struct WiFiClientSecure{bool stopped=false;',
 'struct WiFiClientSecure{bool stopped=false;WiFiClientSecure(){fakeHeap-=6200;}~WiFiClientSecure(){fakeHeap+=6200;}')
lifetime_stub=lifetime_stub.replace('struct HTTPClient {',r'''
struct String {std::string value;String(const char* s=""):value(s){}const char* c_str()const{return value.c_str();}};
static int replyCode=200;static bool preparedAllowed=true,readyAllowed=true;
static constexpr int HTTP_CODE_OK=200,SERVICE_GEO=0;
enum DiagOp {D_GEO_SEARCH,D_GEO_REVERSE};
bool geoHeapReady(DiagOp,bool){return preparedAllowed;}bool geoReady(){return readyAllowed;}
void serviceResponse(int,int,const char*){}void makeSecure(BearSSL::WiFiClientSecure&){}
struct HTTPClient {''')
lifetime_stub=lifetime_stub.replace('int getSize(){',r'''
 void useHTTP10(bool){}void setTimeout(int){}void setReuse(bool){}
 bool begin(BearSSL::WiFiClientSecure&,const String&){feed.body=R"({"name":"Dresden"})";return true;}
 void addHeader(const char*,const char*){}void setUserAgent(const char*){}
 void collectHeaders(const char**,int){}int GET(){return replyCode;}String header(const char*){return String();}
 int getSize(){''')
helper_source='static int receiveGeo('+helper
lifetime_test=r'''
int main(){
 for(int code:{200,404,500}){reset();replyCode=code;preparedAllowed=readyAllowed=true;
  GeoResponse response;auto before=fakeHeap;String url("https://example.test");DiagRequest trace;
  assert(receiveGeo(url,response,trace,D_GEO_SEARCH)==code);
  assert(fakeHeap==before); // Includes destructor-only release of all 6,200 bytes.
 }
 reset();{GeoResponse response;auto before=fakeHeap;String url("url");DiagRequest trace;
  preparedAllowed=false;assert(receiveGeo(url,response,trace,D_GEO_REVERSE)==0);assert(fakeHeap==before);}
 reset();{GeoResponse response;auto before=fakeHeap;String url("url");DiagRequest trace;
  preparedAllowed=true;readyAllowed=false;assert(receiveGeo(url,response,trace,D_GEO_SEARCH)==0);assert(fakeHeap==before);}
 reset();{GeoResponse response;auto before=fakeHeap;String url("url");DiagRequest trace;
  readyAllowed=true;replyCode=200;writeLimit=0;assert(receiveGeo(url,response,trace,D_GEO_SEARCH)==0);assert(fakeHeap==before);}
}
'''
with tempfile.TemporaryDirectory() as tmp:
 p=Path(tmp);(p/'test.cpp').write_text(lifetime_stub+transfer+buffer+helper_source+lifetime_test)
 subprocess.run(['g++','-std=c++11','-Wall','-Wextra','-Werror',str(p/'test.cpp'),'-o',str(p/'test')],check=True)
 subprocess.run([str(p/'test')],check=True)
print('Production Geo helper: TLS stack released on success, 404, failure and guard refusal before JSON')

import runpy
runpy.run_path(str(root/"tools/test_network_guard.py"),run_name="__main__")
