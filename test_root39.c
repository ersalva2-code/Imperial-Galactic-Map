#define IGM_ROOT_MODEL_ONLY
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include <stdatomic.h>
#include "compat_scan.h"
static int read_self(uint64_t p,void *d,size_t n){if(!p)return 0;memcpy(d,(void*)(uintptr_t)p,n);return 1;}
static void log_line(const char *s,...){(void)s;}
static int menu_mode;static void navigate(int m){menu_mode=m;}
#include "root39.inc"
static uint64_t vtable[20];
typedef struct {unsigned char node[0x200],meta[0x240];uint64_t children[32];} Fixture;
static void *assigned_field;static unsigned assignment_calls;
static void *assign_text(void *field,const char *text){assert(!strcmp(text,"Imperial Galactic Map"));assigned_field=field;assignment_calls++;return field;}
static Fixture *all[160];static unsigned nall,requests,clicks;static int click_new;
static void ptr(unsigned char *b,unsigned off,const void *p){uint64_t v=(uint64_t)(uintptr_t)p;memcpy(b+off,&v,8);}
static int type(void *p){int t;memcpy(&t,(char*)p+0x10,4);return t;}
static Fixture *make(int t,const char *id,float y){Fixture *f=calloc(1,sizeof(*f));assert(f);all[nall++]=f;ptr(f->node,0,vtable);memcpy(f->node+0x10,&t,4);ptr(f->node,0x48,f->meta);ptr(f->node,0x60,f->children);uint32_t cap=32;memcpy(f->node+0x58,&cap,4);snprintf((char*)f->meta+0x48,16,"%s",id);float h=32;memcpy(f->meta+0x24,&h,4);memcpy(f->meta+0x30,&y,4);return f;}
static void *add(void *p,void *c,unsigned char end){assert(end==1);Fixture *f=p;uint32_t n;memcpy(&n,f->node+0x5c,4);assert(n<32);f->children[n++]=(uint64_t)(uintptr_t)c;memcpy(f->node+0x5c,&n,4);ptr(((Fixture*)c)->node,0x40,p);return c;}
static void settext(void *p,const char *s){snprintf((char*)((Fixture*)p)->meta+0x80,128,"%s",s);}
static void *clone(void *p){Fixture *src=p;Fixture *f=make(type(p),"COPY",0);memcpy(f->meta,src->meta,sizeof(f->meta));if(type(p)==0){uint32_t n;memcpy(&n,src->node+0x5c,4);for(unsigned i=0;i<n;i++)add(f,clone((void*)(uintptr_t)src->children[i]),1);}return f;}
static void *aliasclone(void *p){Fixture *f=make(0,"BAD",0);ptr(f->node,0x48,((Fixture*)p)->meta);return f;}
static unsigned char click(void *p,void *n,unsigned char a,unsigned char b,const void *action,unsigned char c,unsigned char d){(void)p;(void)action;assert(a==1&&!b&&!c&&d==1);clicks++;uint64_t m;memcpy(&m,(char*)n+0x48,8);return click_new&&(!strcmp((char*)(uintptr_t)(m+0x48),"IGM_MENU42")||!strcmp((char*)(uintptr_t)(m+0x48),"IGM_ACTION40"));}
static void request(int t,unsigned char save){assert(t==6&&!save);requests++;}
static Fixture *buttons[7];static void original(void *p){static const unsigned char action[16]={0};for(unsigned i=0;i<7;i++)root39_click(p,buttons[i],1,0,action,0,1);}
int main(void){
 vtable[1]=(uint64_t)(uintptr_t)type;vtable[0x90/8]=(uint64_t)(uintptr_t)settext;
 Fixture *parent=make(0,"OPTIONS",0);unsigned char before[7][0x240];
 for(unsigned i=0;i<7;i++){char id[16];snprintf(id,16,"ORIGINAL%u",i);buttons[i]=make(0,id,40*i);Fixture *label=make(1,"LABEL",0);snprintf((char*)label->meta+0x80,128,"Original %u",i);add(buttons[i],label,1);add(parent,buttons[i],1);memcpy(before[i],buttons[i]->meta,0x240);}
 root39_clone=clone;root39_add=add;root39_click_original=click;root39_original=original;root39_request=request;
 root39_page((void*)1);uint32_t n;memcpy(&n,parent->node+0x5c,4);assert(n==8&&clicks==8&&requests==0);uint64_t entry=root39_find((uint64_t)(uintptr_t)parent);assert(entry);
 for(unsigned i=0;i<7;i++){assert(parent->children[i]==(uint64_t)(uintptr_t)buttons[i]);assert(!memcmp(before[i],buttons[i]->meta,0x240));Fixture *label=(Fixture*)(uintptr_t)buttons[i]->children[0];char expect[20];snprintf(expect,20,"Original %u",i);assert(!strcmp((char*)label->meta+0x80,expect));}
 Fixture *own=(Fixture*)(uintptr_t)entry;Fixture *label=(Fixture*)(uintptr_t)own->children[0];assert(!strcmp((char*)label->meta+0x80,"Imperial Galactic Map"));float y;memcpy(&y,own->meta+0x30,4);assert(y>240);
 snprintf((char*)label->meta+0x80,128,"General");root39_page((void*)1);assert(!strcmp((char*)label->meta+0x80,"Imperial Galactic Map"));puts("PASS v043 existing added label refreshed after native text overwrite; seven stock labels untouched");
 root39_page((void*)1);memcpy(&n,parent->node+0x5c,4);assert(n==8);click_new=1;root39_page((void*)1);assert(requests==1&&menu_mode==1&&root39_owned_page);
 Fixture *other=make(0,"OTHER",0);Fixture *source=make(0,"SOURCE",0);add(other,source,1);root39_clone=aliasclone;assert(!root39_private((uint64_t)(uintptr_t)source,(uint64_t)(uintptr_t)aliasclone(source),0));root39_button_count=0;assert(!root39_entry((uint64_t)(uintptr_t)source));memcpy(&n,other->node+0x5c,4);assert(n==1&&!strcmp((char*)source->meta+0x48,"SOURCE"));
 root39_clone=clone;click_new=0;Fixture *nested=make(0,"NESTED",0);Fixture *rows[7];
 for(unsigned i=0;i<7;i++){rows[i]=make(0,"ROW",40*i);buttons[i]=make(0,"CLICK",0);Fixture *label=make(1,"LABEL",0);add(buttons[i],label,1);add(rows[i],buttons[i],1);add(nested,rows[i],1);}
 unsigned old_requests=requests;root39_page((void*)1);memcpy(&n,nested->node+0x5c,4);assert(n==8);entry=root39_find((uint64_t)(uintptr_t)nested);assert(entry);uint64_t target=root40_action(entry,0);assert(target&&target!=entry);
 for(unsigned i=0;i<7;i++){assert(nested->children[i]==(uint64_t)(uintptr_t)rows[i]);assert(!strcmp((char*)rows[i]->meta+0x48,"ROW"));assert(!strcmp((char*)buttons[i]->meta+0x48,"CLICK"));}
 click_new=1;root39_page((void*)1);assert(requests==old_requests+1);
 uint64_t selected=0,container=0,peers[32];unsigned peer_count=0;root39_button_count=7;for(unsigned i=0;i<7;i++)root39_buttons[i]=(uint64_t)(uintptr_t)buttons[0];assert(!root40_list(root39_buttons[0],&selected,&container,peers,&peer_count));
 puts("PASS nested native rows: common ancestor, seven distinct peers, eighth row, mapped independent child action, originals intact; duplicate calls do not satisfy peer validation");
 Fixture *labelrow=make(0,"ROWLABEL",0),*hit=make(3,"GRAPHIC",0),*texta=make(1,"NORMAL",0),*textb=make(2,"HOVER",0);
 add(labelrow,hit,1);add(labelrow,texta,1);add(labelrow,textb,1);
 assert(root41_label((uint64_t)(uintptr_t)labelrow,(uint64_t)(uintptr_t)hit)==2);
 assert(!strcmp((char*)texta->meta+0x80,"Imperial Galactic Map")&&!strcmp((char*)textb->meta+0x80,"Imperial Galactic Map"));
 Fixture *deep=make(0,"DEEPLAYER",0),*deeptext=make(1,"VISIBLE_LABEL",0);add(deep,deeptext,1);add(labelrow,deep,1);assert(root41_label((uint64_t)(uintptr_t)labelrow,(uint64_t)(uintptr_t)hit)==3);assert(!strcmp((char*)deeptext->meta+0x80,"Imperial Galactic Map"));puts("PASS v044 all cloned label depths updated including deeper visible labels beside shallow labels");
 Fixture *empty=make(0,"EMPTY",0);assert(!root41_label((uint64_t)(uintptr_t)empty,(uint64_t)(uintptr_t)empty));
 puts("PASS native label regression: graphical click target with sibling normal/hover labels; no-text refusal; independent copy only");
 Fixture *persist=make(1,"PERSISTENT",0);ptr(persist->node,0x188,persist->meta);root42_assign=assign_text;assert(root41_label((uint64_t)(uintptr_t)persist,(uint64_t)(uintptr_t)persist)==1);assert(assignment_calls==1&&assigned_field==persist->meta+0x88);root42_assign=NULL;
 puts("PASS persistent label routes private text metadata to native string allocator; allocator is a host stub, not engine execution");
 for(unsigned i=0;i<nall;i++)free(all[i]);
 puts("PASS production root39 graph: 7 originals remain byte-for-byte; new eighth node, independent label and position; no duplicate per frame; native click requests catalogue; aliased clone refused. Host native APIs are stubs; NMS not executed.");
}
