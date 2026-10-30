#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <wchar.h>
#include <sys/stat.h>
#include <dirent.h>
#include <fnmatch.h>
typedef unsigned DWORD;typedef void* HANDLE;
#define TRUE 1
#define ERROR_FILE_EXISTS 80
#define ERROR_ALREADY_EXISTS 183
#define ERROR_FILE_NOT_FOUND 2
#define ERROR_PATH_NOT_FOUND 3
#define FILE_ATTRIBUTE_DIRECTORY 16
#define FILE_ATTRIBUTE_REPARSE_POINT 1024
#define INVALID_HANDLE_VALUE ((HANDLE)(intptr_t)-1)
typedef struct{unsigned dwFileAttributes;wchar_t cFileName[256];} WIN32_FIND_DATAW;
static DWORD err;static DWORD GetLastError(void){return err;}
static void path(const wchar_t*w,char*p){wcstombs(p,w,32768);for(char*s=p;*s;s++)if(*s=='\\')*s='/';}
static int CreateDirectoryW(const wchar_t*w,void*x){(void)x;char p[32768];path(w,p);if(!mkdir(p,0700))return 1;err=ERROR_ALREADY_EXISTS;return 0;}
static int CopyFileW(const wchar_t*a,const wchar_t*b,int exclusive){char x[32768],y[32768];path(a,x);path(b,y);FILE*f=fopen(x,"rb");if(!f){err=2;return 0;}FILE*g=fopen(y,exclusive?"wbx":"wb");if(!g){fclose(f);err=80;return 0;}int c;while((c=fgetc(f))!=EOF)fputc(c,g);fclose(f);fclose(g);return 1;}
typedef struct{DIR*d;char pat[256];} Finder;
static int FindNextFileW(HANDLE h,WIN32_FIND_DATAW*v){Finder*f=h;struct dirent*e;while((e=readdir(f->d)))if(!fnmatch(f->pat,e->d_name,0)){v->dwFileAttributes=0;mbstowcs(v->cFileName,e->d_name,256);return 1;}return 0;}
static HANDLE FindFirstFileW(const wchar_t*w,WIN32_FIND_DATAW*v){char p[32768];path(w,p);char*s=strrchr(p,'/');assert(s);*s++=0;Finder*f=calloc(1,sizeof(*f));strcpy(f->pat,s);f->d=opendir(p);if(!f->d){free(f);return INVALID_HANDLE_VALUE;}if(!FindNextFileW(f,v)){closedir(f->d);free(f);return INVALID_HANDLE_VALUE;}return f;}
static void FindClose(HANDLE h){Finder*f=h;closedir(f->d);free(f);}
static wchar_t base52[32768];
static void make_path(wchar_t*p,const wchar_t*n){swprintf(p,32768,L"%ls/%ls",base52,n);}
static void log_line(const char*s,...){(void)s;}
static FILE*_wfopen(const wchar_t*w,const wchar_t*m){char p[32768],mode[32];path(w,p);wcstombs(mode,m,32);return fopen(p,mode);}
#include "icon52_data.inc"
int main(int argc,char**argv){assert(argc==2);char tmp[]="/tmp/igm52-files-XXXXXX";assert(mkdtemp(tmp));mbstowcs(base52,tmp,32768);char folder[32768];snprintf(folder,sizeof(folder),"%s/iconos_alianzas",tmp);assert(!mkdir(folder,0700));char file[32768];snprintf(file,sizeof(file),"%s/1_2_3_4.bmp",folder);FILE*a=fopen(argv[1],"rb"),*b=fopen(file,"wb");assert(a&&b);int c;while((c=fgetc(a))!=EOF)fputc(c,b);fclose(a);fclose(b);snprintf(file,sizeof(file),"%s/1_2_3_4.igmicon",folder);b=fopen(file,"wb");assert(b);fputs("IGMICON2 1 96\n0 0 96 ff0000ff\n",b);fclose(b);snprintf(file,sizeof(file),"%s/5_6_7_8.igmicon",folder);b=fopen(file,"wb");assert(b);fputs("IGMICON1 1\n0 0 24 00ff00ff\n",b);fclose(b);snprintf(file,sizeof(file),"%s/8_8_8_8.bmp",folder);b=fopen(file,"wb");assert(b);fputs("invalid",b);fclose(b);snprintf(file,sizeof(file),"%s/275889132_1121173938_1505499062_4230148680.igmicon",folder);b=fopen(file,"wb");assert(b);fputs("IGMICON1 1\n0 0 24 00ff00ff\n",b);fclose(b);icons52_load();assert(icons52_count==2);assert(icon52_find(imperial53_id)<0);uint32_t id[4]={1,2,3,4};int i=icon52_find(id);assert(i>=0&&icons52[i].pixels[0]==17&&icons52[i].pixels[1]==51&&icons52[i].pixels[2]==119&&icons52[i].pixels[3]==255);icons52_load();assert(icons52_count==2);puts("PASS production icon file loader: real Companion BMP, exact ID lookup, BMP overrides old icon, legacy supported, invalid rejected, no duplicates; filesystem WinAPI stubs");}
