#include "menu_patterns.h"
#include <stdatomic.h>
typedef void (*OptionsFn)(void);
typedef unsigned char (*ButtonFn)(void*,const char*,const char*,const unsigned char*);
typedef uint64_t (*FinalizeFn)(void*,unsigned char,unsigned char);
typedef void (*HeaderFn)(void*,const char*,int);
typedef const char *(*TranslateFn)(void*,const char*,const char*);
typedef void *(*SystemNameFn)(void*,void*);
static OptionsFn options_original;static ButtonFn button_original;static FinalizeFn finalize_original;
static HeaderFn menu_header;static TranslateFn translate_original;static SystemNameFn system_name_original;
static uint64_t menu_counter_rva,menu_vector_rva;
static _Thread_local int native_scope,injecting,root_seen,menu_rendered;
static _Thread_local void *root_ui;
static _Atomic unsigned menu_options_hits,menu_root_hits,menu_finalize_hits,menu_translate_hits;
static _Atomic int menu_mode;static size_t menu_page;static uint32_t menu_alliance[4];static uint64_t menu_system;static unsigned menu_planet;
static char menu_strings[32][80],menu_display[32][900];static size_t menu_string_count;
static int native_root(void){uint32_t count=0;uint64_t vector=0,last=0;int type=-1;return read_self(image_base+menu_counter_rva,&count,4)&&count>0&&count<=32&&read_self(image_base+menu_vector_rva,&vector,8)&&vector&&read_self(vector+(count-1)*8,&last,8)&&last&&read_self(last,&type,4)&&type==6;}
static const char *menu_string(const char *s,char kind){
 if(menu_string_count>=32)return "IGM37|EMPTY";
 size_t i=menu_string_count++;uint64_t hash=1469598103934665603ULL;
 for(size_t k=0;s[k];k++){hash^=(unsigned char)s[k];hash*=1099511628211ULL;}
 snprintf(menu_strings[i],80,"IGM37|%c%016llx",kind,(unsigned long long)hash);
 snprintf(menu_display[i],900,"%s",s);return menu_strings[i];
}
static const char *menu_label_for_key(const char *key){for(size_t i=0;i<menu_string_count;i++)if(!strcmp(key,menu_strings[i]))return menu_display[i];return NULL;}
static int menu_button(void *ui,const char *s){static const unsigned char options[16]={0};return button_original(ui,menu_string(s,'B'),NULL,options)!=0;}
static void menu_title(void *ui,const char *s){menu_header(ui,menu_string(s,'H'),0);}
static void navigate(int mode){menu_mode=mode;menu_page=0;}
static void system_label(uint64_t ua,unsigned planet,char out[400]){ManagedSystem m=metadata(ua);unsigned galaxy=(unsigned)((ua>>32)&255);char g[40];if(galaxy==0)snprintf(g,sizeof(g),"Euclides");else if(galaxy==9)snprintf(g,sizeof(g),"Eissentam");else snprintf(g,sizeof(g),"Galaxia %u",galaxy+1);snprintf(out,400,"%s%s%u%s (%s)",m.name[0]?m.name:"Nombre pendiente",planet?" / planeta ":"",planet,planet?"":"",g);if(!planet)snprintf(out,400,"%s (%s)",m.name[0]?m.name:"Nombre pendiente",g);}
static size_t catalogue_snapshot(Seen *out){AcquireSRWLockShared(&known_lock);size_t n=known_count;memcpy(out,known,n*sizeof(*out));ReleaseSRWLockShared(&known_lock);return n;}
static int native_home(void *ui){menu_title(ui,"Imperial Galactic Map");if(menu_button(ui,"Alianzas conocidas"))navigate(2);if(menu_button(ui,"Favoritos"))navigate(5);if(menu_button(ui,"Añadir / quitar planeta actual de favoritos")){uint64_t root=0,ua=0;Address a;if(read_self(image_base+native.singleton,&root,8)&&root&&read_self(root+0xc1a0,&a,sizeof(a))&&pack_address(&a,&ua)&&a.planet>=1&&a.planet<=15){management_change(ua,4,(unsigned)a.planet,NULL);}}if(menu_button(ui,"Entradas eliminadas / restaurar"))navigate(7);if(menu_button(ui,"Ajustes de marcadores"))navigate(8);if(menu_button(ui,"Volver a Opciones"))navigate(0);return 1;}
static void render_native_menu(void *ui){
 menu_string_count=0;
#if !defined(IGM_MENU_MODEL_ONLY) && !defined(IGM_MENU_ROUTING_TEST)
 if(menu_mode&&catalogue42_design&&native_text&&catalogue42_offer()){catalogue42_signal();return;}
#endif
 if(menu_mode==0){if(menu_button(ui,"Imperial Galactic Map"))navigate(1);
  return;}
 if(menu_mode==1){native_home(ui);return;}
 Seen snapshot[4096];size_t count=catalogue_snapshot(snapshot);char line[850];
 if(menu_mode==2){
  menu_title(ui,"Alianzas conocidas");uint32_t ids[4096][4];size_t n=0;
  for(size_t i=0;i<count;i++){if(is_excluded(snapshot[i].ua))continue;size_t k;for(k=0;k<n;k++)if(!memcmp(ids[k],snapshot[i].id,16))break;if(k==n)memcpy(ids[n++],snapshot[i].id,16);}
  for(size_t i=menu_page*4;i<n&&i<menu_page*4+4;i++){char name[180];alliance_label(ids[i],name);if(menu_button(ui,name)){memcpy(menu_alliance,ids[i],16);navigate(3);}}
  if(n>4&&menu_button(ui,"Página siguiente"))menu_page=(menu_page+1)%((n+3)/4);
  if(menu_button(ui,"Volver"))navigate(1);
  return;
 }
 if(menu_mode==3){
  char alliance[180];alliance_label(menu_alliance,alliance);menu_title(ui,alliance);size_t found=0;
  char initials[7];get_initials(menu_alliance,initials);
  for(size_t i=0;i<count;i++){if(memcmp(snapshot[i].id,menu_alliance,16)||is_excluded(snapshot[i].ua))continue;if(found>=menu_page*4&&found<menu_page*4+4){char name[400];system_label(snapshot[i].ua,0,name);snprintf(line,sizeof(line),"(%s) %s",initials[0]?initials:"?",name);if(menu_button(ui,line)){menu_system=snapshot[i].ua;menu_planet=0;navigate(4);}}found++;}
  if(found>4&&menu_button(ui,"Página siguiente"))menu_page=(menu_page+1)%((found+3)/4);
  if(menu_button(ui,"Volver a alianzas"))navigate(2);
  return;
 }
 if(menu_mode==4){
  char name[400],glyphs[300],portal[13];system_label(menu_system,menu_planet,name);menu_title(ui,name);portal_address(menu_system,menu_planet,glyphs);portal_address(menu_system,menu_planet,portal);
  snprintf(line,sizeof(line),"Dirección del portal: %s",glyphs);menu_button(ui,line);
  snprintf(line,sizeof(line),"Portal %s / planeta de entrada %u",portal,menu_planet?menu_planet:1);menu_button(ui,line);
  ManagedSystem m=metadata(menu_system);int favourite=menu_planet?(m.planets&(1u<<menu_planet)):(m.flags&1);
  if(menu_button(ui,favourite?"Quitar de favoritos":"Añadir a favoritos"))management_change(menu_system,menu_planet?4:1,menu_planet,NULL);
  if(!imperial53_find(menu_system)&&menu_button(ui,"Eliminar este sistema del catálogo"))navigate(6);
  if(menu_button(ui,"Volver al catálogo de la alianza"))navigate(3);
  return;
 }
 if(menu_mode==5||menu_mode==7){
  int deleted=menu_mode==7;menu_title(ui,deleted?"Entradas eliminadas":"Favoritos");size_t seen=0;
  AcquireSRWLockShared(&management_lock);static ManagedSystem items[4096];size_t n=management.count;memcpy(items,management.entries,n*sizeof(*items));ReleaseSRWLockShared(&management_lock);
  for(size_t i=0;i<n;i++)for(unsigned planet=0;planet<16;planet++){
   if(deleted){if(planet||!(items[i].flags&2))continue;}else{if(items[i].flags&2)continue;if(planet?!(items[i].planets&(1u<<planet)):!(items[i].flags&1))continue;}
   if(seen>=menu_page*4&&seen<menu_page*4+4){system_label(items[i].ua,planet,line);if(menu_button(ui,line)){menu_system=items[i].ua;menu_planet=planet;if(deleted){management_change(menu_system,3,0,NULL);}else{for(size_t k=0;k<count;k++)if(snapshot[k].ua==menu_system){memcpy(menu_alliance,snapshot[k].id,16);break;}navigate(4);}}}seen++;
  }
  if(seen>4&&menu_button(ui,"Página siguiente"))menu_page=(menu_page+1)%((seen+3)/4);
  if(menu_button(ui,"Volver"))navigate(1);
  return;
 }
 if(menu_mode==6){menu_title(ui,"Eliminar entrada del mod");menu_button(ui,"No cambia la propiedad ni los descubrimientos");if(menu_button(ui,"Confirmar eliminación")){management_change(menu_system,2,0,NULL);navigate(3);}if(menu_button(ui,"Cancelar"))navigate(4);return;}
 if(menu_mode==8){menu_title(ui,"Visibilidad de marcadores");const char *modes[4]={"Mostrar todos","Solo mi alianza","Solo otras alianzas","Ocultar todos"};for(int i=0;i<4;i++)if(menu_button(ui,modes[i]))panel.visibility=i;if(menu_button(ui,"Volver"))navigate(1);}
}
#ifndef IGM_MENU_MODEL_ONLY
#include "root39.inc"
/* The native root button is the primary routing evidence. Do not require a
   speculative root-stack read before this actual UI call has occurred. */
