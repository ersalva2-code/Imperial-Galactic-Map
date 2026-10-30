#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "position54.h"
#include "imperial53.h"
static unsigned char mem[200000];static uint64_t base;
static int reader(uint64_t at,void*out,size_t n){if(at<base||at-base>sizeof(mem)||n>sizeof(mem)-(size_t)(at-base))return 0;memcpy(out,mem+(at-base),n);return 1;}
static void put(size_t off,const void*p,size_t n){assert(off+n<=sizeof(mem));memcpy(mem+off,p,n);}
static unsigned calls;static Pos54Coordinate requested;
static void *request(void*manager,unsigned char g,const Pos54Coordinate*c){assert((uintptr_t)manager==base+0x50&&g==0&&c->valid==1&&c->pad==0);requested=*c;calls++;return NULL;}
int main(void){
 base=(uint64_t)(uintptr_t)mem;static Pos54Store store;uint64_t ua=UINT64_C(0x00005200a37323b6),uas[]={ua,ua+1,ua+(UINT64_C(9)<<32)};int16_t origin[3]={950,1842,-93};put(0x1504,origin,6);float camera[3]={0};put(0x1300,camera,12);
 float world[8];assert(!pos54_world(&store,reader,base,ua,world));
 pos54_prefetch(&store,reader,base,uas,3,0,1200,100,request);assert(calls==1&&requested.x==950&&requested.z==1842&&requested.y==-93);
 pos54_prefetch(&store,reader,base,uas,3,0,1200,200,request);assert(calls==1);
 pos54_prefetch(&store,reader,base,uas,3,0,1200,250,request);assert(calls==2&&requested.x==951); // round-robin does not starve second missing region
 uint32_t count=1,stars=1;uint64_t table=base+100000,voxel=base+101000,addresses=base+103000,positions=base+104000;
 put(0xa4,&count,4);put(0xa8,&table,8);put(100000,&voxel,8);put(101000+0x260,origin,6);put(101000+0x214,&stars,4);put(101000+0x258,&addresses,8);put(101000+0x218,&positions,8);put(103000,&ua,8);float local[3]={10,20,30};put(104000,local,12);
 pos54_prepare(&store,reader,base,uas,3,0,1200);assert(pos54_world(&store,reader,base,ua,world)&&world[0]==10&&world[1]==20&&world[2]==30&&store.count==1);
 count=0;put(0xa4,&count,4);uint32_t anchors=0;put(0x13030,&anchors,4); // no engine voxels or visited circles remain
 origin[0]=949;origin[1]=1843;origin[2]=-94;put(0x1504,origin,6);
 assert(pos54_world(&store,reader,base,ua,world)&&world[0]==110&&world[1]==120&&world[2]==-70);assert(store.hits>=1);
 assert(!pos54_world(&store,reader,base,ua+(UINT64_C(9)<<32),world));
 uint64_t far[]={UINT64_C(0x0000520000000001)};pos54_prefetch(&store,reader,base,far,1,0,1200,500,request);assert(calls==2);
 // All 86 immutable core addresses can retain exact positions, independent of any history.
 for(size_t i=0;i<IMPERIAL53_COUNT;i++){Pos54Entry*p=pos54_get(&store,imperial53_systems[i].ua,1);assert(p);memcpy(p->local,local,12);assert(pos54_world(&store,reader,base,p->ua,world));}
 assert(store.count>=86);puts("PASS production positions54: async request ABI/coordinate/galaxy, 150ms throttle, round-robin, nearby-only, exact native local positions copied, retained after ALL voxels/white circles removed, camera-origin rebase, 86 core addresses, no history/selection prerequisite. Request callback is a host stub, not NMS execution.");
}
