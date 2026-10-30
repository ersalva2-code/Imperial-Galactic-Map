#define WIN32_LEAN_AND_MEAN
#define COBJMACROS
#undef _WIN32_WINNT
#define _WIN32_WINNT 0x0A00
#include <windows.h>
#include <shellapi.h>
#include <initguid.h>
#include <d3d11.h>
#include <dxgi.h>
#include <stdint.h>
#include <stdio.h>
#include <wchar.h>
#include "capture50_format.h"
#include "wgc50_abi.h"

typedef HRESULT(WINAPI *RoInit50)(unsigned);
typedef void(WINAPI *RoUninit50)(void);
typedef HRESULT(WINAPI *RoFactory50)(HSTRING,REFIID,void**);
typedef HRESULT(WINAPI *String50)(const wchar_t*,unsigned,HSTRING*);
typedef HRESULT(WINAPI *DeleteString50)(HSTRING);
typedef HRESULT(WINAPI *WinRTDevice50)(IDXGIDevice*,void**);
static HANDLE report50=INVALID_HANDLE_VALUE;
static void report(const char *stage,HRESULT hr){if(report50==INVALID_HANDLE_VALUE)return;char b[256];SYSTEMTIME t;GetSystemTime(&t);int n=snprintf(b,sizeof(b),"%04u-%02u-%02uT%02u:%02u:%02uZ C50 %s hr=0x%08lx\r\n",t.wYear,t.wMonth,t.wDay,t.wHour,t.wMinute,t.wSecond,stage,(unsigned long)hr);DWORD done;if(n>0)WriteFile(report50,b,(DWORD)n,&done,NULL);}
static int target_ok(HWND window,DWORD pid){DWORD actual=0;return IsWindow(window)&&GetWindowThreadProcessId(window,&actual)&&actual==pid&&GetForegroundWindow()==window&&IsWindowVisible(window)&&!IsIconic(window);}
static int game_process(DWORD pid){HANDLE h=OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION,FALSE,pid);if(!h)return 0;wchar_t p[32768];DWORD n=32768;int ok=QueryFullProcessImageNameW(h,0,p,&n);CloseHandle(h);if(!ok)return 0;const wchar_t*name=wcsrchr(p,L'\\');return _wcsicmp(name?name+1:p,L"NMS.exe")==0;}
static int write_bmp(const wchar_t *path,const D3D11_MAPPED_SUBRESOURCE *m,unsigned w,unsigned h){
 unsigned char header[54];if(!c50_bmp_header(header,w,h)||!c50_nonblack(m->pData,w,h,m->RowPitch))return 0;
 HANDLE f=CreateFileW(path,GENERIC_WRITE,0,NULL,CREATE_NEW,FILE_ATTRIBUTE_NORMAL,NULL);if(f==INVALID_HANDLE_VALUE)return 0;
 DWORD done=0;int ok=WriteFile(f,header,54,&done,NULL)&&done==54;unsigned char*row=HeapAlloc(GetProcessHeap(),0,(size_t)w*4);if(!row)ok=0;
 for(unsigned y=0;ok&&y<h;y++){c50_copy_bgra_row(row,(const unsigned char*)m->pData+y*(size_t)m->RowPitch,w);ok=WriteFile(f,row,w*4,&done,NULL)&&done==w*4;}
 if(row)HeapFree(GetProcessHeap(),0,row);if(ok)ok=FlushFileBuffers(f)!=0;CloseHandle(f);if(!ok)DeleteFileW(path);return ok;
}
static int capture(HWND window,DWORD pid,const wchar_t*out){
 HRESULT hr=E_FAIL;const char*stage="initialization";int result=1,initialized=0,acquired=0;
 HMODULE combase=NULL,d3d=NULL;RoInit50 init=NULL;RoUninit50 uninit=NULL;RoFactory50 factory=NULL;String50 str=NULL;DeleteString50 delstr=NULL;WinRTDevice50 rtdev=NULL;
 HSTRING class_name=NULL;ID3D11Device*device=NULL;ID3D11DeviceContext*context=NULL;IDXGIDevice*dxgi=NULL;ID3D11Texture2D*texture=NULL,*staging=NULL;
 void*interop=NULL,*item=NULL,*pool_factory=NULL,*inspectable_device=NULL,*winrt_device=NULL,*pool=NULL,*session=NULL,*frame=NULL,*surface=NULL,*access=NULL;
 if(!target_ok(window,pid)||!game_process(pid)){stage="foreground NMS window required";hr=E_ACCESSDENIED;goto done;}
 combase=LoadLibraryExW(L"combase.dll",NULL,LOAD_LIBRARY_SEARCH_SYSTEM32);d3d=LoadLibraryExW(L"d3d11.dll",NULL,LOAD_LIBRARY_SEARCH_SYSTEM32);
 if(!combase||!d3d){stage="Windows Graphics Capture unavailable";goto done;}
 init=(RoInit50)(void*)GetProcAddress(combase,"RoInitialize");uninit=(RoUninit50)(void*)GetProcAddress(combase,"RoUninitialize");factory=(RoFactory50)(void*)GetProcAddress(combase,"RoGetActivationFactory");str=(String50)(void*)GetProcAddress(combase,"WindowsCreateString");delstr=(DeleteString50)(void*)GetProcAddress(combase,"WindowsDeleteString");rtdev=(WinRTDevice50)(void*)GetProcAddress(d3d,"CreateDirect3D11DeviceFromDXGIDevice");
 if(!init||!uninit||!factory||!str||!delstr||!rtdev){stage="required Windows APIs absent";goto done;}
 stage="RoInitialize MTA";hr=init(1);if(FAILED(hr))goto done;initialized=1;
 stage="D3D11CreateDevice BGRA";hr=D3D11CreateDevice(NULL,D3D_DRIVER_TYPE_HARDWARE,NULL,D3D11_CREATE_DEVICE_BGRA_SUPPORT,NULL,0,D3D11_SDK_VERSION,&device,NULL,&context);if(FAILED(hr))goto done;
 stage="D3D11 IDXGIDevice";hr=ID3D11Device_QueryInterface(device,&IID_IDXGIDevice,(void**)&dxgi);if(FAILED(hr))goto done;
 stage="WinRT Direct3D device";hr=rtdev(dxgi,&inspectable_device);if(FAILED(hr))goto done;
 stage="IDirect3DDevice interface";hr=C50_VT(inspectable_device,C50Inspectable)->QueryInterface(inspectable_device,&C50_IID_DEVICE,&winrt_device);if(FAILED(hr))goto done;
 const wchar_t*name=L"Windows.Graphics.Capture.GraphicsCaptureItem";
 stage="GraphicsCaptureItem activation";hr=str(name,(unsigned)wcslen(name),&class_name);if(FAILED(hr))goto done;hr=factory(class_name,&C50_IID_INTEROP,&interop);delstr(class_name);class_name=NULL;if(FAILED(hr))goto done;
 stage="CreateForWindow";hr=C50_VT(interop,C50Interop)->Window(interop,window,&C50_IID_ITEM,&item);if(FAILED(hr))goto done;
 C50Size size={0};stage="capture item size";hr=C50_VT(item,C50Item)->Size(item,&size);if(FAILED(hr)||size.w<64||size.h<36||!c50_dims((unsigned)size.w,(unsigned)size.h,(size_t)size.w*4)){hr=E_INVALIDARG;goto done;}
 name=L"Windows.Graphics.Capture.Direct3D11CaptureFramePool";stage="FramePool activation";hr=str(name,(unsigned)wcslen(name),&class_name);if(FAILED(hr))goto done;hr=factory(class_name,&C50_IID_POOL_FACTORY,&pool_factory);delstr(class_name);class_name=NULL;if(FAILED(hr))goto done;
 stage="CreateFreeThreaded BGRA8 two buffers";hr=C50_VT(pool_factory,C50PoolFactory)->CreateFreeThreaded(pool_factory,winrt_device,87,2,size,&pool);if(FAILED(hr))goto done;
 stage="CreateCaptureSession";hr=C50_VT(pool,C50Pool)->Session(pool,item,&session);if(FAILED(hr))goto done;
 stage="StartCapture";hr=C50_VT(session,C50Session)->Start(session);if(FAILED(hr))goto done;
 ULONGLONG deadline=GetTickCount64()+2500;stage="waiting for GPU frame";
 while(GetTickCount64()<deadline){if(!target_ok(window,pid)){stage="focus lost; discarded";hr=E_ABORT;goto done;}hr=C50_VT(pool,C50Pool)->Next(pool,&frame);if(FAILED(hr))goto done;if(frame)break;Sleep(16);}
 if(!frame){hr=HRESULT_FROM_WIN32(WAIT_TIMEOUT);goto done;}
 C50Size content={0};stage="frame content bounds";hr=C50_VT(frame,C50Frame)->ContentSize(frame,&content);if(FAILED(hr)||content.w!=size.w||content.h!=size.h){hr=E_ABORT;goto done;}
 stage="frame surface";hr=C50_VT(frame,C50Frame)->Surface(frame,&surface);if(FAILED(hr))goto done;
 stage="DXGI surface access";hr=C50_VT(surface,C50Inspectable)->QueryInterface(surface,&C50_IID_SURFACE_ACCESS,&access);if(FAILED(hr))goto done;
 stage="D3D11 frame texture";hr=C50_VT(access,C50SurfaceAccess)->GetInterface(access,&IID_ID3D11Texture2D,(void**)&texture);if(FAILED(hr))goto done;
 D3D11_TEXTURE2D_DESC desc;ID3D11Texture2D_GetDesc(texture,&desc);if(desc.Format!=DXGI_FORMAT_B8G8R8A8_UNORM||desc.Width<(unsigned)content.w||desc.Height<(unsigned)content.h||desc.SampleDesc.Count!=1){stage="unsupported texture layout";hr=E_INVALIDARG;goto done;}
 desc.Width=(unsigned)content.w;desc.Height=(unsigned)content.h;desc.MipLevels=1;desc.ArraySize=1;desc.Usage=D3D11_USAGE_STAGING;desc.BindFlags=0;desc.CPUAccessFlags=D3D11_CPU_ACCESS_READ;desc.MiscFlags=0;
 stage="CPU staging texture";hr=ID3D11Device_CreateTexture2D(device,&desc,NULL,&staging);if(FAILED(hr))goto done;
 D3D11_BOX region={0,0,0,(unsigned)content.w,(unsigned)content.h,1};ID3D11DeviceContext_CopySubresourceRegion(context,(ID3D11Resource*)staging,0,0,0,0,(ID3D11Resource*)texture,0,&region);
 ID3D11DeviceContext_Flush(context);
 D3D11_MAPPED_SUBRESOURCE mapped;memset(&mapped,0,sizeof(mapped));stage="GPU readback";
 deadline=GetTickCount64()+1800;do{hr=ID3D11DeviceContext_Map(context,(ID3D11Resource*)staging,0,D3D11_MAP_READ,D3D11_MAP_FLAG_DO_NOT_WAIT,&mapped);if(hr!=DXGI_ERROR_WAS_STILL_DRAWING)break;if(!target_ok(window,pid)){hr=E_ABORT;break;}Sleep(8);}while(GetTickCount64()<deadline);
 if(FAILED(hr))goto done;acquired=1;
 if(!target_ok(window,pid)){stage="focus lost before write; discarded";hr=E_ABORT;goto done;}
 stage="write full colour BMP";if(!write_bmp(out,&mapped,(unsigned)content.w,(unsigned)content.h)){hr=E_FAIL;goto done;}
 if(!target_ok(window,pid)){DeleteFileW(out);stage="focus lost after write; discarded";hr=E_ABORT;goto done;}
 report("BMP saved: full BGRA8 source resolution; no palette",S_OK);result=0;
 done:
 if(result)report(stage,hr);
 if(acquired)ID3D11DeviceContext_Unmap(context,(ID3D11Resource*)staging,0);
 c50_close(frame);c50_close(session);c50_close(pool);
 c50_release(staging);c50_release(texture);c50_release(access);c50_release(surface);c50_release(frame);c50_release(session);c50_release(pool);c50_release(pool_factory);c50_release(item);c50_release(interop);c50_release(winrt_device);c50_release(inspectable_device);c50_release(dxgi);c50_release(context);c50_release(device);
 if(class_name&&delstr)delstr(class_name);if(initialized)uninit();if(d3d)FreeLibrary(d3d);if(combase)FreeLibrary(combase);return result;
}
int WINAPI wWinMain(HINSTANCE instance,HINSTANCE previous,wchar_t*command,int show){(void)instance;(void)previous;(void)command;(void)show;
 int argc=0;wchar_t**argv=CommandLineToArgvW(GetCommandLineW(),&argc);if(!argv)return 2;HWND window=NULL;DWORD pid=0;const wchar_t*out=NULL,*log=NULL;int valid=argc==9;
 for(int i=1;valid&&i+1<argc;i+=2){wchar_t*end=NULL;if(!wcscmp(argv[i],L"--window")){unsigned long long n=wcstoull(argv[i+1],&end,16);if(!n||*end)valid=0;window=(HWND)(uintptr_t)n;}else if(!wcscmp(argv[i],L"--pid")){unsigned long n=wcstoul(argv[i+1],&end,10);if(!n||*end)valid=0;pid=(DWORD)n;}else if(!wcscmp(argv[i],L"--out"))out=argv[i+1];else if(!wcscmp(argv[i],L"--log"))log=argv[i+1];else valid=0;}
 int result=2;if(valid&&window&&pid&&out&&log){report50=CreateFileW(log,FILE_APPEND_DATA,FILE_SHARE_READ,NULL,OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);report("helper v050 WGC window-only start",S_OK);result=capture(window,pid,out);if(report50!=INVALID_HANDLE_VALUE)CloseHandle(report50);}LocalFree(argv);return result;
}
