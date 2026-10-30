#include "language48.h"
#include "exploration45.h"
static Ex45Store ex45_store,ex45_staged;
static SRWLOCK ex45_lock=SRWLOCK_INIT;
static unsigned ex45_generation,ex45_saved;
static int ex45_writable=1;
static _Atomic int pirates45_enabled=1;
static _Atomic int settings45_dirty;
static void ex45_remember(uint64_t ua,unsigned planet,int pirate,const char *name,unsigned priority){AcquireSRWLockExclusive(&ex45_lock);if(ex45_put(&ex45_store,ua,planet,pirate,name,priority))ex45_generation++;ReleaseSRWLockExclusive(&ex45_lock);}
static Ex45Record ex45_record(uint64_t ua,unsigned planet){Ex45Record r={0};r.pirate=-1;AcquireSRWLockShared(&ex45_lock);int at=ex45_find(&ex45_store,ua,planet);if(at>=0)r=ex45_store.items[at];ReleaseSRWLockShared(&ex45_lock);const Imperial53System *core=imperial53_find(ua);if(!planet&&core)snprintf(r.name,128,"%s",core->name);return r;}
static void ex45_load(void){wchar_t langpath[32768];make_path(langpath,L"ajustes.ini");int lang=(int)GetPrivateProfileIntW(L"Interface",L"Idioma",0,langpath);atomic_store(&language48,lang>=0&&lang<6?lang:0);wchar_t path[32768];make_path(path,L"exploracion.tsv");FILE *f=_wfopen(path,L"rb");if(f){char line[512];while(fgets(line,512,f)){if(line[0]=='#')continue;Ex45Record r;if(!ex45_parse(line,&r)){ex45_writable=0;continue;}ex45_put(&ex45_store,r.ua,r.planet,r.pirate,r.name,r.priority);}fclose(f);}else if(GetFileAttributesW(path)!=INVALID_FILE_ATTRIBUTES)ex45_writable=0;make_path(path,L"ajustes.ini");panel.visibility=(int)GetPrivateProfileIntW(L"Mapa",L"VisibilidadAlianzas",0,path);if(panel.visibility<0||panel.visibility>3)panel.visibility=0;atomic_store(&pirates45_enabled,GetPrivateProfileIntW(L"Mapa",L"MostrarPiratas",1,path)!=0);for(size_t i=0;i<management.count;i++)if(management.entries[i].name[0])ex45_remember(management.entries[i].ua,0,-1,management.entries[i].name,3);log_line("EXPLORE45 loaded=%zu writable=%d pirates_visible=%d",ex45_store.count,ex45_writable,atomic_load(&pirates45_enabled));}
static void ex45_flush(void){
 if(atomic_exchange(&settings45_dirty,0)){wchar_t path[32768];make_path(path,L"ajustes.ini");wchar_t vis[8];swprintf(vis,8,L"%d",panel.visibility);if(!WritePrivateProfileStringW(L"Mapa",L"VisibilidadAlianzas",vis,path))atomic_store(&settings45_dirty,1);wchar_t lang[8];swprintf(lang,8,L"%d",atomic_load(&language48));if(!WritePrivateProfileStringW(L"Interface",L"Idioma",lang,path))atomic_store(&settings45_dirty,1);if(!WritePrivateProfileStringW(L"Mapa",L"MostrarPiratas",atomic_load(&pirates45_enabled)?L"1":L"0",path)){atomic_store(&settings45_dirty,1);log_line("EXPLORE45 settings write failed");}}
 AcquireSRWLockShared(&ex45_lock);unsigned generation=ex45_generation;if(generation==ex45_saved||!ex45_writable){ReleaseSRWLockShared(&ex45_lock);return;}ex45_staged=ex45_store;ReleaseSRWLockShared(&ex45_lock);
 wchar_t path[32768],tmp[32768];make_path(path,L"exploracion.tsv");make_path(tmp,L"exploracion.tsv.tmp");HANDLE h=CreateFileW(tmp,GENERIC_WRITE,0,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);if(h==INVALID_HANDLE_VALUE)return;int ok=write_handle(h,"# IGM45 ua planet pirate(-1=unknown) priority name_utf8_hex\r\n");for(size_t i=0;ok&&i<ex45_staged.count;i++){char line[400];ex45_format(&ex45_staged.items[i],line);ok=write_handle(h,line);}if(ok)ok=FlushFileBuffers(h)!=0;CloseHandle(h);if(ok)ok=MoveFileExW(tmp,path,MOVEFILE_REPLACE_EXISTING|MOVEFILE_WRITE_THROUGH)!=0;if(ok)ex45_saved=generation;else{DeleteFileW(tmp);log_line("EXPLORE45 data write failed; pending retry");}
}
static _Atomic uint64_t system45_pointer;
static SRWLOCK feature45_lock=SRWLOCK_INIT;
static struct {uint64_t ua;unsigned planet;} feature45_queue[16];static unsigned feature45_head,feature45_tail;
static char feature45_toast_text[230];static ULONGLONG feature45_toast_until;
static void feature45_toast(const char *text){AcquireSRWLockExclusive(&feature45_lock);snprintf(feature45_toast_text,sizeof(feature45_toast_text),"%s",text);feature45_toast_until=GetTickCount64()+3000;ReleaseSRWLockExclusive(&feature45_lock);}
static void feature45_enqueue(uint64_t ua,unsigned planet){AcquireSRWLockExclusive(&feature45_lock);if(feature45_tail-feature45_head<16){unsigned at=feature45_tail++%16;feature45_queue[at].ua=ua;feature45_queue[at].planet=planet;}ReleaseSRWLockExclusive(&feature45_lock);}
static int feature45_dequeue(uint64_t *ua,unsigned *planet){AcquireSRWLockExclusive(&feature45_lock);int ok=feature45_head!=feature45_tail;if(ok){unsigned at=feature45_head++%16;*ua=feature45_queue[at].ua;*planet=feature45_queue[at].planet;}ReleaseSRWLockExclusive(&feature45_lock);return ok;}
