#pragma once
#include <string.h>
// Missing live type is not evidence that any database type belongs to this plane.
static inline bool metadataTypeMatches(const char* live,const char* category,const char* candidate){
  if(!candidate||!*candidate)return false;
  if(live&&*live)return !strcmp(live,candidate);
  if(!category)return false;
  if(!strcmp(category,"A3")){
    const char* medium[]={"A318","A319","A320","A321","A20N","A21N","B737","B738","B739","B38M","B39M","E190","E195","E290","E295","BCS1","BCS3","CRJ9","AT76","DH8D"};
    for(const char* code:medium)if(!strcmp(candidate,code))return true;
  }
  return false;
}
