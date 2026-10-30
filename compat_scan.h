#ifndef IGM_COMPAT_SCAN
#define IGM_COMPAT_SCAN
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>
typedef struct {const unsigned char *bytes,*mask;size_t size;} BytePattern;
typedef struct {const char *name;BytePattern variants[2];} NamedPattern;
#include "compat_patterns.h"
#include "position54_patterns.h"
enum {B_ADDRESS,B_UI,B_META,B_MAP,B_END,B_APPEND,B_FILL,B_PROJECT,B_VIEWPORT,B_VOXEL,B_WORLD,B_MARKER,B_LOOKUP,B_TEXT,B_FONT};
typedef struct {uint32_t rva[15];uint32_t singleton,nvg,renderer,voxel_request;uint32_t alliance_offset,vr_offset,viewport_offset;int profile,render_ok;} NativeBindings;
static int pattern_at(const unsigned char *p,size_t n,const BytePattern *s){if(s->size>n)return 0;for(size_t i=0;i<s->size;i++)if(s->mask[i]&&p[i]!=s->bytes[i])return 0;return 1;}
/* Longest fixed run prefilter; ambiguous matches at different addresses are refused. */
static int scan_named(const unsigned char *text,size_t n,const NamedPattern *s,size_t *hit,int *variant){
 int found=0;size_t prior=0;
 for(int v=0;v<2;v++){
  const BytePattern *p=&s->variants[v];size_t best=0,len=0,run=0,start=0;
  if(p->size>n)continue;
  for(size_t i=0;i<p->size;i++){if(p->mask[i]){if(!run)start=i;run++;if(run>len){best=start;len=run;}}else run=0;}
  if(len<4)return 0;
  size_t pos=best,limit=n-p->size+best;
  while(pos<=limit){const unsigned char *q=memchr(text+pos,p->bytes[best],limit-pos+1);if(!q)break;pos=(size_t)(q-text);
   if(!memcmp(q,p->bytes+best,len)&&pattern_at(text+pos-best,n-(pos-best),p)){
    size_t at=pos-best;if(found&&at!=prior)return 0;found=1;prior=at;*hit=at;*variant=v;
   }pos++;
  }
 }
 return found;
}
static uint32_t relative_target(const unsigned char *text,size_t n,uint32_t text_rva,size_t at,size_t disp,size_t end,uint32_t image_size){
 if(at>n||disp+4>n-at||end>n-at)return 0;
 int32_t d;memcpy(&d,text+at+disp,4);int64_t t=(int64_t)text_rva+(int64_t)at+(int64_t)end+d;
 return t>0&&t<image_size?(uint32_t)t:0;
}
static int resolve_native(const unsigned char *text,size_t n,uint32_t text_rva,uint32_t image_size,NativeBindings *out,char *why,size_t cap){
 memset(out,0,sizeof(*out));out->render_ok=1;size_t at[12]={0};int variants[12]={0};
 for(int i=0;i<12;i++){
  if(!scan_named(text,n,&native_patterns[i],&at[i],&variants[i])){
   snprintf(why,cap,"missing or ambiguous structural pattern %s",native_patterns[i].name);
   if(i<=B_META)return 0;
   out->render_ok=0;continue;
  }
  if(at[i]>UINT32_MAX-text_rva)return 0;
  out->rva[i]=text_rva+(uint32_t)at[i];
 }
 out->profile=variants[B_UI];
 out->alliance_offset=out->profile?0x95e270:0x95e280;
 out->vr_offset=out->profile?0x92cbd4:0x92cbe4;
 out->viewport_offset=out->profile?0x930c00:0x930c10;
 out->singleton=relative_target(text,n,text_rva,at[B_ADDRESS],7,11,image_size);
 if(!out->singleton||out->singleton!=relative_target(text,n,text_rva,at[B_UI]+0x6d,3,7,image_size)||
    out->rva[B_ADDRESS]!=relative_target(text,n,text_rva,at[B_UI]+5,1,5,image_size)){
  snprintf(why,cap,"reader cross-references differ");return 0;
 }
 out->rva[B_LOOKUP]=relative_target(text,n,text_rva,at[B_UI]+0x82,1,5,image_size);
 if(out->rva[B_LOOKUP]<text_rva||out->rva[B_LOOKUP]-text_rva>=n){snprintf(why,cap,"lookup target outside code");return 0;}
 size_t lookup=out->rva[B_LOOKUP]-text_rva;
 if(!pattern_at(text+lookup,n-lookup,&native_patterns[B_LOOKUP].variants[out->profile])||
    relative_target(text,n,text_rva,lookup+0x49,1,5,image_size)!=out->rva[B_META]){
  snprintf(why,cap,"alliance lookup layout or metadata link differs");return 0;
 }
 if(!out->render_ok)return 1;
 if(variants[B_MAP]!=out->profile||variants[B_MARKER]!=out->profile){snprintf(why,cap,"mixed layout profiles");out->render_ok=0;return 1;}
 out->nvg=relative_target(text,n,text_rva,at[B_MAP]+0x121,3,7,image_size);
 out->renderer=relative_target(text,n,text_rva,at[B_MARKER]+0xb0,3,7,image_size);
 if(!out->nvg||!out->renderer||out->singleton!=relative_target(text,n,text_rva,at[B_MAP]+0x25,3,7,image_size)||
 out->singleton!=relative_target(text,n,text_rva,at[B_MARKER]+0x13,3,7,image_size)||
 out->rva[B_VIEWPORT]!=relative_target(text,n,text_rva,at[B_MAP]+0xca,1,5,image_size)||
 out->rva[B_VIEWPORT]!=relative_target(text,n,text_rva,at[B_MARKER]+0xc5,1,5,image_size)||
 out->rva[B_PROJECT]!=relative_target(text,n,text_rva,at[B_MARKER]+0x13a,1,5,image_size)){
  snprintf(why,cap,"render cross-references differ");out->render_ok=0;return 1;
 }
 size_t text_at=0,font_at=0;int text_v=0,font_v=0;
 if(scan_named(text,n,&native_patterns[B_TEXT],&text_at,&text_v)&&scan_named(text,n,&native_patterns[B_FONT],&font_at,&font_v)&&
 relative_target(text,n,text_rva,text_at+0x20f,1,5,image_size)==text_rva+font_at){out->rva[B_TEXT]=text_rva+(uint32_t)text_at;out->rva[B_FONT]=text_rva+(uint32_t)font_at;}
 size_t request_at=0;int request_variant=0;if(scan_named(text,n,&position54_pattern,&request_at,&request_variant))out->voxel_request=text_rva+(uint32_t)request_at;
 snprintf(why,cap,"all structural patterns and dependency links verified");return 1;
}
#endif
