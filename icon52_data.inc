#include "icon52_format.h"
#include "core.h"
#define ICON52_LIMIT 128
static struct {uint32_t id[4];unsigned char pixels[ICON52_BYTES];} icons52[ICON52_LIMIT];
static _Atomic size_t icons52_count;
static int icon52_find(const uint32_t id[4]){for(size_t i=0;i<icons52_count;i++)if(!memcmp(id,icons52[i].id,16))return (int)i;return -1;}
/* Immutable after startup, before installing hooks. No filesystem work during rendering. */
static void icons52_load_pass(int bmp){wchar_t folder[32768],search[32768],path[32768];make_path(folder,L"iconos_alianzas");swprintf(search,32768,L"%ls\\%ls",folder,bmp?L"*.bmp":L"*.igmicon");WIN32_FIND_DATAW d;HANDLE h=FindFirstFileW(search,&d);if(h==INVALID_HANDLE_VALUE)return;do{if(d.dwFileAttributes&(FILE_ATTRIBUTE_DIRECTORY|FILE_ATTRIBUTE_REPARSE_POINT))continue;uint32_t id[4];wchar_t tail[32];int n=0;if(swscanf(d.cFileName,L"%u_%u_%u_%u%31ls%n",&id[0],&id[1],&id[2],&id[3],tail,&n)!=5||d.cFileName[n]||wcscmp(tail,bmp?L".bmp":L".igmicon")||own_id(id)||icon52_find(id)>=0||icons52_count>=ICON52_LIMIT)continue;swprintf(path,32768,L"%ls\\%ls",folder,d.cFileName);FILE*f=_wfopen(path,L"rb");if(!f)continue;int ok=bmp?icon52_bmp(f,icons52[icons52_count].pixels):icon52_legacy(f,icons52[icons52_count].pixels);fclose(f);if(ok){memcpy(icons52[icons52_count++].id,id,16);log_line("ICON52 LOADED id=%u_%u_%u_%u format=%s",id[0],id[1],id[2],id[3],bmp?"BGRA180":"legacy");}else log_line("ICON52 REFUSED invalid file id=%u_%u_%u_%u",id[0],id[1],id[2],id[3]);}while(FindNextFileW(h,&d));FindClose(h);}
static void icons52_load(void){icons52_load_pass(1);icons52_load_pass(0);log_line("ICON52 loaded=%zu (custom overrides default; Borgona vectors retained)",icons52_count);}
static void icons52_begin(void*ctx);
static int icons52_draw(void*ctx,unsigned char*state,const uint32_t id[4],float x,float y,float size,const float transform[6],float alpha);
