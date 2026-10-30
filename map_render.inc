/* Experimental adapter for the exact supplied Forsaken 7.10 executable.
   No marker arrays are overwritten: independent vector paths are appended to NVG. */
#include <math.h>
#include "MinHook.h"

#include "map_style.h"
#include "alliance_palette.inc"

typedef void (*MapRenderFn)(void*,void*,unsigned char);
typedef void (*EndFrameFn)(void*);
typedef void (*AppendFn)(void*,float*,int);
typedef void (*FillFn)(void*);
typedef void *(*ViewportFn)(void*,float*);
typedef void (*ProjectFn)(void*,const float*,float,const float*,float*,unsigned int,unsigned char,const float*);
static MapRenderFn original_map;
static EndFrameFn original_end;
static AppendFn append_commands;
static FillFn fill_path;
static ViewportFn get_viewport;
static ProjectFn project_points;
static uint64_t image_base;
static volatile LONG overlay_enabled=1;
static _Thread_local void *active_map;
static _Thread_local int submitted_in_scope;
static _Thread_local int drawing;
static uint64_t reported_systems[64]; static size_t reported_count;
static ULONGLONG last_map_report;
static unsigned scale_percent;
static float marker_alpha=.9f,fade_start=300,fade_end=1200;
typedef float (*NativeTextFn)(void*,float,float,const char*,const char*);
static NativeTextFn native_text;
static unsigned native_text_count;
static int panel_font_index;
static int native_menu_enabled=1;


