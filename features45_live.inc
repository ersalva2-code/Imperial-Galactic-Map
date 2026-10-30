#include "language48.h"
#include "feature45_patterns.h"
typedef unsigned char (*Env45Fn)(void*);
typedef void (*Names45Fn)(void*,void*);
static Env45Fn env45_original;static Names45Fn names45_original;
static _Atomic uint64_t env45_pointer;
static unsigned char hook_env45(void *p){unsigned char result=env45_original(p);atomic_store(&env45_pointer,(uint64_t)(uintptr_t)p);return result;}
static void hook_names45(void *discovery,void *output){
 uint64_t ua=0;int type=0;read_self((uint64_t)(uintptr_t)discovery,&ua,8);read_self((uint64_t)(uintptr_t)discovery+0x38,&type,4);
 names45_original(discovery,output);
 if(!ua||(type!=1&&type!=2))return;
 unsigned planet=(unsigned)((ua>>52)&15);if((type==1&&planet)||(type==2&&!planet))return;
 char generated[127],custom[127];if(!read_self((uint64_t)(uintptr_t)output+4,generated,127)||!read_self((uint64_t)(uintptr_t)output+0x83,custom,127))return;
 if(!memchr(generated,0,127)||!memchr(custom,0,127))return;
 ex45_remember(ua&0x000fffffffffffffULL,planet,-1,custom[0]?custom:generated,custom[0]?3:2);
}
#ifndef IGM_FEATURE45_HOST
static void feature45_install(uint64_t base){unsigned char *text=VirtualAlloc(NULL,native_text_size,MEM_COMMIT|MEM_RESERVE,PAGE_READWRITE);if(!text)return;if(!read_self(base+native_text_rva,text,native_text_size)){VirtualFree(text,0,MEM_RELEASE);return;}void *detours[2]={(void*)hook_env45,(void*)hook_names45};void **originals[2]={(void**)&env45_original,(void**)&names45_original};for(int i=0;i<2;i++){size_t at=0;int variant=0;if(scan_named(text,native_text_size,&feature45_patterns[i],&at,&variant)){void *target=(void*)(uintptr_t)(base+native_text_rva+at);if(MH_CreateHook(target,detours[i],originals[i])==MH_OK&&MH_EnableHook(target)==MH_OK)log_line("FEATURE45 READY %s",feature45_patterns[i].name);else log_line("FEATURE45 REFUSED %s",feature45_patterns[i].name);}else log_line("FEATURE45 REFUSED signature %s",feature45_patterns[i].name);}VirtualFree(text,0,MEM_RELEASE);}
#endif
static int feature45_system(uint64_t root,uint64_t ua,uint64_t *system){uint64_t p=atomic_load(&system45_pointer),found=0;if(!p||!read_self(p+0x2670,&found,8)||(found&0x000fffffffffffffULL)!=ua)return 0;Address a;uint64_t current=0;if(!read_self(root+0xc1a0,&a,sizeof(a))||!pack_address(&a,&current)||current!=ua)return 0;*system=p;return 1;}
static void feature45_observe(uint64_t root,uint64_t ua){uint64_t p=0;if(!feature45_system(root,ua,&p))return;unsigned conflict=99,planets=99;char name[128];if(read_self(p+0x2554,name,128)&&memchr(name,0,128))ex45_remember(ua,0,-1,name,1);if(!read_self(p+0x2530,&conflict,4)||conflict>3||!read_self(p+0x2544,&planets,4)||planets>16)return;uint64_t again=0;if(feature45_system(root,ua,&again)&&p==again){Ex45Record old=ex45_record(ua,0);ex45_remember(ua,0,conflict==3,NULL,0);if(old.pirate!=(int)(conflict==3))log_line("PIRATE45 observed ua=%016llx conflict=%u pirate=%d source=live_solar_system",(unsigned long long)ua,conflict,conflict==3);}}
static void feature45_add_current(void){DWORD pid=0;GetWindowThreadProcessId(GetForegroundWindow(),&pid);if(pid!=GetCurrentProcessId())return;
 uint64_t root=0,ua=0,env=atomic_load(&env45_pointer);Address a={0},b;int location=0,nearest=-1;unsigned planet=0;
 if(!env||!read_self(image_base+native.singleton,&root,8)||!root||!read_self(root+0xc1a0,&a,sizeof(a))||!pack_address(&a,&ua)||!read_self(env+0x474,&location,4)||!read_self(env+0x2bc,&nearest,4)||!ex45_favourite_target(location,a.planet,nearest,&planet)||!read_self(root+0xc1a0,&b,sizeof(b))||memcmp(&a,&b,sizeof(a))){feature45_toast(tr48("No se pudo confirmar la ubicación. Inténtalo de nuevo."));log_line("FAV45 refused uncertain context location=%d address_planet=%d nearest=%d",location,root?a.planet:-1,nearest);return;}
 uint64_t system=0;if(feature45_system(root,ua,&system)){char name[128];if(read_self(system+0x2554,name,128)&&memchr(name,0,128))ex45_remember(ua,0,-1,name,1);if(planet){uint64_t object=system+0x2e30+(planet-1)*0xd9170ULL,discovery=0;if(read_self(object+8,&discovery,8)&&((discovery>>52)&15)==planet&&(discovery&0x000fffffffffffffULL)==ua&&read_self(object+0x3aae,name,128)&&memchr(name,0,128))ex45_remember(ua,planet,-1,name,1);}}
 feature45_enqueue(ua,planet);log_line("FAV45 queued ua=%016llx planet=%u location=%d",(unsigned long long)ua,planet,location);
}
static void hotkey45_poll(void){static int held;int down=(GetAsyncKeyState(VK_F8)&0x8000)!=0,edge=down&&!held;held=down;if(edge)feature45_add_current();}
#ifndef IGM_FEATURE45_HOST
#include "capture50_async.inc"
#endif
static void feature45_process(void){
#ifndef IGM_FEATURE45_HOST
capture50_poll();
#endif
uint64_t ua;unsigned planet;while(feature45_dequeue(&ua,&planet)){Ex45Record system=ex45_record(ua,0);if(system.name[0])management_change(ua,0,0,system.name);int ok=management_change(ua,planet?6:5,planet,NULL);if(ok){int capture_started=0;
#ifndef IGM_FEATURE45_HOST
 capture_started=capture50_start(ua,planet);
#endif
 if(!capture_started){Ex45Record name=ex45_record(ua,planet);char message[230];snprintf(message,sizeof(message),tr48("Favorito añadido: %s"),name.name[0]?name.name:planet?tr48("planeta (nombre pendiente)"):tr48("sistema (nombre pendiente)"));feature45_toast(message);}}else feature45_toast(tr48("No se pudo guardar el favorito. Revisa el registro."));log_line("FAV45 saved ua=%016llx planet=%u ok=%d",(unsigned long long)ua,planet,ok);}ex45_flush();}
