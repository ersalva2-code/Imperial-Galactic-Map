#define IGM_FEATURE45_HOST 1
#include <assert.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdatomic.h>
#include "core.h"
#include "compat_scan.h"
#include "exploration45.h"
#include "catalogue_management.h"
typedef unsigned long DWORD;typedef void* HWND;
#define VK_F8 0x77
static unsigned char rootdata[0xc200],solar[0x200000],environment[0x500];
static uint64_t root_pointer=(uint64_t)(uintptr_t)rootdata,image_base;
static struct {uint64_t singleton;} native;
static _Atomic uint64_t system45_pointer;
static Ex45Store data;static Management management;static int down,queued,added,env_calls,name_calls,flushes,fail;
static uint64_t queued_ua;static unsigned queued_planet;static char toast[230];
static int read_self(uint64_t p,void*d,size_t n){if(!p)return 0;memcpy(d,(void*)(uintptr_t)p,n);return 1;}
static void log_line(const char*s,...){(void)s;}
static void ex45_remember(uint64_t ua,unsigned planet,int pirate,const char*name,unsigned priority){ex45_put(&data,ua,planet,pirate,name,priority);}
static Ex45Record ex45_record(uint64_t ua,unsigned planet){int at=ex45_find(&data,ua,planet);if(at>=0)return data.items[at];Ex45Record r={.pirate=-1};return r;}
static void ex45_flush(void){flushes++;}
static int management_change(uint64_t ua,int action,unsigned planet,const char*name){if(fail)return 0;ManagedSystem *r=managed_get(&management,ua);if(name){snprintf(r->name,128,"%s",name);return 1;}return managed_action(&management,ua,action,planet);}
static void feature45_toast(const char*s){snprintf(toast,sizeof(toast),"%s",s);}
static void feature45_enqueue(uint64_t ua,unsigned planet){queued++;queued_ua=ua;queued_planet=planet;added++;}
static int feature45_dequeue(uint64_t*ua,unsigned*p){if(!queued)return 0;queued--;*ua=queued_ua;*p=queued_planet;return 1;}
static int GetAsyncKeyState(int key){assert(key==VK_F8);return down?0x8000:0;}
static HWND GetForegroundWindow(void){return (HWND)1;}
static void GetWindowThreadProcessId(HWND w,DWORD*p){(void)w;*p=1;}
static DWORD GetCurrentProcessId(void){return 1;}
#include "features45_live.inc"
static unsigned char env_original(void*p){env_calls++;int loc;memcpy(&loc,(char*)p+0x474,4);return loc>=3&&loc<=8;}
static void names_original(void*p,void*out){(void)p;name_calls++;memset(out,0,0x102);snprintf((char*)out+4,127,"Generated name");snprintf((char*)out+0x83,127,"Nombre personalizado Ñ");}
static void press(void){down=1;hotkey45_poll();hotkey45_poll();down=0;hotkey45_poll();}
int main(void){native.singleton=(uint64_t)(uintptr_t)&root_pointer;Address a={2,68,950,-93,1842,0};uint64_t ua=0;assert(pack_address(&a,&ua));memcpy(rootdata+0xc1a0,&a,sizeof(a));memcpy(solar+0x2670,&ua,8);unsigned planets=6,conflict=3;memcpy(solar+0x2544,&planets,4);memcpy(solar+0x2530,&conflict,4);snprintf((char*)solar+0x2554,128,"Sistema conocido");atomic_store(&system45_pointer,(uint64_t)(uintptr_t)solar);
 int loc=3,nearest=1;memcpy(environment+0x474,&loc,4);memcpy(environment+0x2bc,&nearest,4);env45_original=env_original;assert(hook_env45(environment)&&env_calls==1);uint64_t planetua=ua|(2ULL<<52),planetobject=0x2e30+0xd9170;memcpy(solar+planetobject+8,&planetua,8);snprintf((char*)solar+planetobject+0x3aae,128,"Planeta favorito");press();assert(added==1&&queued_planet==2&&queued_ua==ua);feature45_process();assert(management.entries[0].planets==4&&strstr(toast,"Planeta favorito"));press();feature45_process();assert(management.entries[0].planets==4&&added==2);
 loc=1;memcpy(environment+0x474,&loc,4);assert(!hook_env45(environment));press();assert(queued_planet==0);feature45_process();assert(management.entries[0].flags==1&&management.entries[0].planets==4);
 loc=0;memcpy(environment+0x474,&loc,4);int old=added;press();assert(added==old&&strstr(toast,"confirmar"));
 feature45_observe(root_pointer,ua);assert(ex45_record(ua,0).pirate==1);conflict=99;memcpy(solar+0x2530,&conflict,4);feature45_observe(root_pointer,ua);assert(ex45_record(ua,0).pirate==1);conflict=0;memcpy(solar+0x2530,&conflict,4);feature45_observe(root_pointer,ua);assert(ex45_record(ua,0).pirate==0);
 unsigned char discovery[0x40]={0},output[0x110];int type=2;memcpy(discovery,&planetua,8);memcpy(discovery+0x38,&type,4);names45_original=names_original;hook_names45(discovery,output);assert(name_calls==1&&!strcmp(ex45_record(ua,2).name,"Nombre personalizado Ñ"));
 loc=1;memcpy(environment+0x474,&loc,4);fail=1;press();feature45_process();assert(strstr(toast,"No se pudo guardar"));
 puts("PASS v045 production feature callbacks + F8: original forwarding, held-key debounce, planet capture/name/glyph identity, space rejects stale planet, repeated add preserves favourites, uncertain context refusal, actual conflict=3 classification, unknown not classified, renamed planet capture and write-failure notice. Engine/memory/persistence APIs stubbed; NMS not executed.");
}