static void migrate_journal(void) {
    wchar_t oldpath[32768],newpath[32768],bin[32768],config[32768];
    if(!GetModuleFileNameW(self_module,bin,32768))return;
    wchar_t *sep=wcsrchr(bin,L'\\');if(!sep)return;*sep=0;
    if(make_path(config,L"IGM.ini"))
        {scale_percent=GetPrivateProfileIntW(L"Visual",L"ScalePercent",0,config);
        catalogue42_design=GetPrivateProfileIntW(L"Interface",L"CatalogueDesign",1,config)!=0;
        native_menu_enabled=GetPrivateProfileIntW(L"Interface",L"EnableNativeMenu",1,config)!=0;
        panel_font_index=(int)GetPrivateProfileIntW(L"Interface",L"FontIndex",0,config);
        if(panel_font_index<0||panel_font_index>4095)panel_font_index=0;
        fade_start=(float)GetPrivateProfileIntW(L"Visual",L"FadeStart",300,config);
        fade_end=(float)GetPrivateProfileIntW(L"Visual",L"FadeEnd",1200,config);
        if(fade_start<0||fade_end<=fade_start||fade_end>100000){fade_start=300;fade_end=1200;}}
    make_path(newpath,L"catalogo_observado.jsonl");
    const wchar_t *prior[]={L"IGM_v030_datos",L"IGM_v029_datos",L"IGM_v028_datos"};
    for(size_t i=0;i<3;i++) {
        if(swprintf(oldpath,32768,L"%ls\\%ls\\catalogo_observado.jsonl",bin,prior[i])<0)continue;
        if(CopyFileW(oldpath,newpath,TRUE)){log_line("MIGRATED: observed journal copied from prior version %d; original retained",30-(int)i);break;}
        if(GetLastError()==ERROR_FILE_EXISTS)break;
    }
}
static size_t visible_systems(Seen out[4096],unsigned galaxy,size_t *imperial,size_t *other) {
    AcquireSRWLockShared(&known_lock);
    size_t n=select_catalogue(known,known_count,galaxy,out,4096,imperial,other);
    size_t kept=0;for(size_t i=0;i<n;i++)if(!is_excluded(out[i].ua))out[kept++]=out[i];n=kept;
    ReleaseSRWLockShared(&known_lock);
    AcquireSRWLockShared(&management_lock);for(size_t i=0;i<management.count&&n<4096;i++){
        ManagedSystem *m=&management.entries[i];if((m->flags&2)||(!(m->flags&1)&&!m->planets)||((m->ua>>32)&255)!=galaxy)continue;
        size_t k;for(k=0;k<n;k++)if(out[k].ua==m->ua)break;if(k==n){memset(&out[n],0,sizeof(out[n]));out[n++].ua=m->ua;}
    }ReleaseSRWLockShared(&management_lock);
    if(atomic_load(&pirates45_enabled)){AcquireSRWLockShared(&ex45_lock);for(size_t i=0;i<ex45_store.count&&n<4096;i++){Ex45Record *r=&ex45_store.items[i];if(r->planet||r->pirate!=1||((r->ua>>32)&255)!=galaxy||is_excluded(r->ua))continue;size_t k;for(k=0;k<n;k++)if(out[k].ua==r->ua)break;if(k==n){memset(&out[n],0,sizeof(out[n]));out[n++].ua=r->ua;}}ReleaseSRWLockShared(&ex45_lock);}return n;
}
#include "position54.h"
static Pos54Store positions54;
static Pos54Request voxel_request54;
#include "position54_render.inc"
static void set_float(unsigned char *at,float value){memcpy(at,&value,4);}
static void color(unsigned char *state,float r,float g,float b,float a) {
    unsigned char *paint=state;memset(paint,0,80);
    set_float(paint,1);set_float(paint+12,1);set_float(paint+36,1);
    float rgba[4]={r,g,b,a};memcpy(paint+40,rgba,16);memcpy(paint+56,rgba,16);
}
#include "path_batch.h"
static _Thread_local IgmPathBatch text_batch;
static unsigned native_fill_count;
static void submit_batch(void *ctx,IgmPathBatch *batch){
    if(!batch->count)return;
    uint64_t cache=0;memcpy(&cache,(unsigned char*)ctx+0x42b0,8);
    *(int*)((unsigned char*)ctx+0x9c)=0;
    *(int*)(uintptr_t)(cache+8)=0;*(int*)(uintptr_t)(cache+24)=0;
    append_commands(ctx,batch->commands,(int)batch->count);fill_path(ctx);native_fill_count++;batch->count=0;
}
static void polygon(void *ctx,const float *xy,int points) {
    float commands[40];if(points<3||points>12)return;
    uint64_t cache=0;memcpy(&cache,(unsigned char*)ctx+0x42b0,8);
    *(int*)((unsigned char*)ctx+0x9c)=0;
    *(int*)(uintptr_t)(cache+8)=0;*(int*)(uintptr_t)(cache+24)=0;
    for(int i=0;i<points;i++){commands[i*3]=i?1.0f:0.0f;commands[i*3+1]=xy[i*2];commands[i*3+2]=xy[i*2+1];}
    commands[points*3]=3;
    append_commands(ctx,commands,points*3+1);fill_path(ctx);native_fill_count++;
}
static void plank(void *ctx,float cx,float cy,float ax,float ay,float bx,float by,float width) {
    float dx=bx-ax,dy=by-ay,len=sqrtf(dx*dx+dy*dy);if(len<0.01f)return;
    float nx=-dy*width/(2*len),ny=dx*width/(2*len);
    float poly[8]={ax+nx,ay+ny,bx+nx,by+ny,bx-nx,by-ny,ax-nx,ay-ny};
    for(int i=0;i<4;i++) {
        float span=fabsf(poly[2*i])+fabsf(poly[2*i+1]);
        if(span>10.9f){poly[2*i]*=10.9f/span;poly[2*i+1]*=10.9f/span;}
        poly[2*i]+=cx;poly[2*i+1]+=cy;
    }
    polygon(ctx,poly,4);
}
static void draw_favourite(void *ctx,unsigned char *state,float x,float y){
    float points[20];for(int i=0;i<10;i++){float a=(float)i*0.62831853f-1.57079633f,r=i%2?2.8f:6.4f;points[i*2]=x+cosf(a)*r;points[i*2+1]=y+sinf(a)*r;}color(state,1,.77f,.12f,marker_alpha);polygon(ctx,points,10);
}
static void draw_cross(void *ctx,unsigned char *state,float x,float y) {
    float outside[8]={x,y-13.2f,x+13.2f,y,x,y+13.2f,x-13.2f,y};
    color(state,.25f,.25f,.25f,marker_alpha);polygon(ctx,outside,4);
    float inside[8]={x,y-11.6f,x+11.6f,y,x,y+11.6f,x-11.6f,y};
    color(state,1,1,1,marker_alpha);polygon(ctx,inside,4);
    color(state,.77f,.08f,.17f,marker_alpha);
    for(int diagonal=-1;diagonal<=1;diagonal+=2) {
        plank(ctx,x,y,-5.4f,-5.4f*diagonal,5.4f,5.4f*diagonal,2.3f);
        for(int t=-3;t<=3;t+=3) for(int side=-1;side<=1;side+=2) {
            float a=(float)t,b=a*diagonal;
            plank(ctx,x,y,a,b,a+side*1.8f,b-side*diagonal*1.8f,1.4f);
        }
    }
}
static int writable_region(uint64_t at,size_t size) {
    MEMORY_BASIC_INFORMATION region;
    if(!VirtualQuery((void*)(uintptr_t)at,&region,sizeof(region))||region.State!=MEM_COMMIT||
       (region.Protect&(PAGE_GUARD|PAGE_NOACCESS)))return 0;
    DWORD access=region.Protect&255;
    if(access!=PAGE_READWRITE&&access!=PAGE_WRITECOPY&&access!=PAGE_EXECUTE_READWRITE&&access!=PAGE_EXECUTE_WRITECOPY)return 0;
    uint64_t start=(uint64_t)(uintptr_t)region.BaseAddress;
    return at>=start&&size<=region.RegionSize&&at-start<=region.RegionSize-size;
}
static int valid_context(void *ctx,unsigned char **state) {
    int states=0;uint64_t cache=0;unsigned char probe[264];
    uint64_t c=(uint64_t)(uintptr_t)ctx;
    if(!read_self(c+0x42a8,&states,4)||states<1||states>64||!read_self(c+0x42b0,&cache,8)||!cache)return 0;
    *state=(unsigned char*)ctx+0xa8+(states-1)*264;
    if(!read_self((uint64_t)(uintptr_t)*state,probe,264)||!writable_region((uint64_t)(uintptr_t)*state,264)||
       !writable_region(c+0x9c,4)||!writable_region(cache+8,4)||!writable_region(cache+24,4))return 0;
    int counts[2];return read_self(cache+8,&counts[0],4)&&read_self(cache+24,&counts[1],4);
}
static void draw_other(void *ctx,unsigned char *state,float x,float y,const unsigned char rgb[3],const char *text) {
    float outside[8]={x,y-13.2f,x+13.2f,y,x,y+13.2f,x-13.2f,y};
    color(state,.25f,.25f,.25f,marker_alpha);polygon(ctx,outside,4);
    float inside[8]={x,y-11.6f,x+11.6f,y,x,y+11.6f,x-11.6f,y};
    color(state,rgb[0]/255.0f,rgb[1]/255.0f,rgb[2]/255.0f,marker_alpha);polygon(ctx,inside,4);
    size_t n=strlen(text);if(!n||n>6)return;
    float u=initials_unit(n),left=x-(6*(float)n-1)*u/2,top=y-3.5f*u;
    float ink=initials_light(rgb)?1.0f:0.0f;color(state,ink,ink,ink,marker_alpha);
    text_batch.count=0;
    for(size_t k=0;k<n;k++)if(text[k]>='A'&&text[k]<='Z')for(int row=0;row<7;row++)for(int col=0;col<5;col++) {
        if(!(initial_font[text[k]-'A'][row]&(1<<(4-col))))continue;
        float a=left+(6*(float)k+col)*u,b=top+row*u;
        if(!batch_rect(&text_batch,a,b,u,u)){submit_batch(ctx,&text_batch);batch_rect(&text_batch,a,b,u,u);}
    }
    submit_batch(ctx,&text_batch);
}
#include "panel.inc"