#ifndef IGM_FEATURE45_HOST
static void feature45_toast_draw(void *ctx){char text[230];AcquireSRWLockShared(&feature45_lock);int visible=GetTickCount64()<feature45_toast_until;snprintf(text,sizeof(text),"%s",feature45_toast_text);ReleaseSRWLockShared(&feature45_lock);if(!visible||!native_text)return;unsigned char *state=NULL,saved[264];if(!valid_context(ctx,&state))return;uint64_t root=0;float size[2];if(!read_self(image_base+native.singleton,&root,8)||!root)return;get_viewport((void*)(uintptr_t)(root+native.viewport_offset),size);if(!isfinite(size[0])||!isfinite(size[1])||size[0]<640||size[1]<360)return;uint64_t stash=0;int fonts=0;if(!read_self((uint64_t)(uintptr_t)ctx+0x42c8,&stash,8)||!stash||!read_self(stash+0x74,&fonts,4)||fonts<1)return;memcpy(saved,state,264);drawing=1;float scale=fminf(size[0]/1600,size[1]/900),t[6]={scale,0,0,scale,(size[0]-1600*scale)/2,(size[1]-900*scale)/2};memcpy(state+0xb4,t,24);memset(state+0xcc,0,32);set_float(state+0xe4,-1);set_float(state+0xe8,-1);set_float(state+0xb0,1);int font=panel_font_index>=0&&panel_font_index<fonts?panel_font_index:0;memcpy(state+0x104,&font,4);ui_rect(ctx,state,350,740,900,56,.02f,.02f,.04f,.9f);set_float(state+0xf0,21);set_float(state+0xf4,0);set_float(state+0xf8,1);set_float(state+0xfc,0);float clip[8]={scale,0,0,scale,t[4]+800*scale,t[5]+768*scale,430,28};memcpy(state+0xcc,clip,32);int align=1|8;memcpy(state+0x100,&align,4);color(state,.98f,.85f,.16f,1);native_text(ctx,370,756,text,NULL);memcpy(state,saved,264);drawing=0;}

#endif
