#include "texture51_patterns.h"
typedef int(*Tex51Create)(void*,unsigned char,int,int,int,const unsigned char*);
typedef int(*Tex51Delete)(void*,int);
typedef int(*Tex51Size)(void*,int,int*,int*);
static struct {void*ctx;uint64_t backend,create,remove,size;unsigned frame,uploads,epoch;int ready,blocked;struct {uint64_t ua;unsigned planet,generation,used;int id,w,h;} slots[16];} tex51;
static int tex51_check(uint64_t fn,const unsigned char*b,const unsigned char*m,size_t n){unsigned char code[256];if(fn<image_base+native_text_rva||n>sizeof(code)||fn-image_base-native_text_rva>native_text_size||n>native_text_size-(fn-image_base-native_text_rva)||!read_self(fn,code,n))return 0;for(size_t i=0;i<n;i++)if(m[i]&&code[i]!=b[i])return 0;return 1;}
static void tex51_begin(void*ctx){uint64_t table[7];tex51.frame++;tex51.uploads=0;if(!read_self((uint64_t)(uintptr_t)ctx,table,sizeof(table))){tex51.ready=0;return;}
 if(tex51.ctx==ctx&&tex51.backend==table[0]&&tex51.create==table[3]&&tex51.remove==table[4]&&tex51.size==table[6]&&(tex51.ready||tex51.blocked))return;
 /* Never call stale handles when context/backend changes. Engine owns their old lifetime. */
 tex51.epoch++;memset(tex51.slots,0,sizeof(tex51.slots));tex51.ready=0;tex51.blocked=0;tex51.ctx=ctx;tex51.backend=table[0];tex51.create=table[3];tex51.remove=table[4];tex51.size=table[6];
 unsigned char jump[8];uint64_t dest=0;int32_t rel=0;
 if(table[0]&&read_self(table[4],jump,8)&&!memcmp(jump,"\x45\x33\xc0\xe9",4)){memcpy(&rel,jump+4,4);dest=table[4]+8+rel;}
 tex51.ready=tex51_check(table[3],tex51_create_bytes,tex51_create_mask,sizeof(tex51_create_bytes))&&tex51_check(table[6],tex51_size_bytes,tex51_size_mask,sizeof(tex51_size_bytes))&&tex51_check(dest,tex51_delete_target_bytes,tex51_delete_target_mask,sizeof(tex51_delete_target_bytes));
 static int reported=-1;if(reported!=tex51.ready){reported=tex51.ready;log_line("TEXTURE51 backend %s; create=%llx size=%llx delete=%llx",tex51.ready?"VERIFIED":"REFUSED (text-only fallback)",(unsigned long long)table[3],(unsigned long long)table[6],(unsigned long long)table[4]);}
}
static int tex51_draw(void*ctx,unsigned char*state,uint64_t ua,unsigned planet,float x,float y,float w,float h){if(!tex51.ready||tex51.ctx!=ctx||!photos51_enabled)return 0;
 static _Thread_local Photo51 photo;unsigned generation=0;if(!photos51_get(ua,planet,NULL,&generation))return 0;
 int at=-1,old=-1;unsigned oldest=UINT32_MAX;for(int i=0;i<16;i++){if(tex51.slots[i].ua==ua&&tex51.slots[i].planet==planet){at=i;break;}if(tex51.slots[i].used!=tex51.frame&&tex51.slots[i].used<oldest){oldest=tex51.slots[i].used;old=i;}}
 if(at<0)at=old;if(at<0)return 0;
 if(tex51.slots[at].ua!=ua||tex51.slots[at].planet!=planet||tex51.slots[at].generation!=generation||tex51.slots[at].id<=0){if(tex51.slots[at].id>0&&tex51.slots[at].used==tex51.frame)return 0;if(tex51.uploads)return 0;if(!photos51_get(ua,planet,&photo,&generation))return 0;tex51.uploads++;
  if(tex51.slots[at].id>0)((Tex51Delete)(uintptr_t)tex51.remove)((void*)(uintptr_t)tex51.backend,tex51.slots[at].id);
  memset(&tex51.slots[at],0,sizeof(tex51.slots[at]));int id=((Tex51Create)(uintptr_t)tex51.create)((void*)(uintptr_t)tex51.backend,7,(int)photo.w,(int)photo.h,0,photo.pixels);if(id<=0){tex51.ready=0;tex51.blocked=1;log_line("TEXTURE51 create failed; previews disabled for context");return 0;}
  tex51.slots[at].ua=ua;tex51.slots[at].planet=planet;tex51.slots[at].generation=generation;tex51.slots[at].id=id;tex51.slots[at].w=(int)photo.w;tex51.slots[at].h=(int)photo.h;log_line("TEXTURE51 uploaded ua=%016llx planet=%u image=%d size=%ux%u",(unsigned long long)ua,planet,id,photo.w,photo.h);
 }
 int iw=0,ih=0;((Tex51Size)(uintptr_t)tex51.size)((void*)(uintptr_t)tex51.backend,tex51.slots[at].id,&iw,&ih);if(iw!=tex51.slots[at].w||ih!=tex51.slots[at].h){tex51.slots[at].id=0;tex51.slots[at].generation=0;return 0;}
 tex51.slots[at].used=tex51.frame;float scale=fminf(w/iw,h/ih),dw=iw*scale,dh=ih*scale,dx=x+(w-dw)/2,dy=y+(h-dh)/2;
 unsigned char saved[80];memcpy(saved,state,80);color(state,1,1,1,1);float*t=catalogue42_transform;set_float(state,t[0]);set_float(state+12,t[3]);set_float(state+16,t[4]+t[0]*dx);set_float(state+20,t[5]+t[3]*dy);set_float(state+24,dw);set_float(state+28,dh);memcpy(state+72,&tex51.slots[at].id,4);
 IgmPathBatch batch;batch.count=0;batch_rect(&batch,dx,dy,dw,dh);submit_batch(ctx,&batch);memcpy(state,saved,80);return 1;
}
