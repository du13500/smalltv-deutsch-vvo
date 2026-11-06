#!/usr/bin/env python3
"""Chunked HTTP/1.1 bodies for adsb.lol are decoded without extra buffering."""
from pathlib import Path
import subprocess,tempfile
root=Path(__file__).resolve().parents[1]
test=r'''
#include "ChunkDecoder.h"
#include <cassert>
#include <cstdio>
#include <string>
struct Src {std::string d;size_t i=0;int nextByte(){return i<d.size()?(unsigned char)d[i++]:-1;}};
static std::string decode(const std::string& raw,bool chunked,bool* bad=nullptr){
 Src s;s.d=raw;ChunkDecoder<Src> dec(s,chunked);std::string out;int c;
 while((c=dec.read())>=0){out+=(char)c;}
 if(bad)*bad=dec.failed();
 return out;}
int main(){
 const std::string json="{\"ac\":[{\"hex\":\"4400b6\"}],\"msg\":\"No error\"}";
 assert(decode(json,false)==json);
 bool bad=true;
 assert(decode("5\r\n{\"ac\"\r\n6\r\n:[1,2]\r\n1\r\n}\r\n0\r\n\r\n",true,&bad)=="{\"ac\":[1,2]}"&&!bad);
 assert(decode("A;ext=1\r\n0123456789\r\n0\r\nX-Trailer: 1\r\n\r\n",true,&bad)=="0123456789"&&!bad);
 assert(decode("a\r\nabcdefghij\r\n0\r\n\r\n",true)=="abcdefghij");
 assert(decode("3\nabc\n0\n\n",true)=="abc");
 assert(decode("4\r\nabc",true)=="abc");
 assert(decode("zz\r\nabc\r\n",true,&bad).empty()&&bad);
 assert(decode("FFFFFFFF\r\n",true,&bad).empty()&&bad);
 assert(decode("\r\nabc",true,&bad).empty()&&bad);
 assert(decode("0\r\n\r\n",true,&bad).empty()&&!bad);
 puts("Chunked bodies, extensions, trailers, truncation and malformed sizes: OK");
}
'''
with tempfile.TemporaryDirectory() as tmp:
 p=Path(tmp);f=p/'test.cpp';f.write_text(test)
 subprocess.run(['g++','-std=c++11','-Wall','-Wextra','-Werror','-I',str(root/'src'),str(f),'-o',str(p/'test')],check=True)
 subprocess.run([str(p/'test')],check=True)
