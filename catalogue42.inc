#include "catalogue_ui.h"
#include "portal_vectors.h"
#ifndef IGM_CATALOGUE_HOST_TEST
static void catalogue43_release(void){}
#endif
static _Atomic unsigned catalogue42_serial,catalogue42_resets;
static _Atomic ULONGLONG catalogue42_first_prepare,catalogue42_last_draw;
static _Atomic int catalogue42_failed;
static unsigned catalogue42_consumed,catalogue42_reset_seen;
static Ui42State catalogue42_state;
static Ui42Data catalogue42_data;
static Ui42Frame catalogue42_frame;
static ULONGLONG catalogue42_loaded,catalogue42_perf;
static int catalogue42_keys[9],catalogue42_mouse_mode=1;static unsigned short catalogue42_pad;
static float catalogue42_mouse[2]={-100,-100},catalogue42_transform[6];
static char catalogue42_notice[180];static ULONGLONG catalogue42_notice_until;
static void catalogue42_signal(void){ULONGLONG zero=0;atomic_compare_exchange_strong(&catalogue42_first_prepare,&zero,GetTickCount64());unsigned n=atomic_fetch_add(&catalogue42_serial,1);if(!n)log_line("CATALOGUE42 PREPARE: owned Options page; awaiting native renderer");}
static void catalogue42_reset(void){atomic_store(&catalogue42_failed,0);atomic_store(&catalogue42_first_prepare,0);atomic_store(&catalogue42_last_draw,0);atomic_fetch_add(&catalogue42_resets,1);
#ifndef IGM_CATALOGUE_HOST_TEST
 input44_reset();
#endif
}
static int catalogue42_offer(void){if(atomic_load(&catalogue42_failed))return 0;ULONGLONG first=atomic_load(&catalogue42_first_prepare);if(first&&!atomic_load(&catalogue42_last_draw)&&GetTickCount64()-first>1000){if(!atomic_exchange(&catalogue42_failed,1))log_line("CATALOGUE42 FALLBACK: renderer unavailable after Options preparation; native list retained");return 0;}return 1;}
static int catalogue42_was_pressed(int key,int index){int down=(GetAsyncKeyState(key)&0x8000)!=0,edge=down&&!catalogue42_keys[index];catalogue42_keys[index]=down;return edge;}
static void catalogue42_load(void){
 Ui42Data *d=&catalogue42_data;d->ns=d->na=0;d->player_name[0]=0;uint32_t identity58[4];if(player58_get(identity58))alliance_label(identity58,d->player_name);
 static Seen records[4096];size_t n=catalogue_snapshot(records);
 for(size_t i=0;i<n&&d->ns<UI42_LIMIT;i++){Ui42System *s=&d->systems[d->ns++];memset(s,0,sizeof(*s));s->ua=records[i].ua;memcpy(s->id,records[i].id,16);ManagedSystem m=metadata(s->ua);s->flags=m.flags;s->planets=m.planets;snprintf(s->name,128,"%s",m.name);}
 AcquireSRWLockShared(&management_lock);
 for(size_t i=0;i<management.count&&d->ns<UI42_LIMIT;i++){ManagedSystem *m=&management.entries[i];if(!m->flags&&!m->planets)continue;if(ui42_record(d,m->ua)>=0)continue;Ui42System *s=&d->systems[d->ns++];memset(s,0,sizeof(*s));s->ua=m->ua;s->flags=m->flags;s->planets=m->planets;snprintf(s->name,128,"%s",m->name);}ReleaseSRWLockShared(&management_lock);
 for(size_t i=0;i<d->ns;i++){Ui42System *s=&d->systems[i];if(!s->id[0]&&!s->id[1]&&!s->id[2]&&!s->id[3])continue;if(ui42_group(d,s->id)>=0)continue;Ui42Alliance *a=&d->alliances[d->na++];memset(a,0,sizeof(*a));memcpy(a->id,s->id,16);a->own=own_id(s->id);alliance_label(a->id,a->name);get_initials(a->id,a->initials);if(a->own){snprintf(a->name,180,"Imperio Español (IESP)");snprintf(a->initials,7,"IESP");memset(a->rgb,255,3);}else{uint32_t slot;if(!get_alliance_color(a->id,a->rgb,&slot)){a->rgb[0]=40;a->rgb[1]=122;a->rgb[2]=235;}}}
 /* Reserved artwork stays imperial; player membership is a separate GUID. */
 for(size_t i=1;i<d->na;i++)if(d->alliances[i].own){Ui42Alliance a=d->alliances[i];memmove(d->alliances+1,d->alliances,i*sizeof(a));d->alliances[0]=a;break;}
 uint32_t current58[4];if(player58_get(current58))for(size_t i=1;i<d->na;i++)if(!memcmp(d->alliances[i].id,current58,16)){Ui42Alliance a=d->alliances[i];memmove(d->alliances+1,d->alliances,i*sizeof(a));d->alliances[0]=a;break;}
 #ifndef IGM_CATALOGUE_HOST_TEST
 d->show_pirates=atomic_load(&pirates45_enabled);for(size_t i=0;i<d->ns;i++){Ui42System *s=&d->systems[i];Ex45Record n=ex45_record(s->ua,0);if(n.name[0])snprintf(s->name,128,"%s",n.name);for(unsigned p=1;p<16;p++){n=ex45_record(s->ua,p);snprintf(s->planet_names[p],128,"%s",n.name);}}
#endif
 catalogue42_loaded=GetTickCount64();
}
static int catalogue42_apply(const Ui42Item *r){
 Ui42State *s=&catalogue42_state;int wrote=0,ok=1;
 switch(r->action){
 case U42_ALLIANCE:memcpy(s->alliance,r->id,16);s->system=0;s->planet=0;s->spage=0;break;
 case U42_SYSTEM:s->system=r->ua;s->planet=r->planet;break;
 case U42_FAVOURITE:wrote=1;ok=management_change(r->ua,r->planet?4:1,r->planet,NULL);break;
 case U42_DELETE:if(imperial53_find(r->ua))break;s->confirmation=r->ua;s->focus=-1;break;
 case U42_CONFIRM:wrote=1;ok=management_change(r->ua,2,0,NULL);if(ok){s->confirmation=0;s->system=0;s->planet=0;}break;
 case U42_CANCEL:s->confirmation=0;s->focus=-1;break;
 case U42_RESTORE:wrote=1;ok=management_change(r->ua,3,0,NULL);if(ok)s->system=0;break;
 case U42_APAGE:if(r->value<0){if(s->apage)s->apage--;}else s->apage++;break;
 case U42_SPAGE:if(r->value<0){if(s->spage)s->spage--;}else s->spage++;break;
 case U42_TAB:s->tab=r->value;s->apage=s->spage=0;s->system=0;s->planet=0;s->confirmation=0;s->focus=-1;break;
 case U42_TRASH:s->trash=r->value;s->spage=0;s->system=0;break;
 case U42_VISIBILITY:panel.visibility=r->value;
#ifndef IGM_CATALOGUE_HOST_TEST
 atomic_store(&settings45_dirty,1);
#endif
 break;
 case U48_LANGUAGE:if(r->value>=0&&r->value<6){atomic_store(&language48,r->value);
#ifndef IGM_CATALOGUE_HOST_TEST
atomic_store(&settings45_dirty,1);
#endif
}break;
 case U45_PIRATES:
#ifndef IGM_CATALOGUE_HOST_TEST
 atomic_store(&pirates45_enabled,!atomic_load(&pirates45_enabled));atomic_store(&settings45_dirty,1);catalogue42_loaded=0;
#endif
 break;
 case U42_PLANET:{
#ifdef IGM_CATALOGUE_HOST_TEST
 uint64_t root=0,ua=0;Address a;if(read_self(image_base+native.singleton,&root,8)&&root&&read_self(root+0xc1a0,&a,sizeof(a))&&pack_address(&a,&ua)&&a.planet>=1&&a.planet<=15){wrote=1;ok=management_change(ua,4,(unsigned)a.planet,NULL);}
#else
 feature45_add_current();
#endif
 break;}

 case U42_CLOSE:navigate(0);return 1;
 default:return 0;
 }
 if(wrote){catalogue42_loaded=0;if(!ok){snprintf(catalogue42_notice,180,"%s",tr48("No se pudo guardar el cambio. Revisa el registro del mod."));catalogue42_notice_until=GetTickCount64()+5000;}}
 return 1;
}
static void catalogue42_clip(unsigned char *state,float x,float y,float w,float h){float *t=catalogue42_transform;float sc[8]={t[0],0,0,t[3],t[4]+t[0]*(x+w/2),t[5]+t[3]*(y+h/2),w/2,h/2};memcpy(state+0xcc,sc,32);}
static void catalogue42_unclip(unsigned char *state){memset(state+0xcc,0,32);set_float(state+0xe4,-1);set_float(state+0xe8,-1);}
static float catalogue42_text(void *ctx,unsigned char *state,float x,float y,float size,float w,const char *text,float r,float g,float b){
 if(!text[0])return x;
 color(state,r,g,b,1);set_float(state+0xf0,size);set_float(state+0xf4,0);set_float(state+0xf8,1);set_float(state+0xfc,0);int align=1|8;memcpy(state+0x100,&align,4);catalogue42_clip(state,x,y,w,size*1.7f);float end=native_text(ctx,x,y,text,NULL);native_text_count++;catalogue42_unclip(state);return end;
}
static void catalogue42_border(void *ctx,unsigned char *state,float x,float y,float w,float h,float r,float g,float b,float a){IgmPathBatch batch;batch.count=0;batch_rect(&batch,x,y,w,1.3f);batch_rect(&batch,x,y+h-1.3f,w,1.3f);batch_rect(&batch,x,y,1.3f,h);batch_rect(&batch,x+w-1.3f,y,1.3f,h);color(state,r,g,b,a);submit_batch(ctx,&batch);}
static void catalogue42_icon(void *ctx,unsigned char *state,float x,float y,const Ui42Item *r){
#ifndef IGM_CATALOGUE_HOST_TEST
 if(icons52_draw(ctx,state,r->id,x-28,y-28,56,catalogue42_transform,1))return;
#endif
 if(r->own){float t[6];memcpy(t,catalogue42_transform,24);float local[6]={t[0]*1.8f,0,0,t[3]*1.8f,t[4]+t[0]*x,t[5]+t[3]*y};memcpy(state+0xb4,local,24);draw_cross(ctx,state,0,0);memcpy(state+0xb4,t,24);return;}
 float border[8]={x,y-28,x+28,y,x,y+28,x-28,y},inside[8]={x,y-25,x+25,y,x,y+25,x-25,y};color(state,r->rgb[0]/255.f,r->rgb[1]/255.f,r->rgb[2]/255.f,1);polygon(ctx,border,4);color(state,r->rgb[0]/420.f,r->rgb[1]/420.f,r->rgb[2]/420.f,1);polygon(ctx,inside,4);size_t n=strlen(r->initials);float font=n>4?10.f:14.f,w=font*.62f*(float)n;catalogue42_text(ctx,state,x-w/2,y-font*.5f,font,50,r->initials,1,1,1);
}
static void catalogue42_glyphs(void *ctx,unsigned char *state,float x,float y,float size,const char portal[13]){
 static _Thread_local IgmPathBatch batch;batch.count=0;color(state,.92f,.94f,.98f,1);
 for(int i=0;i<12;i++){int g=portal[i]>='0'&&portal[i]<='9'?portal[i]-'0':portal[i]>='A'&&portal[i]<='F'?portal[i]-'A'+10:-1;if(g<0)return;
  for(unsigned k=portal42_offsets[g];k<portal42_offsets[g+1];k++){if(batch.count+10>IGM_BATCH_FLOATS)submit_batch(ctx,&batch);const float *q=portal42_triangles[k];for(int j=0;j<3;j++){batch.commands[batch.count++]=j?1.f:0.f;batch.commands[batch.count++]=x+(float)i*(size+5)+q[j*2]*size;batch.commands[batch.count++]=y+q[j*2+1]*size;}batch.commands[batch.count++]=3;}
 }submit_batch(ctx,&batch);
}
#ifndef IGM_CATALOGUE_HOST_TEST
#include "texture51_render.inc"
#include "icon52_render.inc"
#endif
static void catalogue42_item(void *ctx,unsigned char *state,const Ui42Item *r,int focused){
 float x=r->x,y=r->y,w=r->w,h=r->h;
 if(r->kind==U57_PHOTO){
#ifndef IGM_CATALOGUE_HOST_TEST
 if(tex51_draw(ctx,state,r->ua,r->planet,x,y,w,h))return;
#endif
 catalogue42_text(ctx,state,x+20,y+h/2-12,18,w-40,tr48("Sin captura disponible"),.67f,.70f,.77f);return;
 }
 if(r->action==U42_TAB){if(focused)catalogue42_border(ctx,state,x,y,w,h,.48f,.67f,1,.8f);if(r->selected)ui_rect(ctx,state,x+15,y+h-1,w-30,2,.98f,.79f,.07f,1);ui_rect(ctx,state,x+w+7,y+8,1,h-16,.65f,.65f,.75f,.5f);catalogue42_text(ctx,state,x+20,y+9,21,w-35,r->text,r->selected?1.f:.95f,r->selected?.85f:.95f,r->selected?.3f:1.f);return;}
 if(r->kind==U42_LABEL||r->kind==U42_HEADING){catalogue42_text(ctx,state,x,y,r->kind==U42_HEADING?24:18,w,r->text,.92f,.94f,.98f);return;}
 ui_rect(ctx,state,x,y,w,h,r->selected?.20f:.16f,r->selected?.22f:.15f,r->selected?.34f:.21f,r->selected?.90f:.42f);
 catalogue42_border(ctx,state,x,y,w,h,focused?.48f:.65f,focused?.67f:.65f,focused?1.f:.72f,focused?1.f:r->selected?.65f:.22f);
 ui_rect(ctx,state,x,y,2,h,focused?.48f:.65f,focused?.67f:.65f,focused?1.f:.72f,focused?1.f:.45f);
 if(r->kind==U42_ALLIANCE_CARD){int icon=r->own||r->initials[0];
#ifndef IGM_CATALOGUE_HOST_TEST
 icon=icon||icon52_find(r->id)>=0;
#endif
 if(icon)catalogue42_icon(ctx,state,x+48,y+h/2,r);float tx=x+(icon?94:25);catalogue42_text(ctx,state,tx,y+15,22,w-(tx-x)-24,r->text,.98f,.85f,.16f);catalogue42_text(ctx,state,tx,y+46,16,w-(tx-x)-24,r->subtitle,.67f,.70f,.77f);}
 else if(r->kind==U42_SYSTEM_CARD){
#ifndef IGM_CATALOGUE_HOST_TEST
 if(((catalogue42_state.tab==1&&x<790)||catalogue42_state.tab==0)&&tex51_draw(ctx,state,r->ua,r->planet,x+w-120,y+8,104,58.5f))w-=120;
#endif
 float orb[24];for(int i=0;i<12;i++){float a=(float)i*.5235988f;orb[i*2]=x+30+cosf(a)*10;orb[i*2+1]=y+25+sinf(a)*10;}color(state,.8f,.84f,.95f,1);polygon(ctx,orb,12);
  char prefix[16];snprintf(prefix,sizeof(prefix),"(%s) ",r->initials);size_t plen=strlen(prefix);if(r->initials[0]&&!strncmp(r->text,prefix,plen)){float end=catalogue42_text(ctx,state,x+62,y+11,21,w-84,prefix,.98f,.85f,.16f);if(!isfinite(end)||end<x+63||end>x+w-22)end=x+62+plen*11.f;catalogue42_text(ctx,state,end,y+11,21,x+w-22-end,r->text+plen,.97f,.97f,1);}else catalogue42_text(ctx,state,x+62,y+11,21,w-84,r->text,.97f,.97f,1);catalogue42_glyphs(ctx,state,x+62,y+43,24,r->portal);
  if(r->subtitle[0])draw_favourite(ctx,state,x+w-23,y+h-20);
 }else{
  if(r->action==U42_FAVOURITE)draw_favourite(ctx,state,x+25,y+h/2);
  if(r->action==U42_DELETE){ui_rect(ctx,state,x+18,y+11,12,14,.83f,.84f,.9f,1);ui_rect(ctx,state,x+15,y+7,18,2,.83f,.84f,.9f,1);}
  int icon=r->action==U42_FAVOURITE||r->action==U42_DELETE;float font=r->action==U42_CLOSE?18.f:21.f;catalogue42_text(ctx,state,x+(icon?53:20),y+(h-font)/2,font,w-(icon?70:40),r->text,.95f,.95f,1);
 }
}
static void catalogue44_lifecycle(void){
 if(root39_owned_page&&menu_mode&&!native_root()&&atomic_load(&catalogue42_last_draw)&&GetTickCount64()-atomic_load(&catalogue42_last_draw)>100){menu_mode=0;root39_owned_page=0;
#ifndef IGM_CATALOGUE_HOST_TEST
 input44_reset();
#endif
 log_line("INPUT44 CLOSE: native Options page left; catalogue ownership released");}
}
static int catalogue42_active(void){return catalogue42_design&&!atomic_load(&catalogue42_failed)&&menu_mode&&root39_owned_page&&native_root()&&native_text!=NULL;}
static int catalogue42_draw(void *ctx){
 unsigned serial=atomic_load(&catalogue42_serial);if(!catalogue42_design||atomic_load(&catalogue42_failed)||serial==catalogue42_consumed||!menu_mode||!root39_owned_page||!native_root()||!native_text)return 0;
 HWND window=GetForegroundWindow();DWORD pid=0;GetWindowThreadProcessId(window,&pid);if(pid!=GetCurrentProcessId())return 0;
 uint64_t root=0;float viewport[2];unsigned char vr=0;if(!read_self(image_base+native.singleton,&root,8)||!root||!read_self(root+native.vr_offset,&vr,1)||vr)return 0;get_viewport((void*)(uintptr_t)(root+native.viewport_offset),viewport);if(!isfinite(viewport[0])||!isfinite(viewport[1])||viewport[0]<640||viewport[1]<360||viewport[0]>16384||viewport[1]>16384)return 0;
 unsigned char *state=NULL,saved[264];if(!valid_context(ctx,&state))return 0;uint64_t stash=0;int fonts=0;if(!read_self((uint64_t)(uintptr_t)ctx+0x42c8,&stash,8)||!stash||!read_self(stash+0x74,&fonts,4)||fonts<1||fonts>4096)return 0;
 catalogue42_consumed=serial;atomic_store(&catalogue42_last_draw,GetTickCount64());unsigned reset=atomic_load(&catalogue42_resets);int just_opened=reset!=catalogue42_reset_seen;if(just_opened){catalogue42_reset_seen=reset;memset(&catalogue42_state,0,sizeof(catalogue42_state));catalogue42_state.focus=-1;catalogue42_loaded=0;memset(catalogue42_keys,0,sizeof(catalogue42_keys));catalogue42_pad=0;catalogue42_mouse_mode=1;}
 ULONGLONG start=GetTickCount64();unsigned fills=native_fill_count,texts=native_text_count;if(!catalogue42_loaded||start-catalogue42_loaded>1000)catalogue42_load();
 ui42_make(&catalogue42_data,&catalogue42_state,&catalogue42_frame,panel.visibility);
 if(catalogue42_state.tab==1&&!catalogue42_state.confirmation){uint64_t ua=0;Address a;if(read_self(root+0xc1a0,&a,sizeof(a))&&pack_address(&a,&ua)&&a.planet>=1&&a.planet<=15)ui42_add(&catalogue42_frame,U42_BUTTON,105,776,610,27,tr48("Añadir ubicación actual a favoritos (F8)"),U42_PLANET);}
 float scale=fminf(viewport[0]/1600,viewport[1]/900),tx=(viewport[0]-1600*scale)/2,ty=(viewport[1]-900*scale)/2;float transform[6]={scale,0,0,scale,tx,ty};memcpy(catalogue42_transform,transform,24);
 POINT mouse;RECT client;float mx=-100,my=-100;
#ifdef IGM_CATALOGUE_HOST_TEST
if(GetCursorPos(&mouse)&&ScreenToClient(window,&mouse)&&GetClientRect(window,&client)&&client.right>0&&client.bottom>0){mx=((float)mouse.x*viewport[0]/client.right-tx)/scale;my=((float)mouse.y*viewport[1]/client.bottom-ty)/scale;}
#else
 /* Native cursor sample replaces the former Win32 observer. */
 (void)mouse;(void)client;
#endif

 int click=catalogue42_was_pressed(VK_LBUTTON,0),enter=catalogue42_was_pressed(VK_RETURN,1),tab=catalogue42_was_pressed(VK_TAB,2);int up=catalogue42_was_pressed(VK_UP,3),down=catalogue42_was_pressed(VK_DOWN,4),left=catalogue42_was_pressed(VK_LEFT,5),right=catalogue42_was_pressed(VK_RIGHT,6);
 struct {DWORD packet;unsigned short buttons;unsigned char lt,rt;short axes[4];} gamepad={0};unsigned short pressed=0;if(XInputGetState(0,&gamepad)==0){pressed=gamepad.buttons&~catalogue42_pad;catalogue42_pad=gamepad.buttons;}else catalogue42_pad=0;

#ifndef IGM_CATALOGUE_HOST_TEST
 int native_click=0,native_pointer=input44_get(&mx,&my,&native_click);if(native_pointer){if(native_click)click=1;catalogue42_mouse_mode=1;}
#endif
 int hovered=ui42_hit(&catalogue42_frame,mx,my);if(fabsf(mx-catalogue42_mouse[0])+fabsf(my-catalogue42_mouse[1])>1){catalogue42_mouse_mode=1;if(hovered>=0)catalogue42_state.focus=hovered;}catalogue42_mouse[0]=mx;catalogue42_mouse[1]=my;
 if((pressed&0x1000)&&catalogue42_mouse_mode){click=1;enter=0;}
 up|=(pressed&1)!=0;down|=(pressed&2)!=0;left|=(pressed&4)!=0;right|=(pressed&8)!=0;enter|=((pressed&0x1000)!=0&&!catalogue42_mouse_mode);
 if(tab||up||down||left||right||enter)catalogue42_mouse_mode=0;
 if(click)catalogue42_mouse_mode=1;
 if(tab)catalogue42_state.focus=ui42_next(&catalogue42_frame,catalogue42_state.focus,(GetAsyncKeyState(VK_SHIFT)&0x8000)?-1:1);
 if(up||down||left||right)catalogue42_state.focus=ui42_focus_direction(&catalogue42_frame,catalogue42_state.focus,right-left,down-up);
 if(catalogue42_state.focus<0||(size_t)catalogue42_state.focus>=catalogue42_frame.count||!catalogue42_frame.items[catalogue42_state.focus].action)catalogue42_state.focus=ui42_next(&catalogue42_frame,-1,1);
 int action=just_opened?-1:click?hovered:enter?catalogue42_state.focus:-1;if(action>=0&&(size_t)action<catalogue42_frame.count){Ui42Item item=catalogue42_frame.items[action];catalogue42_apply(&item);if(!menu_mode)return 1;if(!catalogue42_loaded)catalogue42_load();ui42_make(&catalogue42_data,&catalogue42_state,&catalogue42_frame,panel.visibility);}
 memcpy(saved,state,264);drawing=1;float old_alpha=marker_alpha;marker_alpha=1;int font=panel_font_index;if(font<0||font>=fonts)font=0;memcpy(state+0x104,&font,4);memcpy(state+0xb4,transform,24);catalogue42_unclip(state);set_float(state+0xb0,1);
 #ifndef IGM_CATALOGUE_HOST_TEST
 icons52_begin(ctx);
#endif
 ui_rect(ctx,state,65,82,1470,710,.035f,.025f,.065f,.70f);
 catalogue42_text(ctx,state,105,108,24,155,tr48("OPCIONES"),.96f,.96f,1);catalogue42_text(ctx,state,270,108,24,40,"/",.96f,.96f,1);catalogue42_text(ctx,state,315,108,24,1180,"IMPERIAL GALACTIC MAP",.98f,.85f,.16f);
 ui_rect(ctx,state,65,150,580,3,.05f,.70f,.80f,1);ui_rect(ctx,state,645,150,890,3,.98f,.79f,.07f,1);
 catalogue42_text(ctx,state,120,210,27,590,catalogue42_frame.left,.96f,.96f,1);
 int selected_group=ui42_group(&catalogue42_data,catalogue42_state.alliance);
 if(catalogue42_state.tab==0&&selected_group>=0){Ui42Alliance *a=&catalogue42_data.alliances[selected_group];Ui42Item icon={0};memcpy(icon.id,a->id,16);memcpy(icon.rgb,a->rgb,3);snprintf(icon.initials,7,"%s",a->initials);icon.own=a->own;catalogue42_icon(ctx,state,820,224,&icon);catalogue42_text(ctx,state,865,210,24,635,catalogue42_frame.right,.98f,.85f,.16f);}else catalogue42_text(ctx,state,800,210,24,700,catalogue42_frame.right,.96f,.96f,1);
 ui_rect(ctx,state,105,251,610,1,.7f,.7f,.8f,.5f);ui_rect(ctx,state,790,251,710,1,.7f,.7f,.8f,.5f);ui_rect(ctx,state,750,208,1,552,.7f,.7f,.8f,.35f);
 if(catalogue42_state.tab==0)catalogue42_text(ctx,state,802,265,15,680,tr48("S I S T E M A S   C O N O C I D O S"),.70f,.73f,.82f);
 size_t normal=catalogue42_frame.count;if(catalogue42_frame.modal)normal-=4;
 for(size_t i=0;i<normal;i++)catalogue42_item(ctx,state,&catalogue42_frame.items[i],catalogue42_state.focus==(int)i);
 ui_rect(ctx,state,65,804,1470,1,.65f,.65f,.75f,.5f);
 if(catalogue42_frame.modal){ui_rect(ctx,state,65,82,1470,778,.01f,.01f,.025f,.78f);ui_rect(ctx,state,420,335,760,244,.09f,.07f,.15f,.99f);catalogue42_border(ctx,state,420,335,760,244,.65f,.67f,.79f,1);for(size_t i=normal;i<catalogue42_frame.count;i++)catalogue42_item(ctx,state,&catalogue42_frame.items[i],catalogue42_state.focus==(int)i);}
 if(start<catalogue42_notice_until)catalogue42_text(ctx,state,790,780,15,710,catalogue42_notice,1,.65f,.45f);
 if(catalogue42_mouse_mode
#ifndef IGM_CATALOGUE_HOST_TEST
 &&!native_pointer
#endif
 &&mx>=65&&mx<=1535&&my>=82&&my<=860){IgmPathBatch cursor;cursor.count=0;for(int i=0;i<20;i++){float a=i*.314159265f,b=(i+1)*.314159265f;float q[8]={mx+cosf(a)*10,my+sinf(a)*10,mx+cosf(b)*10,my+sinf(b)*10,mx+cosf(b)*8.6f,my+sinf(b)*8.6f,mx+cosf(a)*8.6f,my+sinf(a)*8.6f};for(int j=0;j<4;j++){cursor.commands[cursor.count++]=j?1.f:0.f;cursor.commands[cursor.count++]=q[j*2];cursor.commands[cursor.count++]=q[j*2+1];}cursor.commands[cursor.count++]=3;}color(state,.95f,.96f,1,.95f);submit_batch(ctx,&cursor);}
 memcpy(state,saved,264);marker_alpha=old_alpha;drawing=0;
 if(start-catalogue42_perf>10000){catalogue42_perf=start;log_line("CATALOGUE42 FRAME cpu_ms=%llu fills=%u text=%u items=%zu tab=%d serial=%u one_submission=1",(unsigned long long)(GetTickCount64()-start),native_fill_count-fills,native_text_count-texts,catalogue42_frame.count,catalogue42_state.tab,serial);}return 1;
}
