/* Called only by the existing worker. No GPU or capture APIs in NMS. */
#include "capture50_format.h"
#include "capture50_policy.h"
static struct {HANDLE process;uint64_t ua;unsigned planet;HWND window;ULONGLONG start;wchar_t pending[32768],final[32768];} capture50_job;
static ULONGLONG capture50_last;
static int capture50_context(uint64_t ua,unsigned planet){
 uint64_t root=0,current=0,env=atomic_load(&env45_pointer);Address a={0},b={0};int location=0,nearest=-1;unsigned p=99;
 return !catalogue42_active()&&env&&read_self(image_base+native.singleton,&root,8)&&root&&read_self(root+0xc1a0,&a,sizeof(a))&&pack_address(&a,&current)&&current==ua&&read_self(env+0x474,&location,4)&&read_self(env+0x2bc,&nearest,4)&&ex45_favourite_target(location,a.planet,nearest,&p)&&p==planet&&read_self(root+0xc1a0,&b,sizeof(b))&&!memcmp(&a,&b,sizeof(a));
}
static int capture50_valid_bmp(const wchar_t*path){FILE*f=_wfopen(path,L"rb");if(!f)return 0;unsigned char h[54];int ok=fread(h,1,54,f)==54;_fseeki64(f,0,SEEK_END);long long n=_ftelli64(f);fclose(f);return ok&&n>0&&c50_bmp_valid(h,(uint64_t)n);}
static int capture50_foreground(HWND window){DWORD pid=0;return GetForegroundWindow()==window&&GetWindowThreadProcessId(window,&pid)&&pid==GetCurrentProcessId()&&!IsIconic(window);}
static void capture50_poll(void){
 if(!capture50_job.process)return;DWORD status=STILL_ACTIVE;int queried=GetExitCodeProcess(capture50_job.process,&status),running=!queried||status==STILL_ACTIVE;
 int match=capture50_context(capture50_job.ua,capture50_job.planet),focus=capture50_foreground(capture50_job.window);int bmp=!running&&queried&&status==0&&capture50_valid_bmp(capture50_job.pending);
 int action=c50_result(capture50_job.start,GetTickCount64(),running,queried&&status==0,match,focus,bmp);if(action==C50_WAIT)return;
 if(running){TerminateProcess(capture50_job.process,4);WaitForSingleObject(capture50_job.process,50);}CloseHandle(capture50_job.process);capture50_job.process=NULL;
 int saved=action==C50_COMMIT&&MoveFileExW(capture50_job.pending,capture50_job.final,MOVEFILE_REPLACE_EXISTING|MOVEFILE_WRITE_THROUGH);
 if(!saved)DeleteFileW(capture50_job.pending);
#ifdef IGM_PHOTOS51
 if(saved)photos51_refresh(capture50_job.ua,capture50_job.planet);
#endif
 log_line("CAPTURE50 finished ua=%016llx planet=%u action=%d exit=%lu bmp=%d committed=%d",(unsigned long long)capture50_job.ua,capture50_job.planet,action,(unsigned long)status,bmp,saved);
 feature45_toast(tr48(saved?"Favorito y captura guardados.":"Favorito guardado; captura no disponible. Revisa captura_v050.log."));
}
static int capture50_start(uint64_t ua,unsigned planet){
 if(capture50_job.process||(capture50_last&&GetTickCount64()-capture50_last<5000)||!capture50_context(ua,planet))return 0;
 wchar_t ini[32768];make_path(ini,L"captura.ini");if(!GetPrivateProfileIntW(L"Captura",L"Activa",1,ini))return 0;
 HWND window=GetForegroundWindow();if(!capture50_foreground(window))return 0;
 wchar_t helper[32768],folder[32768],log[32768],command[32768];DWORD length=GetModuleFileNameW(self_module,helper,32768);if(!length||length>=32768)return 0;wchar_t*sep=wcsrchr(helper,L'\\');if(!sep)return 0;*sep=0;if(wcslen(helper)>32000)return 0;wcscat(helper,L"\\IGM\\IGM_Captura\\IGM_Captura.exe");if(GetFileAttributesW(helper)==INVALID_FILE_ATTRIBUTES){log_line("CAPTURE50 helper missing; favourite retained");return 0;}
 make_path(folder,L"capturas");if(!CreateDirectoryW(folder,NULL)&&GetLastError()!=ERROR_ALREADY_EXISTS)return 0;make_path(log,L"captura_v050.log");
 ULONGLONG now=GetTickCount64();if(swprintf(capture50_job.pending,32768,L"%ls\\pendiente_%lu_%llu.bmp",folder,(unsigned long)GetCurrentProcessId(),(unsigned long long)now)<0||swprintf(capture50_job.final,32768,L"%ls\\%016llx_p%02u.bmp",folder,(unsigned long long)ua,planet)<0)return 0;
 int n=swprintf(command,32768,L"\"%ls\" --window %llx --pid %lu --out \"%ls\" --log \"%ls\"",helper,(unsigned long long)(uintptr_t)window,(unsigned long)GetCurrentProcessId(),capture50_job.pending,log);if(n<=0||n>=32767)return 0;
 STARTUPINFOW startup={0};startup.cb=sizeof(startup);startup.dwFlags=STARTF_USESHOWWINDOW;startup.wShowWindow=SW_HIDE;PROCESS_INFORMATION process={0};capture50_last=now;
 if(!CreateProcessW(helper,command,NULL,NULL,FALSE,CREATE_NO_WINDOW|BELOW_NORMAL_PRIORITY_CLASS,NULL,NULL,&startup,&process)){log_line("CAPTURE50 helper start failed err=%lu",(unsigned long)GetLastError());return 0;}
 CloseHandle(process.hThread);capture50_job.process=process.hProcess;capture50_job.ua=ua;capture50_job.planet=planet;capture50_job.window=window;capture50_job.start=now;
 log_line("CAPTURE50 launched helper_pid=%lu ua=%016llx planet=%u; no rendering changes",(unsigned long)process.dwProcessId,(unsigned long long)ua,planet);return 1;
}
