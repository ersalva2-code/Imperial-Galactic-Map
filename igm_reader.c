/* IGM v0.43: adaptive local observation and experimental map/panel overlay. */
#define WIN32_LEAN_AND_MEAN
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0601
#endif
#include <windows.h>
#include <wincrypt.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdatomic.h>
#include <wchar.h>
#include "core.h"
#include "map_catalogue.h"

static HMODULE self_module, real_xinput;
static INIT_ONCE input_once=INIT_ONCE_STATIC_INIT;
static wchar_t output_dir[32768], exe_path[32768];
static HANDLE log_file=INVALID_HANDLE_VALUE;
static SRWLOCK log_lock=SRWLOCK_INIT, known_lock=SRWLOCK_INIT;


/* Only the real Windows System32 library is loaded, never a similarly named local file. */
static BOOL CALLBACK input_init(PINIT_ONCE once, PVOID param, PVOID *context) {
    (void)once; (void)param; (void)context;
    wchar_t path[MAX_PATH]; UINT n=GetSystemDirectoryW(path,MAX_PATH);
    if(n && n<MAX_PATH-20) { wcscat(path,L"\\xinput9_1_0.dll"); real_xinput=LoadLibraryW(path); }
    return TRUE;
}
static FARPROC input_proc(const char *name) {
    InitOnceExecuteOnce(&input_once,input_init,NULL,NULL);
    return real_xinput?GetProcAddress(real_xinput,name):NULL;
}
__declspec(dllexport) DWORD WINAPI XInputGetState(DWORD a, void *b) {
    typedef DWORD (WINAPI *Fn)(DWORD,void*); Fn f=(Fn)input_proc("XInputGetState"); return f?f(a,b):ERROR_DEVICE_NOT_CONNECTED;
}
__declspec(dllexport) DWORD WINAPI XInputSetState(DWORD a, void *b) {
    typedef DWORD (WINAPI *Fn)(DWORD,void*); Fn f=(Fn)input_proc("XInputSetState"); return f?f(a,b):ERROR_DEVICE_NOT_CONNECTED;
}
__declspec(dllexport) void WINAPI XInputEnable(BOOL a) {
    typedef void (WINAPI *Fn)(BOOL); Fn f=(Fn)input_proc("XInputEnable"); if(f)f(a);
}
#define FORWARD3(Name) __declspec(dllexport) DWORD WINAPI Name(DWORD a,DWORD b,void*c) { \
    typedef DWORD (WINAPI *Fn)(DWORD,DWORD,void*); Fn f=(Fn)input_proc(#Name); return f?f(a,b,c):ERROR_DEVICE_NOT_CONNECTED; }
FORWARD3(XInputGetCapabilities)
FORWARD3(XInputGetBatteryInformation)
FORWARD3(XInputGetKeystroke)
__declspec(dllexport) DWORD WINAPI XInputGetDSoundAudioDeviceGuids(DWORD a,void*b,void*c) {
    typedef DWORD (WINAPI *Fn)(DWORD,void*,void*); Fn f=(Fn)input_proc("XInputGetDSoundAudioDeviceGuids"); return f?f(a,b,c):ERROR_DEVICE_NOT_CONNECTED;
}

static int read_self(uint64_t address, void *dst, SIZE_T size) {
    SIZE_T n=0;
    return address>=0x10000 && address<=UINT64_MAX-size &&
        ReadProcessMemory(GetCurrentProcess(),(const void*)(uintptr_t)address,dst,size,&n) && n==size;
}
static int write_handle(HANDLE h,const char *s) {
    DWORD written=0; size_t len=strlen(s);
    return h!=INVALID_HANDLE_VALUE && WriteFile(h,s,(DWORD)len,&written,NULL) && written==len;
}
static void log_line(const char *fmt,...) {
    char line[2048]; va_list args; va_start(args,fmt); vsnprintf(line,sizeof(line),fmt,args); va_end(args);
    AcquireSRWLockExclusive(&log_lock);
    write_handle(log_file,line); write_handle(log_file,"\r\n");
    ReleaseSRWLockExclusive(&log_lock);
}
static int make_path(wchar_t *out, const wchar_t *name) {
    return swprintf(out,32768,L"%ls\\%ls",output_dir,name)>0;
}
static int sha256_file(const wchar_t *path,char out[65]) {
    HCRYPTPROV provider=0; HCRYPTHASH hash=0; int ok=0; unsigned char buf[65536],digest[32];
    HANDLE h=CreateFileW(path,GENERIC_READ,FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
    if(h==INVALID_HANDLE_VALUE)return 0;
    if(!CryptAcquireContextW(&provider,NULL,NULL,PROV_RSA_AES,CRYPT_VERIFYCONTEXT))goto done;
    if(!CryptCreateHash(provider,CALG_SHA_256,0,0,&hash))goto done;
    for(;;) { DWORD n=0; if(!ReadFile(h,buf,sizeof(buf),&n,NULL))goto done; if(!n)break; if(!CryptHashData(hash,buf,n,0))goto done; }
    { DWORD n=32; if(!CryptGetHashParam(hash,HP_HASHVAL,digest,&n,0)||n!=32)goto done; }
    for(int i=0;i<32;i++)sprintf(out+i*2,"%02x",digest[i]);
    ok=1;
done:
    if(hash)CryptDestroyHash(hash);
    if(provider)CryptReleaseContext(provider,0);
    CloseHandle(h); return ok;
}
/* The complete relevant entry/map headers are copied and rechecked. No native calls,
   engine iterators, pointer writes, detours, or server lookups are performed. */
#include "compat_runtime.inc"
typedef struct { uint64_t ua; uint32_t id[4]; } CacheEntry;
typedef IgmRecord Seen;
static Seen known[4096]; static size_t known_count;
#include "known_state53.inc"
#include "initials.h"
typedef struct {uint32_t id[4];char text[7];char name[128];} InitialLabel;
static InitialLabel labels[4096];static size_t label_count;
static SRWLOCK labels_lock=SRWLOCK_INIT;
static void remember_name(const uint32_t id[4],const char *name) {
    char text[7];make_initials(name,text);if(!text[0])return;
    AcquireSRWLockExclusive(&labels_lock);
    size_t i;for(i=0;i<label_count;i++)if(!memcmp(labels[i].id,id,16))break;
    if(i<label_count||label_count<4096){if(i==label_count){memcpy(labels[i].id,id,16);label_count++;}memcpy(labels[i].text,text,strlen(text)+1);snprintf(labels[i].name,128,"%s",name);}
    ReleaseSRWLockExclusive(&labels_lock);
}
static void get_initials(const uint32_t id[4],char out[7]) {
    if(own_id(id)){snprintf(out,7,"IESP");return;}
    out[0]=0;AcquireSRWLockShared(&labels_lock);
    for(size_t i=0;i<label_count;i++)if(!memcmp(labels[i].id,id,16)){memcpy(out,labels[i].text,7);break;}
    ReleaseSRWLockShared(&labels_lock);
}
#include "layout51.inc"
#include "menu_data.inc"
#include "panel_model.h"
static IgmPanel panel;
#include "exploration45_data.inc"
#include "photo51_data.inc"
#include "icon52_data.inc"


static void load_journal(void) {
    wchar_t path[32768];make_path(path,L"catalogo_observado.jsonl");
    FILE *f=_wfopen(path,L"rb"); if(!f)return;
    char line[2048];
    while(fgets(line,sizeof(line),f)) {
        unsigned long long ua; unsigned a,b,c,d;
        if(sscanf(line,"{\"system_ua\":\"%16llx\",\"alliance_id_words\":[%u,%u,%u,%u]",&ua,&a,&b,&c,&d)==5) {
            uint32_t id[4]={a,b,c,d};if(nonzero_id(id))changed(ua,id);
            const char *v=strstr(line,"\"cached_alliance_name\":\"");
            if(v){v+=24;char name[128];size_t k=0;while(*v && *v!='"' && k<127){if(*v=='\\' && v[1])v++;name[k++]=*v++;}name[k]=0;remember_name(id,name);}
        }
    }
    fclose(f);
}
static void json_escape(const char *in,char *out,size_t cap) {
    size_t j=0;
    for(size_t i=0;in[i] && j+7<cap;i++) {
        unsigned char c=(unsigned char)in[i];
        if(c=='"'||c=='\\') {out[j++]='\\';out[j++]=(char)c;}
        else if(c<32) {j+=(size_t)snprintf(out+j,cap-j,"\\u%04x",c);}
        else out[j++]=(char)c;
    }
    out[j]=0;
}
static int lookup(uint64_t state,uint64_t ua,uint32_t id[4],char name[128]) {
    uint64_t heads[2],after[2]; size_t count=0; unsigned char meta_before[16],meta_after[16];
    const uint64_t root=state+native.alliance_offset;
    if(!read_self(root+0x40,heads,16)||!valid_span(heads[0],heads[1],24,32768,&count))return 0;
    CacheEntry entry,again; int found=0;
    for(size_t i=0;i<count;i++) {
        uint64_t at=heads[0]+i*24;
        if(!read_self(at,&entry,sizeof(entry)))return 0;
        if((entry.ua&0x000fffffffffffffULL)!=ua)continue;
        if(!read_self(at,&again,sizeof(again))||memcmp(&entry,&again,sizeof(entry)))return 0;
        if(!nonzero_id(entry.id))continue;
        memcpy(id,entry.id,16);found=1;break;
    }
    if(!read_self(root+0x40,after,16)||memcmp(heads,after,16)||!found)return 0;
    name[0]=0;
    /* cGcAtlas manager's compact local alliance records, not serialised cGcAtlasAlliance. */
    if(read_self(root+0x10,meta_before,16)) {
        uint32_t n;uint64_t records;memcpy(&n,meta_before+4,4);memcpy(&records,meta_before+8,8);
        if(n<=4096 && records>=0x10000) for(uint32_t i=0;i<n;i++) {
            unsigned char record[168],twice[168];uint64_t at=records+(uint64_t)i*168;
            if(!read_self(at,record,168))break;
            if(memcmp(record,id,16))continue;
            if(read_self(at,twice,168)&&!memcmp(record,twice,168)) {
                const char *text=(char*)record+17; const char *end=memchr(text,0,127);
                if(end) {size_t len=(size_t)(end-text);memcpy(name,text,len);name[len]=0;}
            }
            break;
        }
        if(!read_self(root+0x10,meta_after,16)||memcmp(meta_before,meta_after,16))name[0]=0;
    }
    return 1;
}
static int record_observation(uint64_t ua,const uint32_t id[4],const char *name) {
    char escaped[800],line[1800];json_escape(name,escaped,sizeof(escaped));SYSTEMTIME t;GetSystemTime(&t);
    snprintf(line,sizeof(line),"{\"system_ua\":\"%016llx\",\"alliance_id_words\":[%u,%u,%u,%u],\"classification\":\"%s\",\"cached_alliance_name\":\"%s\",\"source\":\"live_local_alliance_cache\",\"utc\":\"%04u-%02u-%02uT%02u:%02u:%02uZ\",\"freshness\":\"cached_not_server_verified\"}\r\n",
        (unsigned long long)ua,id[0],id[1],id[2],id[3],own_id(id)?"imperio_espanol":"otra_alianza",escaped,t.wYear,t.wMonth,t.wDay,t.wHour,t.wMinute,t.wSecond);
    wchar_t path[32768];make_path(path,L"catalogo_observado.jsonl");
    HANDLE h=CreateFileW(path,FILE_APPEND_DATA,FILE_SHARE_READ,NULL,OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
    if(h==INVALID_HANDLE_VALUE){log_line("ERROR: cannot append mod journal");return 0;}
    int ok=write_handle(h,line);CloseHandle(h);
    if(!ok){log_line("ERROR: incomplete journal write");return 0;}
    log_line("OBSERVED system=%016llx category=%s name=%s",(unsigned long long)ua,own_id(id)?"imperio_espanol":"otra_alianza",name);
    return 1;
}
static int catalogue42_offer(void);
static int catalogue42_active(void);
static int catalogue42_draw(void *ctx);
static void catalogue42_signal(void);
static void catalogue42_reset(void);
static int catalogue42_design=1;
static void catalogue43_release(void);
static void catalogue44_lifecycle(void);
static void hotkey45_poll(void);
static void feature45_toast_draw(void *ctx);
#include "map_render.inc"
#include "auto58_icons.inc"
#include "identity58_live.inc"
#include "native_menu.inc"
#include "catalogue42.inc"

static DWORD WINAPI worker(void *unused) {
    (void)unused;
    if(!GetModuleFileNameW(self_module,output_dir,32768)||!GetModuleFileNameW(NULL,exe_path,32768))return 0;
    wchar_t *sep=wcsrchr(output_dir,L'\\');if(!sep)return 0;*sep=0;
    if(wcslen(output_dir)+25>=32768)return 0;
    if(!layout51_init(output_dir))return 0;
    CreateDirectoryW(output_dir,NULL);wchar_t path[32768];make_path(path,L"igm_diagnostico.log");
    log_file=CreateFileW(path,FILE_APPEND_DATA,FILE_SHARE_READ,NULL,OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
    log_line("IGM STEAM v059 NATIVE ANCHOR RECOVERY + NATIVE PROJECTION STATUS; NATIVE TEXTURE THUMBNAILS + IGM LAYOUT; v045 vector renderer unchanged; external one-shot GPU capture; bounded native image cache; up to twelve temporary hooks; native menu and map; no missions, no save writes, no network");
    char hash[65];
    if(sha256_file(exe_path,hash))log_line("EXE SHA256=%s (diagnostic; compatibility decided by structural checks)",hash);
    else log_line("EXE hash unavailable; attempting loaded-code structural validation");
    uint64_t base=(uint64_t)(uintptr_t)GetModuleHandleW(NULL);
    if(!probe_compatibility(base))goto done;
    identity58_init(base);
    log_line("LAYOUT51 migrated=%u failed=%u",layout51_copied,layout51_failed);wchar_t photo_ini[32768];make_path(photo_ini,L"ajustes.ini");photos51_enabled=GetPrivateProfileIntW(L"Miniaturas",L"Activas",1,photo_ini)!=0;
    migrate_journal();load_management();ex45_load();load_palette();
    for(size_t i=0;i<IMPERIAL53_COUNT;i++)changed(imperial53_systems[i].ua,imperial53_id);
    log_line("IMPERIAL53 CORE systems=%zu; protected built-in catalogue; reserved Borgona emblem",IMPERIAL53_COUNT);
    load_journal();sync_alliance_colors();
    icons52_load();auto58_sync();install_map_hooks(base);install_native_menu(base);feature45_install(base);
    log_line("READY: observing current system only; journal loaded=%zu; open system alliance screen if cache is unavailable",known_count);
    uint64_t previous_ua=UINT64_MAX,last_reported=UINT64_MAX;uint32_t previous_id[4]={0};unsigned stable=0,unknown_ticks=0,menu_trace_ticks=0;ULONGLONG observe45_last=0;
    for(;;) {
        Sleep(25);feature45_process();photos51_worker();ULONGLONG now45=GetTickCount64();if(now45-observe45_last<1000)continue;observe45_last=now45;if(native_menu_enabled&&++menu_trace_ticks<=180&&menu_trace_ticks%20==0)log_line("MENU41 STATUS options=%u legacy_root_buttons=%u finalize=%u catalogue_labels=%u root_controller=%u modern_buttons=%u entry_labels=%u",atomic_load(&menu_options_hits),atomic_load(&menu_root_hits),atomic_load(&menu_finalize_hits),atomic_load(&menu_translate_hits),atomic_load(&root39_calls),atomic_load(&root39_last_buttons),atomic_load(&root39_label_calls));uint64_t state=0,state_after=0,ua=0;Address a,b;uint32_t id[4];char name[128];
        if(!read_self(base+native.singleton,&state,8)||!state || !read_self(state+0xc1a0,&a,sizeof(a)) || !pack_address(&a,&ua)){identity58_reset();continue;}
        identity58_poll(state);sync_alliance_colors();auto58_sync();
        if(!read_self(state+0xc1a0,&b,sizeof(b))||memcmp(&a,&b,sizeof(a)))continue;
        if(ua!=last_reported){log_line("CURRENT system=%016llx galaxy=%d voxel=%d,%d,%d solar=%d",(unsigned long long)ua,a.galaxy,a.x,a.y,a.z,a.solar);last_reported=ua;unknown_ticks=0;}
        feature45_observe(state,ua);
        if(!lookup(state,ua,id,name)) {stable=0;if(++unknown_ticks==5)log_line("UNKNOWN: no usable cached alliance for current system; not classified as unclaimed or pirate");continue;}
        if(!read_self(base+native.singleton,&state_after,8)||state_after!=state||!read_self(state+0xc1a0,&b,sizeof(b))||memcmp(&a,&b,sizeof(a)))continue;
        if(ua==previous_ua&&!memcmp(id,previous_id,16))stable++;else{previous_ua=ua;memcpy(previous_id,id,16);stable=1;}
        if(stable>=2 && needs_record(ua,id) && record_observation(ua,id,name))changed(ua,id);
        remember_name(id,name);
        sync_alliance_colors();
    }
done:
    if(log_file!=INVALID_HANDLE_VALUE){CloseHandle(log_file);log_file=INVALID_HANDLE_VALUE;}return 0;
}
BOOL WINAPI DllMain(HINSTANCE module,DWORD reason,LPVOID reserved) {
    (void)reserved;
    if(reason==DLL_PROCESS_ATTACH) {
        self_module=module;DisableThreadLibraryCalls(module);
        HANDLE t=CreateThread(NULL,0,worker,NULL,0,NULL);if(t)CloseHandle(t);
    }
    /* Deliberately no wait or live unload. Remove only after closing NMS. */
    return TRUE;
}
