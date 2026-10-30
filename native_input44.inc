#include "native_pointer.h"
#include "input44_patterns.h"
typedef void (*BeginInput44Fn)(void*,void*,const float*);
static BeginInput44Fn input44_original;
static SRWLOCK input44_lock=SRWLOCK_INIT;
static IgmNativePointer input44_pointer;
static _Atomic unsigned input44_clicks;
static unsigned input44_reports;static ULONGLONG input44_report_tick;
static void hook_input44(void *manager,void *state,const float *size){
 input44_original(manager,state,size);
 if(!menu_mode||!root39_owned_page||!native_root())return;
 /* +3a0 is the secondary input context; prefer primary when both run. */
 ULONGLONG now=GetTickCount64();unsigned char data[0x54];float bounds[2];
 if(!read_self((uint64_t)(uintptr_t)state,data,sizeof(data))||!read_self((uint64_t)(uintptr_t)size,bounds,8))return;
 AcquireSRWLockExclusive(&input44_lock);
 int secondary=(char*)state==(char*)manager+0x3a0;
 if(!secondary||!input44_pointer.tick||now-input44_pointer.tick>100){int edge=0;if(igm_native_sample(&input44_pointer,data,bounds,now,&edge)){if(edge)atomic_fetch_add(&input44_clicks,1);if(input44_reports++<3)log_line("INPUT44 SAMPLE native_xy=%.1f,%.1f size=%.0fx%.0f confirm=%d secondary=%d",input44_pointer.x,input44_pointer.y,bounds[0],bounds[1],input44_pointer.down,secondary);}}
 if(now-input44_report_tick>10000){input44_report_tick=now;log_line("INPUT44 STATUS native_xy=%.1f,%.1f size=%.0fx%.0f valid_age_ms=%llu samples=%u",input44_pointer.x,input44_pointer.y,input44_pointer.w,input44_pointer.h,(unsigned long long)(now-input44_pointer.tick),input44_reports);}
 ReleaseSRWLockExclusive(&input44_lock);
}
static int input44_get(float *x,float *y,int *click){IgmNativePointer p;AcquireSRWLockShared(&input44_lock);p=input44_pointer;ReleaseSRWLockShared(&input44_lock);int valid=igm_native_map(&p,GetTickCount64(),x,y);unsigned n=atomic_exchange(&input44_clicks,0);*click=valid&&n>0;return valid;}
static void input44_reset(void){AcquireSRWLockExclusive(&input44_lock);memset(&input44_pointer,0,sizeof(input44_pointer));ReleaseSRWLockExclusive(&input44_lock);atomic_store(&input44_clicks,0);}
