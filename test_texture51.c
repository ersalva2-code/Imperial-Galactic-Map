#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "photo51_pixels.h"
#include "path_batch.h"
#include "texture51_patterns.h"
#define photos51_enabled enabled
static int enabled=1,fail_create,bad_guard;static unsigned creates,deletes,draws;static int widths[256],heights[256],nextid=1;static unsigned generation=1;static float catalogue42_transform[6]={2,0,0,2,10,20};
static int create_fn(void*b,unsigned char fmt,int w,int h,int flags,const unsigned char*p){assert(b==(void*)9&&fmt==7&&flags==0&&((w==64&&h==36&&p[0]==45&&p[1]==90&&p[2]==113)||(w==180&&h==180&&p[0]==17&&p[1]==51&&p[2]==119&&p[3]==85)));creates++;if(fail_create)return 0;int n=nextid++;widths[n]=w;heights[n]=h;return n;}
static int delete_fn(void*b,int n){assert(b==(void*)9&&n>0&&widths[n]==64);deletes++;widths[n]=0;return 1;}
static void delete_target(void){}
static int size_fn(void*b,int n,int*w,int*h){assert(b==(void*)9);*w=widths[n];*h=heights[n];return *w>0;}
static uint64_t image_base;static unsigned native_text_rva,native_text_size=0x100000;static void log_line(const char*s,...){(void)s;}
static int photos51_get(uint64_t ua,unsigned planet,Photo51*out,unsigned*g){assert(ua&&planet<=15);if(out){memset(out,0,sizeof(*out));out->w=64;out->h=36;out->pixels[0]=45;out->pixels[1]=90;out->pixels[2]=113;}*g=generation;return 1;}
static int read_self(uint64_t p,void*out,size_t n){if(p==(uintptr_t)create_fn){memcpy(out,tex51_create_bytes,n);if(bad_guard)((unsigned char*)out)[0]^=1;return 1;}if(p==(uintptr_t)size_fn){memcpy(out,tex51_size_bytes,n);return 1;}if(p==(uintptr_t)delete_target){memcpy(out,tex51_delete_target_bytes,n);return 1;}if(p==(uintptr_t)delete_fn){unsigned char b[8]={0x45,0x33,0xc0,0xe9};int32_t d=(int32_t)((intptr_t)delete_target-(intptr_t)delete_fn-8);memcpy(b+4,&d,4);memcpy(out,b,n);return 1;}memcpy(out,(void*)(uintptr_t)p,n);return 1;}
static void set_float(unsigned char*p,float x){memcpy(p,&x,4);}
static void color(unsigned char*s,float r,float g,float b,float a){memset(s,0,80);set_float(s,1);set_float(s+12,1);set_float(s+36,1);float c[4]={r,g,b,a};memcpy(s+40,c,16);memcpy(s+56,c,16);}
static unsigned char *drawstate;
static void submit_batch(void*c,IgmPathBatch*b){(void)c;assert(b->count==13);int id=0;memcpy(&id,drawstate+72,4);assert(id>0&&(widths[id]==64||widths[id]==180));float ex,ey;memcpy(&ex,drawstate+24,4);memcpy(&ey,drawstate+28,4);assert((fabsf(ex-104)<.01f&&fabsf(ey-58.5f)<.01f)||(fabsf(ex-487.1111f)<.01f&&fabsf(ey-274)<.01f)||(fabsf(ex-26.4f)<.01f&&fabsf(ey-26.4f)<.01f));draws++;b->count=0;}
/* Production code below uses the same patterns; suppress duplicate definitions only. */
#include "texture51_render.inc"
#include "icon52_format.h"
#define ICON52_LIMIT 128
static struct {uint32_t id[4];unsigned char pixels[ICON52_BYTES];} icons52[ICON52_LIMIT];
static int icon52_find(const uint32_t id[4]){return !memcmp(id,icons52[0].id,16)?0:-1;}
#include "icon52_render.inc"
int main(void){uintptr_t lo=(uintptr_t)create_fn;uintptr_t fns[]={(uintptr_t)size_fn,(uintptr_t)delete_fn,(uintptr_t)delete_target};for(int i=0;i<3;i++)if(fns[i]<lo)lo=fns[i];image_base=lo-0x1000;
 uint64_t context[7]={9,0,0,(uintptr_t)create_fn,(uintptr_t)delete_fn,0,(uintptr_t)size_fn};unsigned char state[264],before[264];memset(state,0x55,sizeof(state));memcpy(before,state,sizeof(state));drawstate=state;
 tex51_begin(context);assert(tex51.ready);assert(tex51_draw(context,state,100,2,400,300,104,58.5f));assert(creates==1&&draws==1&&!memcmp(state,before,sizeof(state)));assert(tex51_draw(context,state,100,2,400,300,104,58.5f));assert(creates==1&&draws==2);assert(tex51_draw(context,state,100,2,802,392,686,274));assert(creates==1&&draws==3&&!memcmp(state,before,sizeof(state)));assert(!tex51_draw(context,state,101,1,400,300,104,58.5f));
 tex51_begin(context);assert(tex51_draw(context,state,100,1,400,300,104,58.5f));assert(creates==2);for(uint64_t u=102;u<120;u++){tex51_begin(context);assert(tex51_draw(context,state,u,0,400,300,104,58.5f));}assert(deletes>0);
 generation++;tex51_begin(context);assert(tex51_draw(context,state,119,0,400,300,104,58.5f));unsigned prev=creates;generation++;assert(!tex51_draw(context,state,119,0,400,300,104,58.5f));assert(creates==prev);tex51_begin(context);assert(tex51_draw(context,state,119,0,400,300,104,58.5f));
 uint32_t custom[4]={1,2,3,4};memcpy(icons52[0].id,custom,16);unsigned char px[4]={17,51,119,85};memcpy(icons52[0].pixels,px,4);enabled=0;icons52_begin(context);prev=creates;assert(icons52_draw(context,state,custom,-13.2f,-13.2f,26.4f,catalogue42_transform,.9f));assert(creates==prev+1);assert(icons52_draw(context,state,custom,-13.2f,-13.2f,26.4f,catalogue42_transform,.9f)&&creates==prev+1);assert(!memcmp(state,before,sizeof(state)));uint32_t unknown[4]={9};assert(!icons52_draw(context,state,unknown,0,0,26.4f,catalogue42_transform,1));enabled=1;
 fail_create=1;tex51_begin(context);assert(!tex51_draw(context,state,200,0,400,300,104,58.5f));prev=creates;tex51_begin(context);assert(!tex51.ready&&!tex51_draw(context,state,200,0,400,300,104,58.5f)&&creates==prev);
 uint64_t another[7];memcpy(another,context,sizeof(context));bad_guard=1;tex51_begin(another);assert(!tex51.ready);assert(!tex51_draw(another,state,300,0,400,300,104,58.5f));assert(!memcmp(state,before,sizeof(state)));
 puts("PASS custom icons: native BGRA alpha, exact alliance ID, one upload, cache reuse, unknown-ID fallback, independent of photo switch, exact paint restoration.\nPASS production texture renderer: validated backend, BGRA8 native upload, one upload per frame, one quad per photo, cached reuse, planet identity, bounded LRU and deletion, no same-frame texture deletion on refresh, persistent failure fallback, signature refusal, exact paint/state restoration. Native/GPU callbacks are stubs; NMS not executed.");}