#include "pirate_style.h"
#include "owned56_render.inc"
static void draw_skull45(void *ctx,unsigned char *state,float x,float y){IgmPathBatch batch;batch.count=0;float u=26.4f*PIRATE45_RATIO/9;for(int row=0;row<9;row++)for(int col=0;col<9;col++)if(pirate45_rows[row]&(1u<<(8-col)))batch_rect(&batch,x-3.3f+col*u,y-3.3f+row*u,u,u);color(state,.94f,.94f,.94f,marker_alpha);submit_batch(ctx,&batch);}

static int draw_overlay(void *map,void *ctx) {
    if(drawing||!InterlockedCompareExchange(&overlay_enabled,0,0))return 0;
    if(GetAsyncKeyState(VK_F10)&0x8000) {InterlockedExchange(&overlay_enabled,0);log_line("RENDER DISABLED by F10 until restart");return 0;}
    uint64_t stateptr=0;Address address;unsigned char vr=0;float viewport[2]={0};
    if(!read_self(image_base+native.singleton,&stateptr,8)||!stateptr ||
       !read_self(stateptr+0xc1a0,&address,24)||!read_self(stateptr+native.vr_offset,&vr,1)||vr)return 0;
    get_viewport((void*)(uintptr_t)(stateptr+native.viewport_offset),viewport);
    if(!isfinite(viewport[0])||!isfinite(viewport[1])||viewport[0]<256||viewport[0]>16384||viewport[1]<256||viewport[1]>16384)return 0;
    unsigned char *state=NULL,saved[264];if(!valid_context(ctx,&state))return 0;
    Seen systems[4096];size_t imperial=0,other=0;size_t n=visible_systems(systems,(unsigned)address.galaxy,&imperial,&other);
    uint64_t uas54[4096];for(size_t i=0;i<n;i++)uas54[i]=systems[i].ua;
    pos54_prepare(&positions54,read_self,(uint64_t)(uintptr_t)map,uas54,n,(unsigned)address.galaxy,fade_end);
    pos54_prefetch(&positions54,read_self,(uint64_t)(uintptr_t)map,uas54,n,(unsigned)address.galaxy,fade_end,GetTickCount64(),voxel_request54);
    float scale=icon_scale(viewport[1],scale_percent);int drew_imperial=0,drew_other=0;
    drawing=1;int drew=0,missing55=0,anchors55=0,projected55=0,faded55=0;memcpy(saved,state,264);icons52_begin(ctx);
    int report_positions=GetTickCount64()-last_map_report>10000;
    /* Restore all drawing state after adding our paths. Never replace native records. */
    memset(state+0xb4,0,24);set_float(state+0xb4,1);set_float(state+0xc0,1);
    memset(state+0xcc,0,32);set_float(state+0xe4,-1);set_float(state+0xe8,-1);set_float(state+0xb0,1);
    for(size_t i=0;i<n && drew<256;i++) {
        float x,y;int mode=0;if(!locate_system((uint64_t)(uintptr_t)map,systems[i].ua,viewport,&x,&y,&mode)){missing55++;continue;}if(mode==1)anchors55++;else projected55++;
        float distance=-1;int measured=mode==1?anchor55_distance((uint64_t)(uintptr_t)map,systems[i].ua,&distance):marker_distance((uint64_t)(uintptr_t)map,systems[i].ua,&distance);
        marker_alpha=.9f*(measured?marker_fade(distance,fade_start,fade_end):1.0f);
        if(marker_alpha<=.001f){faded55++;if(report_positions)log_line("FADE_HIDDEN ua=%016llx distance=%.1f",(unsigned long long)systems[i].ua,distance);continue;}
        /* Offset to the right in screen space, constant across zoom; no edge pinning. */
        float center[2];icon_center(x,y,viewport,scale,center);
        float transform[6]={scale,0,0,scale,center[0],center[1]};memcpy(state+0xb4,transform,24);
        int category=alliance_category(systems[i].id);int alliance_visible=panel_category_visible(&panel,category);int pirate_visible=atomic_load(&pirates45_enabled)&&ex45_record(systems[i].ua,0).pirate==1;if(!alliance_visible&&!pirate_visible)continue;unsigned char rgb[3]={255,255,255};uint32_t slot=UINT32_MAX;
        if(alliance_visible&&category==1){float identity[6]={1,0,0,1,0,0};memcpy(state+0xb4,identity,24);draw_owned56(ctx,state,x,y,viewport[1]/1080.f);memcpy(state+0xb4,transform,24);}
        if(alliance_visible&&own_id(systems[i].id)){draw_cross(ctx,state,0,0);drew_imperial++;}
        else if(alliance_visible&&category){if(!get_alliance_color(systems[i].id,rgb,&slot))continue;char text[7];get_initials(systems[i].id,text);if(!icons52_draw(ctx,state,systems[i].id,-13.2f,-13.2f,26.4f,transform,marker_alpha))draw_other(ctx,state,0,0,rgb,text);drew_other++;}
        if(pirate_visible)draw_skull45(ctx,state,alliance_visible&&category?21:0,0);
        ManagedSystem meta=metadata(systems[i].ua);if(alliance_visible&&((meta.flags&1)||meta.planets))draw_favourite(ctx,state,category?(pirate_visible?32:21):pirate_visible?11:0,0);
        drew++;
        if(report_positions)log_line("ANCHOR_CHECK ua=%016llx star_xy=%.1f,%.1f icon_xy=%.1f,%.1f delta=%.2f,%.2f color=#%02x%02x%02x alpha=%.3f distance=%.1f",(unsigned long long)systems[i].ua,x,y,center[0],center[1],center[0]-x,center[1]-y,rgb[0],rgb[1],rgb[2],marker_alpha,distance);
        size_t k;for(k=0;k<reported_count;k++)if(reported_systems[k]==systems[i].ua)break;
        if(k==reported_count && reported_count<64) {
            reported_systems[reported_count++]=systems[i].ua;
            log_line("DRAW_SUBMITTED ua=%016llx category=%s id=%u,%u,%u,%u source=%s star_xy=%.1f,%.1f icon_xy=%.1f,%.1f scale=%.2f color=#%02x%02x%02x slot=%u (confirm visually)",(unsigned long long)systems[i].ua,category==1?"player_alliance":"other_alliance",systems[i].id[0],systems[i].id[1],systems[i].id[2],systems[i].id[3],mode==1?"native_anchor":"catalogue_position_projection",x,y,center[0],center[1],scale,rgb[0],rgb[1],rgb[2],slot);
        }
    }
    memcpy(state,saved,264);drawing=0;
    ULONGLONG now=GetTickCount64();if(now-last_map_report>10000) {
        log_line("VISIBILITY55 candidates=%zu native_anchors=%d projected=%d missing=%d faded=%d filter=%d",n,anchors55,projected55,missing55,faded55,panel.visibility);
        last_map_report=now;log_line("MAP_FRAME catalogue_own=%zu catalogue_other=%zu submitted_own=%d submitted_other=%d viewport=%.0fx%.0f scale=%.2f positions54=%zu requests54=%u cache_hits54=%u",imperial,other,drew_imperial,drew_other,viewport[0],viewport[1],scale,positions54.count,positions54.requests,positions54.hits);
    }
    return drew;
}
static void hook_end(void *ctx) {
    catalogue44_lifecycle();
    if(active_map&&!submitted_in_scope&&!drawing) {
        uint64_t normal=0;read_self(image_base+native.nvg,&normal,8);
        if(normal==(uint64_t)(uintptr_t)ctx){draw_overlay(active_map,ctx);submitted_in_scope=1;}
    }
    uint64_t normal=0;read_self(image_base+native.nvg,&normal,8);
    if(!drawing&&normal==(uint64_t)(uintptr_t)ctx){hotkey45_poll();if(catalogue42_active())catalogue42_draw(ctx);else {catalogue43_release();draw_panel(ctx);}feature45_toast_draw(ctx);}
    original_end(ctx);
}
static void hook_map(void *map,void *input,unsigned char flag) {
    void *previous=active_map;int previous_submitted=submitted_in_scope;
    active_map=map;submitted_in_scope=0;
    original_map(map,input,flag);
    if(!submitted_in_scope) {
        uint64_t ctx=0;if(read_self(image_base+native.nvg,&ctx,8)&&ctx)draw_overlay(map,(void*)(uintptr_t)ctx);
    }
    active_map=previous;submitted_in_scope=previous_submitted;
}
static void install_map_hooks(uint64_t base) {
    image_base=base;
    /* Resolved structural guards are rechecked on live code before hooking. */
    if(!verify_active_bindings(base)){log_line("RENDER REFUSED: code guards differ; detection remains available");return;}
    if(native.rva[B_TEXT]&&native.rva[B_FONT]){
        unsigned char text_guard[900],font_guard[256];
        const BytePattern *t=&native_patterns[B_TEXT].variants[native.profile],*f=&native_patterns[B_FONT].variants[native.profile];
        if(read_self(base+native.rva[B_TEXT],text_guard,sizeof(text_guard))&&pattern_at(text_guard,sizeof(text_guard),t)&&
           read_self(base+native.rva[B_FONT],font_guard,sizeof(font_guard))&&pattern_at(font_guard,sizeof(font_guard),f))native_text=(NativeTextFn)(uintptr_t)(base+native.rva[B_TEXT]);
    }
    if(native.voxel_request){unsigned char bytes54[sizeof(pos54_bytes)];if(read_self(base+native.voxel_request,bytes54,sizeof(bytes54))&&pattern_at(bytes54,sizeof(bytes54),&position54_pattern.variants[0]))voxel_request54=(Pos54Request)(uintptr_t)(base+native.voxel_request);}
    log_line("POSITIONS54 async voxel cache=%s rva=%x; own position cache survives history/voxel eviction; 150ms budget",voxel_request54?"READY":"UNAVAILABLE",native.voxel_request);
    log_line("PANEL native text %s",native_text?"READY":"UNAVAILABLE; no vector text fallback");
    append_commands=(AppendFn)(uintptr_t)(base+native.rva[B_APPEND]);fill_path=(FillFn)(uintptr_t)(base+native.rva[B_FILL]);
    get_viewport=(ViewportFn)(uintptr_t)(base+native.rva[B_VIEWPORT]);project_points=(ProjectFn)(uintptr_t)(base+native.rva[B_PROJECT]);
    MH_STATUS status=MH_Initialize();if(status!=MH_OK){log_line("RENDER REFUSED: MinHook init %d",status);return;}
    void *map=(void*)(uintptr_t)(base+native.rva[B_MAP]),*end=(void*)(uintptr_t)(base+native.rva[B_END]);
    status=MH_CreateHook(map,(void*)hook_map,(void**)&original_map);
    if(status==MH_OK)status=MH_CreateHook(end,(void*)hook_end,(void**)&original_end);
    if(status!=MH_OK){log_line("RENDER REFUSED: hook creation %d",status);MH_RemoveHook(map);MH_RemoveHook(end);return;}
    MH_QueueEnableHook(map);MH_QueueEnableHook(end);status=MH_ApplyQueued();
    if(status!=MH_OK){InterlockedExchange(&overlay_enabled,0);log_line("RENDER REFUSED: enabling hooks %d",status);return;}
    log_line("RENDER READY: native map and EndFrame hooks enabled; awaiting map; no mission changes");
}
