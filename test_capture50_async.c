#include <assert.h>
#include <stdint.h>
#include <stdatomic.h>
#include <stdio.h>
#include <wchar.h>
#include <string.h>
#include "core.h"
#include "exploration45.h"
#include "language48.h"
#include "capture50_format.h"
typedef unsigned long DWORD;typedef unsigned long long ULONGLONG;typedef void*HANDLE;typedef void*HWND;
typedef struct {DWORD cb,dwFlags;int wShowWindow;} STARTUPINFOW;
typedef struct {HANDLE hThread,hProcess;DWORD dwProcessId;} PROCESS_INFORMATION;
#define STILL_ACTIVE 259
#define SW_HIDE 0
#define STARTF_USESHOWWINDOW 1
#define CREATE_NO_WINDOW 0x8000000
#define BELOW_NORMAL_PRIORITY_CLASS 0x4000
#define INVALID_FILE_ATTRIBUTES ((DWORD)-1)
#define ERROR_ALREADY_EXISTS 183
#define MOVEFILE_REPLACE_EXISTING 1
#define MOVEFILE_WRITE_THROUGH 8
#define FALSE 0
#define SEEK_END 2
static unsigned char rootdata[0xc200],envdata[0x500];static uint64_t ptr=(uint64_t)(uintptr_t)rootdata,image_base;
static struct {uint64_t singleton;} native;static _Atomic uint64_t env45_pointer;static HANDLE self_module;
static ULONGLONG now=10000;static int foreground=1,menu,missing,enabled=1,start_fail,valid_bmp=1,query_ok=1;static DWORD status=STILL_ACTIVE;static unsigned launches,commits,deletes,kills;static char toast[230];
static int read_self(uint64_t p,void*out,size_t n){if(!p)return 0;memcpy(out,(void*)(uintptr_t)p,n);return 1;}
static int catalogue42_active(void){return menu;}
static HWND GetForegroundWindow(void){return (HWND)(uintptr_t)(foreground?1:2);}
static int GetWindowThreadProcessId(HWND w,DWORD*p){(void)w;*p=55;return 1;}
static DWORD GetCurrentProcessId(void){return 55;}
static int IsIconic(HWND w){(void)w;return 0;}
static ULONGLONG GetTickCount64(void){return now;}
static int GetExitCodeProcess(HANDLE h,DWORD*p){assert(h==(HANDLE)3);*p=status;return query_ok;}
static int TerminateProcess(HANDLE h,unsigned c){assert(h==(HANDLE)3&&c==4);kills++;status=4;return 1;}
static int WaitForSingleObject(HANDLE h,unsigned ms){assert(h==(HANDLE)3&&ms==50);return 0;}
static int CloseHandle(HANDLE h){assert(h==(HANDLE)3||h==(HANDLE)4);return 1;}
static int MoveFileExW(const wchar_t*a,const wchar_t*b,DWORD flags){assert(wcsstr(a,L"pendiente_")&&wcsstr(b,L"_p02.bmp")&&flags==9);commits++;return 1;}
static int DeleteFileW(const wchar_t*p){assert(wcsstr(p,L"pendiente_"));deletes++;return 1;}
static void log_line(const char*s,...){(void)s;}
static void feature45_toast(const char*s){snprintf(toast,sizeof(toast),"%s",s);}
static void make_path(wchar_t*p,const wchar_t*n){swprintf(p,32768,L"C:\\NMS\\Binaries\\IGM\\IGM_datos\\%ls",n);}
static unsigned GetPrivateProfileIntW(const wchar_t*a,const wchar_t*b,unsigned c,const wchar_t*d){(void)a;(void)b;(void)c;(void)d;return enabled;}
static DWORD GetModuleFileNameW(HANDLE h,wchar_t*p,DWORD n){(void)h;swprintf(p,n,L"C:\\NMS\\Binaries\\xinput9_1_0.dll");return (DWORD)wcslen(p);}
static DWORD GetFileAttributesW(const wchar_t*p){assert(wcsstr(p,L"IGM_Captura.exe"));return missing?INVALID_FILE_ATTRIBUTES:0;}
static int CreateDirectoryW(const wchar_t*p,void*s){(void)s;assert(wcsstr(p,L"capturas"));return 1;}
static DWORD GetLastError(void){return 0;}
static int CreateProcessW(const wchar_t*exe,wchar_t*command,void*a,void*b,int inherit,DWORD flags,void*c,void*d,STARTUPINFOW*s,PROCESS_INFORMATION*p){(void)a;(void)b;(void)c;(void)d;assert(wcsstr(exe,L"IGM_Captura.exe")&&wcsstr(command,L"--window 1 --pid 55 --out")&&s->wShowWindow==SW_HIDE&&!inherit&&flags==(CREATE_NO_WINDOW|BELOW_NORMAL_PRIORITY_CLASS));if(start_fail)return 0;p->hProcess=(HANDLE)3;p->hThread=(HANDLE)4;p->dwProcessId=99;launches++;status=STILL_ACTIVE;return 1;}
static FILE*_wfopen(const wchar_t*p,const wchar_t*m){(void)p;(void)m;FILE*f=tmpfile();assert(f);unsigned char h[54],pixels[64*36*4]={0};assert(c50_bmp_header(h,64,36));if(!valid_bmp)h[28]=24;fwrite(h,1,54,f);fwrite(pixels,1,sizeof(pixels),f);rewind(f);return f;}
#define _fseeki64 fseek
#define _ftelli64 ftell
#include "capture50_async.inc"
int main(void){native.singleton=(uint64_t)(uintptr_t)&ptr;atomic_store(&env45_pointer,(uint64_t)(uintptr_t)envdata);Address a={2,68,950,-93,1842,0};memcpy(rootdata+0xc1a0,&a,sizeof(a));int loc=3,nearest=1;memcpy(envdata+0x474,&loc,4);memcpy(envdata+0x2bc,&nearest,4);uint64_t ua;assert(pack_address(&a,&ua));
 assert(capture50_start(ua,2)&&launches==1&&!toast[0]);capture50_poll();assert(capture50_job.process&&commits==0);status=0;capture50_poll();assert(!capture50_job.process&&commits==1&&strstr(toast,"captura"));assert(!capture50_start(ua,2));
 now+=5001;assert(capture50_start(ua,2));a.solar++;memcpy(rootdata+0xc1a0,&a,sizeof(a));capture50_poll();assert(!capture50_job.process&&kills==1&&commits==1);a.solar--;memcpy(rootdata+0xc1a0,&a,sizeof(a));
 now+=5001;assert(capture50_start(ua,2));foreground=0;capture50_poll();assert(!capture50_job.process&&kills==2&&commits==1);foreground=1;
 now+=5001;assert(capture50_start(ua,2));status=0;valid_bmp=0;capture50_poll();assert(!capture50_job.process&&commits==1);valid_bmp=1;
 now+=5001;assert(capture50_start(ua,2));now+=8000;capture50_poll();assert(!capture50_job.process&&kills==3&&commits==1);
 now+=5001;missing=1;assert(!capture50_start(ua,2));missing=0;menu=1;assert(!capture50_start(ua,2));menu=0;enabled=0;assert(!capture50_start(ua,2));enabled=1;start_fail=1;assert(!capture50_start(ua,2));start_fail=0;
 now+=5001;assert(capture50_start(ua,2));status=5;capture50_poll();assert(!capture50_job.process&&commits==1&&deletes==5);
 puts("PASS production asynchronous capture integration: nonblocking launch/poll, cool-down, same-UA+planet validation, focus loss cancellation, timeout termination, corrupt BMP/nonzero exit refusal, atomic commit only on success, menu/config/missing-helper/start-failure fallback. OS/process APIs stubbed; WGC not executed.");}
