#include <assert.h>
#include <stdio.h>
#include "position54.h"
static unsigned char mem[200000];static uint64_t base,image_base;
static struct {uint32_t renderer;} native={16};static Pos54Store positions54;
static int read_self(uint64_t a,void*out,size_t n){if(a<base||a-base>sizeof(mem)||n>sizeof(mem)-(size_t)(a-base))return 0;memcpy(out,mem+(a-base),n);return 1;}
static void put(size_t o,const void*p,size_t n){assert(o+n<=sizeof(mem));memcpy(mem+o,p,n);}
static unsigned projected;
static void project_points(void*r,const float*camera,float fov,const float*world,float*screen,unsigned n,unsigned char mode,const float*vp){assert(r==(void*)123&&n==1&&mode==0&&fov==60&&vp[0]==1280&&vp[1]==720);projected++;screen[0]=640+world[0]-camera[12];screen[1]=360-world[1]+camera[13];screen[3]=camera[10]>0?1:-1;}
#include "position54_render.inc"
int main(void){base=(uint64_t)(uintptr_t)mem;image_base=base+180000;uint64_t renderer=123,ua=UINT64_C(0x00005200a37323b6);put(180016,&renderer,8);int16_t origin[3]={950,1842,-93};put(0x1504,origin,6);float cam[16]={1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1},fov=60;put(0x12d0,cam,64);put(0x1490,&fov,4);float vp[2]={1280,720},x,y,dist;int mode;
 Pos54Entry*p=pos54_get(&positions54,ua,1);p->local[0]=10;p->local[1]=20;p->local[2]=30;assert(locate_system(base,ua,vp,&x,&y,&mode)&&mode==2&&x==650&&y==340&&projected==1);assert(marker_distance(base,ua,&dist)&&fabsf(dist-sqrtf(1400))<.01f);
 // Native visited marker count remains zero; rebased camera+origin preserves pixel anchor.
 origin[0]--;put(0x1504,origin,6);cam[12]=100;put(0x12d0,cam,64);assert(locate_system(base,ua,vp,&x,&y,&mode)&&mode==2&&x==650&&y==340);
 uint32_t anchors=1;put(0x13030,&anchors,4);put(0xcc30,&ua,8);float old[4]={500,300,1,1};put(0xb830,old,16);assert(locate_system(base,ua,vp,&x,&y,&mode)&&mode==1&&x==500); // native anchor survives even when private cached projection disagrees
 anchors=0;put(0x13030,&anchors,4);cam[10]=-1;put(0x12d0,cam,64);assert(!locate_system(base,ua,vp,&x,&y,&mode)); // native projector rejects points behind the camera
 cam[10]=1;put(0x12d0,cam,64);anchors=1;put(0x13030,&anchors,4);float native_world[8]={110,20,30,1,0,0,0,0};put(0x9030,native_world,32);assert(anchor55_distance(base,ua,&dist)&&fabsf(dist-sqrtf(1400))<.01f);uint64_t other=ua+1;put(0xcc30,&other,8);old[0]=500;old[1]=300;put(0xb830,old,16);assert(locate_system(base,other,vp,&x,&y,&mode)&&mode==1&&x==500);
 puts("PASS production positions54 projection routing: own positions draw with zero native markers, rebase tracks camera, native anchors retained despite mismatched private positions, native front/back status, native anchor distance independent of private cache. Native projection is a host stub.");}
