#include <stdlib.h>
#include <assert.h>
#include "compat_scan.h"
static uint16_t u16(const unsigned char *p){uint16_t a;memcpy(&a,p,2);return a;}
static uint32_t u32(const unsigned char *p){uint32_t a;memcpy(&a,p,4);return a;}
int main(int argc,char **argv){assert(argc==2);FILE *f=fopen(argv[1],"rb");assert(f);fseek(f,0,SEEK_END);long sz=ftell(f);rewind(f);unsigned char *b=malloc((size_t)sz);assert(b&&fread(b,1,(size_t)sz,f)==(size_t)sz);fclose(f);uint32_t pe=u32(b+60),image=u32(b+pe+24+56);const unsigned char *sections=b+pe+24+u16(b+pe+20);unsigned n=u16(b+pe+6);const unsigned char *text=NULL;uint32_t tr=0,ts=0;
 for(unsigned i=0;i<n;i++){const unsigned char *s=sections+40*i;if(!memcmp(s,".text\0\0",8)){tr=u32(s+12);ts=u32(s+16);assert((uint64_t)u32(s+20)+ts<=(uint64_t)sz);text=b+u32(s+20);}}
 printf("text_rva=%x size=%x\n",tr,ts);assert(text);NativeBindings r;char why[200]={0};int ok=resolve_native(text,ts,tr,image,&r,why,sizeof(why));printf("%s reader=%d render=%d profile=%d singleton=%x nvg=%x renderer=%x %s\n",argv[1],ok,r.render_ok,r.profile,r.singleton,r.nvg,r.renderer,why);assert(ok&&r.render_ok&&r.rva[B_TEXT]&&r.rva[B_FONT]);
 assert(r.voxel_request);if(!r.profile)assert(r.voxel_request==0x1389190);assert(r.singleton==(r.profile?0x6f5f868:0x6f60828));assert(r.rva[B_MAP]==(r.profile?0x135de00:0x135def0));assert(r.rva[B_END]==(r.profile?0x199bcc0:0x199bf00));
 NativeBindings relocated;assert(resolve_native(text,ts,tr+0x2000,image+0x2000,&relocated,why,sizeof(why)));assert(relocated.singleton==r.singleton+0x2000);assert(relocated.rva[B_MAP]==r.rva[B_MAP]+0x2000);
 unsigned char *copy=malloc(ts);assert(copy);memcpy(copy,text,ts);size_t ui=r.rva[B_UI]-tr;copy[ui+0x7b]^=1;assert(!resolve_native(copy,ts,tr,image,&r,why,sizeof(why))); /* manager layout immediate */
 memcpy(copy,text,ts);size_t o=0;int v=0;assert(scan_named(copy,ts,&native_patterns[B_MAP],&o,&v));copy[o]^=1;assert(resolve_native(copy,ts,tr,image,&r,why,sizeof(why))&&!r.render_ok); /* reader survives render refusal */
 memcpy(copy,text,ts);size_t request_at=0;int request_variant=0;assert(scan_named(copy,ts,&position54_pattern,&request_at,&request_variant));copy[request_at]^=1;assert(resolve_native(copy,ts,tr,image,&r,why,sizeof(why))&&r.render_ok&&!r.voxel_request); /* independent position adapter refuses invalid code */
 unsigned char tiny[1000]={0};const BytePattern *p=&native_patterns[B_ADDRESS].variants[0];memcpy(tiny,p->bytes,p->size);memcpy(tiny+400,p->bytes,p->size);assert(!scan_named(tiny,sizeof(tiny),&native_patterns[B_ADDRESS],&o,&v)); /* ambiguity */
 free(copy);free(b);puts("PASS real EXE scan, expected bindings, changed layout refusal, renderer failure isolation, duplicate refusal");return 0;}
