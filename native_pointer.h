#ifndef IGM_NATIVE_POINTER_H
#define IGM_NATIVE_POINTER_H
#include <math.h>
#include <stdint.h>
/* BeginInput's final screen-space coordinates and native confirm state. */
typedef struct {float x,y,w,h;int down;uint64_t tick;} IgmNativePointer;
static int igm_native_sample(IgmNativePointer *p,const unsigned char data[0x54],const float size[2],uint64_t tick,int *edge){
 float xy[2];int button;memcpy(xy,data+0xc,8);memcpy(&button,data+0x34,4);
 if(!isfinite(size[0])||!isfinite(size[1])||size[0]<640||size[1]<360||size[0]>16384||size[1]>16384||!isfinite(xy[0])||!isfinite(xy[1])||xy[0]<0||xy[1]<0||xy[0]>size[0]+1||xy[1]>size[1]+1||button<0||button>3)return 0;
 int down=button==2||button==3;*edge=button==2&&!p->down;p->x=xy[0];p->y=xy[1];p->w=size[0];p->h=size[1];p->down=down;p->tick=tick;return 1;
}
static int igm_native_map(const IgmNativePointer *p,uint64_t tick,float *x,float *y){if(!p->tick||tick<p->tick||tick-p->tick>500)return 0;float s=fminf(p->w/1600,p->h/900);*x=(p->x-(p->w-1600*s)/2)/s;*y=(p->y-(p->h-900*s)/2)/s;return isfinite(*x)&&isfinite(*y);}
#endif
