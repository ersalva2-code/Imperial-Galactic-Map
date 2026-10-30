#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "compat_scan.h"
#include "player58_pattern.h"
int main(int argc,char**argv){FILE*f=fopen(argc>1?argv[1]:(getenv("IGM_REFERENCE_EXE")?getenv("IGM_REFERENCE_EXE"):"upload/NMS.exe"),"rb");assert(f);fseek(f,0,SEEK_END);long n=ftell(f);rewind(f);unsigned char*b=malloc((size_t)n);assert(b&&fread(b,1,(size_t)n,f)==(size_t)n);fclose(f);uint32_t pe;memcpy(&pe,b+60,4);unsigned short count,opt;memcpy(&count,b+pe+6,2);memcpy(&opt,b+pe+20,2);int ok=0;for(unsigned i=0;i<count;i++){unsigned char*s=b+pe+24+opt+40*i;if(memcmp(s,".text\0",6))continue;uint32_t rva,size,off;memcpy(&rva,s+12,4);memcpy(&size,s+16,4);memcpy(&off,s+20,4);assert((uint64_t)off+size<=(uint64_t)n);NamedPattern p={"identity58",{{player58_bytes,player58_mask,sizeof(player58_bytes)},{player58_bytes,player58_mask,sizeof(player58_bytes)}}};size_t hit=0;int v=0;assert(scan_named(b+off,size,&p,&hit,&v));assert(rva+hit==0x359910);ok=1;}assert(ok);free(b);puts("PASS guard58: unique live MyAlliances loader signature in exact supplied Steam EXE at RVA359910; input offsets10/50 copy to manager144/154 (static verification).");}
