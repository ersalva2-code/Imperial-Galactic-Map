#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "core.h"
#include "map_catalogue.h"
#include "catalogue_ui.h"
typedef IgmRecord Seen;
static Seen known[4096];static size_t known_count;
static int known_lock;
static void AcquireSRWLockExclusive(int*p){(void)p;}
static void ReleaseSRWLockExclusive(int*p){(void)p;}
#include "known_state53.inc"
int main(void){
 assert(IMPERIAL53_COUNT==92);size_t galaxies[256]={0},named=0;
 for(size_t i=0;i<IMPERIAL53_COUNT;i++){const Imperial53System*s=&imperial53_systems[i];if(i)assert(s->ua>imperial53_systems[i-1].ua);assert(imperial53_find(s->ua)==s);assert(strlen(s->name)<128);named+=s->name[0]!=0;galaxies[(s->ua>>32)&255]++;assert(changed(s->ua,imperial53_id));}
 assert(named==57&&galaxies[0]==77&&galaxies[9]==13&&galaxies[20]==2&&known_count==92);
 uint32_t other[4]={1,2,3,4};uint64_t first=imperial53_systems[0].ua;assert(!changed(first,other));assert(!needs_record(first,other));assert(own_id(known[0].id));assert(!imperial53_find(UINT64_C(0x990012345678)));assert(changed(UINT64_C(0x990012345678),other));assert(!memcmp(known[92].id,other,16));
 static Ui42Data data;Ui42State state={0};Ui42Frame frame;data.ns=1;data.na=1;data.systems[0].ua=first;memcpy(data.systems[0].id,imperial53_id,16);memcpy(data.alliances[0].id,imperial53_id,16);data.alliances[0].own=1;strcpy(data.alliances[0].name,"Imperio Español (IESP)");state.system=first;ui42_make(&data,&state,&frame,0);int fav=0;for(size_t i=0;i<frame.count;i++){assert(frame.items[i].action!=U42_DELETE);fav+=frame.items[i].action==U42_FAVOURITE;}assert(fav==1);
 data.systems[0].ua=UINT64_C(0x990012345678);state.system=data.systems[0].ua;ui42_make(&data,&state,&frame,0);int del=0;for(size_t i=0;i<frame.count;i++)del+=frame.items[i].action==U42_DELETE;assert(del==1);
 puts("PASS production compiled imperial records + catalogue mutation policy + native UI: 92 systems, galaxies/names, override rejected, user catalogue mutable, protected delete absent, favourite retained");
}
