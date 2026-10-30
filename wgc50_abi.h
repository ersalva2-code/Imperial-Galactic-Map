#ifndef IGM_WGC50_ABI
#define IGM_WGC50_ABI
/* WinRT ABI layouts checked against Microsoft windows-rs Graphics/Capture.
   Only methods used in this one-frame capture are exposed. No SDK implementation is copied. */
#include <windows.h>
#include <winstring.h>
#include <stdint.h>
#include <stddef.h>
typedef struct {int32_t w,h;} C50Size;
typedef struct {
 HRESULT (STDMETHODCALLTYPE *QueryInterface)(void*,REFIID,void**);
 ULONG (STDMETHODCALLTYPE *AddRef)(void*);
 ULONG (STDMETHODCALLTYPE *Release)(void*);
 void *GetIids,*GetRuntimeClassName,*GetTrustLevel;
} C50Inspectable;
typedef struct {C50Inspectable base;HRESULT (STDMETHODCALLTYPE *DisplayName)(void*,HSTRING*);HRESULT (STDMETHODCALLTYPE *Size)(void*,C50Size*);} C50Item;
typedef struct {C50Inspectable base;HRESULT (STDMETHODCALLTYPE *CreateFreeThreaded)(void*,void*,int,int,C50Size,void**);} C50PoolFactory;
typedef struct {C50Inspectable base;void*Recreate;HRESULT(STDMETHODCALLTYPE *Next)(void*,void**);void*AddFrameArrived;void*RemoveFrameArrived;HRESULT(STDMETHODCALLTYPE *Session)(void*,void*,void**);void*Dispatcher;} C50Pool;
typedef struct {C50Inspectable base;HRESULT(STDMETHODCALLTYPE *Surface)(void*,void**);HRESULT(STDMETHODCALLTYPE *Time)(void*,int64_t*);HRESULT(STDMETHODCALLTYPE *ContentSize)(void*,C50Size*);} C50Frame;
typedef struct {C50Inspectable base;HRESULT(STDMETHODCALLTYPE *Start)(void*);} C50Session;
typedef struct {C50Inspectable base;HRESULT(STDMETHODCALLTYPE *Close)(void*);} C50Closable;
typedef struct {HRESULT(STDMETHODCALLTYPE *QueryInterface)(void*,REFIID,void**);ULONG(STDMETHODCALLTYPE *AddRef)(void*);ULONG(STDMETHODCALLTYPE *Release)(void*);HRESULT(STDMETHODCALLTYPE *Window)(void*,HWND,REFIID,void**);void*Monitor;} C50Interop;
typedef struct {HRESULT(STDMETHODCALLTYPE *QueryInterface)(void*,REFIID,void**);ULONG(STDMETHODCALLTYPE *AddRef)(void*);ULONG(STDMETHODCALLTYPE *Release)(void*);HRESULT(STDMETHODCALLTYPE *GetInterface)(void*,REFIID,void**);} C50SurfaceAccess;
#define C50_VT(obj,type) (*(type**)(obj))
_Static_assert(offsetof(C50Pool,Next)==7*sizeof(void*),"frame pool ABI");
_Static_assert(offsetof(C50Pool,Session)==10*sizeof(void*),"capture session ABI");
_Static_assert(offsetof(C50Item,Size)==7*sizeof(void*),"item size ABI");
_Static_assert(offsetof(C50Frame,ContentSize)==8*sizeof(void*),"frame size ABI");
static const GUID C50_IID_ITEM={0x79c3f95b,0x31f7,0x4ec2,{0xa4,0x64,0x63,0x2e,0xf5,0xd3,0x07,0x60}};
static const GUID C50_IID_INTEROP={0x3628e81b,0x3cac,0x4c60,{0xb7,0xf4,0x23,0xce,0x0e,0x0c,0x33,0x56}};
static const GUID C50_IID_POOL_FACTORY={0x589b103f,0x6bbc,0x5df5,{0xa9,0x91,0x02,0xe2,0x8b,0x3b,0x66,0xd5}};
static const GUID C50_IID_CLOSABLE={0x30d5a829,0x7fa4,0x4026,{0x83,0xbb,0xd7,0x5b,0xae,0x4e,0xa9,0x9e}};
static const GUID C50_IID_SURFACE_ACCESS={0xa9b3d012,0x3df2,0x4ee3,{0xb8,0xd1,0x86,0x95,0xf4,0x57,0xd3,0xc1}};
static const GUID C50_IID_DEVICE={0xa37624ab,0x8d5f,0x4650,{0x9d,0x3e,0x9e,0xae,0x3d,0x9b,0xc6,0x70}};
static void c50_release(void*p){if(p)C50_VT(p,C50Inspectable)->Release(p);}
static void c50_close(void*p){if(!p)return;void*q=NULL;if(SUCCEEDED(C50_VT(p,C50Inspectable)->QueryInterface(p,&C50_IID_CLOSABLE,&q))&&q){C50_VT(q,C50Closable)->Close(q);c50_release(q);}}
#endif