static int root_button_key(const char *key){return key&&(!strcmp(key,"UI_OPTIONS_GENERAL_L")||!strcmp(key,"UI_OPTIONS_GENERAL"));}
static void inject_menu(void *ui){
 if(menu_rendered||injecting)return;
 menu_rendered=1;injecting=1;render_native_menu(ui);injecting=0;
}
static void hook_options(void){
 int before=native_scope,seen=root_seen,rendered=menu_rendered;void *previous=root_ui;
 native_scope=1;root_seen=0;menu_rendered=0;root_ui=NULL;
 if(menu_options_hits++<3)log_line("MENU42 OPTIONS ENTER root_snapshot=%d",native_root());
 options_original();
 if(root39_owned_page&&!menu_mode&&native_root()){root39_owned_page=0;root39_pop(0);log_line("MENU42 CLOSE returned through native page pop");}
 if(menu_options_hits<=3)log_line("MENU42 OPTIONS EXIT root_button=%d injected=%d",root_seen,menu_rendered);
 native_scope=before;root_seen=seen;menu_rendered=rendered;root_ui=previous;
}
static unsigned char hook_button(void *ui,const char *key,const char *description,const unsigned char *options){
 if(!injecting&&root_button_key(key)){
  root_seen=1;root_ui=ui;
  if(menu_root_hits++<6){int pass=-1;read_self((uint64_t)(uintptr_t)ui+8,&pass,4);log_line("MENU42 ROOT BUTTON key=%s pass=%d scope=%d snapshot=%d",key,pass,native_scope,native_root());}
  /* Insert before stock rows so it participates in their native layout pass. */
  if(!native_scope){menu_rendered=0;} /* observed root outside DoOptions */
  inject_menu(ui);
 }
 if(!injecting&&root_seen&&ui==root_ui&&menu_mode)return 0;
 return button_original(ui,key,description,options);
}
static uint64_t hook_finalize(void *ui,unsigned char a,unsigned char b){
 if(native_scope){
  if(menu_finalize_hits++<6)log_line("MENU42 FINALIZE root_button=%d injected=%d snapshot=%d",root_seen,menu_rendered,native_root());
  if(!menu_rendered&&(root_seen||native_root()))inject_menu(ui);
 }
 uint64_t result=finalize_original(ui,a,b);
 if(!native_scope&&ui==root_ui){root_seen=0;root_ui=NULL;menu_rendered=0;}
 return result;
}
static const char *hook_translate(void *self,const char *key,const char *fallback){
#if !defined(IGM_MENU_ROUTING_TEST)
 if(key&&catalogue42_design&&root39_owned_page&&native_root()&&(!strcmp(key,"UI_OPTIONS")||!strcmp(key,"UI_OPTIONS_L")))return "";
#endif
 if(key&&!strcmp(key,"IGM39_ROOT")){if(atomic_fetch_add(&root39_label_calls,1)<3)log_line("MENU42 ROOT LABEL translated");return "Imperial Galactic Map";}
 if(key&&!strncmp(key,"IGM37|",6)){const char *label=menu_label_for_key(key);if(label){if(menu_translate_hits++<6)log_line("MENU42 LABEL translated=%s",label);return label;}}
 return translate_original(self,key,fallback);
}
#ifndef IGM_MENU_ROUTING_TEST
#include "native_input44.inc"
#include "features45_live.inc"
static void *hook_system_name(void *system,void *output){void *result=system_name_original(system,output);uint64_t ua=0,again=0,root=0,current=0;Address a;char name[128];if(read_self((uint64_t)(uintptr_t)system+0x2670,&ua,8)&&read_self((uint64_t)(uintptr_t)output,name,128)&&memchr(name,0,128)&&name[0]&&read_self((uint64_t)(uintptr_t)system+0x2670,&again,8)&&ua==again&&read_self(image_base+native.singleton,&root,8)&&root&&read_self(root+0xc1a0,&a,sizeof(a))&&pack_address(&a,&current)&&(ua&0x000fffffffffffffULL)==current){atomic_store(&system45_pointer,(uint64_t)(uintptr_t)system);ex45_remember(current,0,-1,name,3);ManagedSystem m=metadata(current);if(strcmp(m.name,name))management_change(current,0,0,name);}return result;}
static void install_native_menu(uint64_t base){
 if(!native_menu_enabled){log_line("NATIVE MENU disabled by INI; F8 adds favourites");return;}
 unsigned char *text=VirtualAlloc(NULL,native_text_size,MEM_COMMIT|MEM_RESERVE,PAGE_READWRITE);if(!text)return;
 size_t root_positions[7]={0};
 size_t positions[6]={0};int v=0,ok=read_self(base+native_text_rva,text,native_text_size);
 for(int i=0;ok&&i<5;i++)if(!scan_named(text,native_text_size,&menu_patterns[i],&positions[i],&v))ok=0;
 if(ok)scan_named(text,native_text_size,&menu_patterns[5],&positions[5],&v);
 for(int i=0;ok&&i<7;i++)if(!scan_named(text,native_text_size,&root39_patterns[i],&root_positions[i],&v))ok=0;
 if(!ok){VirtualFree(text,0,MEM_RELEASE);log_line("NATIVE MENU REFUSED: missing/ambiguous signatures; F8 favourites remain");return;}
 size_t options=positions[2];uint32_t image_size=0x10000000;
 menu_counter_rva=relative_target(text,native_text_size,native_text_rva,options+0x17,2,7,image_size);
 menu_vector_rva=relative_target(text,native_text_size,native_text_rva,options+0x63,3,7,image_size);
 if(!menu_counter_rva||!menu_vector_rva||text[options+0x63]!=0x48||text[options+0x64]!=0x8b){VirtualFree(text,0,MEM_RELEASE);log_line("NATIVE MENU REFUSED: options globals not validated");return;}
 size_t input_position=0;int input_found=scan_named(text,native_text_size,&input44_patterns[0],&input_position,&v);
 VirtualFree(text,0,MEM_RELEASE);
 void *targets[7]={(void*)(uintptr_t)(base+native_text_rva+positions[2]),(void*)(uintptr_t)(base+native_text_rva+positions[3]),(void*)(uintptr_t)(base+native_text_rva+positions[4]),(void*)(uintptr_t)(base+native_text_rva+positions[0]),(void*)(uintptr_t)(base+native_text_rva+root_positions[0]),(void*)(uintptr_t)(base+native_text_rva+root_positions[4]),NULL};
 menu_header=(HeaderFn)(uintptr_t)(base+native_text_rva+positions[1]);
 root39_add=(NodeAddFn)(uintptr_t)(base+native_text_rva+root_positions[1]);
 root39_request=(RequestPageFn)(uintptr_t)(base+native_text_rva+root_positions[2]);
 root39_clone=(NodeCloneFn)(uintptr_t)(base+native_text_rva+root_positions[3]);
 root39_pop=(PopPageFn)(uintptr_t)(base+native_text_rva+root_positions[5]);
 root42_assign=(Root42AssignFn)(uintptr_t)(base+native_text_rva+root_positions[6]);
 MH_STATUS status=MH_Initialize();if(status!=MH_OK&&status!=MH_ERROR_ALREADY_INITIALIZED)return;
 void *detours[6]={(void*)hook_options,(void*)hook_button,(void*)hook_finalize,(void*)hook_translate,(void*)root39_page,(void*)root39_click};
 void **originals[6]={(void**)&options_original,(void**)&button_original,(void**)&finalize_original,(void**)&translate_original,(void**)&root39_original,(void**)&root39_click_original};
 int created=0;
 for(int i=0;i<6;i++){status=MH_CreateHook(targets[i],detours[i],originals[i]);if(status!=MH_OK)break;created++;}
 if(status!=MH_OK){for(int i=0;i<created;i++)MH_RemoveHook(targets[i]);log_line("NATIVE MENU42 REFUSED hook=%d",status);return;}
 for(int i=0;i<6;i++)MH_QueueEnableHook(targets[i]);status=MH_ApplyQueued();
 if(status!=MH_OK){for(int i=0;i<6;i++)MH_DisableHook(targets[i]);log_line("NATIVE MENU42 REFUSED enable=%d",status);return;}

 if(input_found){void *target=(void*)(uintptr_t)(base+native_text_rva+input_position);if(MH_CreateHook(target,(void*)hook_input44,(void**)&input44_original)==MH_OK&&MH_EnableHook(target)==MH_OK)log_line("INPUT44 READY: native BeginInput cursor + confirmation; no Windows input observer");else log_line("INPUT44 REFUSED hook unavailable");}else log_line("INPUT44 REFUSED pattern missing/ambiguous");
 int name_capture=0;
 if(positions[5]){targets[6]=(void*)(uintptr_t)(base+native_text_rva+positions[5]);if(MH_CreateHook(targets[6],(void*)hook_system_name,(void**)&system_name_original)==MH_OK)name_capture=MH_EnableHook(targets[6])==MH_OK;}
 log_line("NATIVE MENU42 READY: DoGameSwitcher + independent appended node; originals preserved; system name capture=%d",name_capture);
}

#endif

#endif /* routing test */
