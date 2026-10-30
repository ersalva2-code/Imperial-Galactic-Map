#ifndef IGM_MAP_GEOMETRY_H
#define IGM_MAP_GEOMETRY_H
#include <math.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
typedef int (*ReadLocal)(uint64_t,void*,size_t);
static int finite_point(const float p[3]) {
    return isfinite(p[0])&&isfinite(p[1])&&isfinite(p[2])&&
           fabsf(p[0])<1000000 && fabsf(p[1])<1000000 && fabsf(p[2])<1000000;
}
/* Recover a star by exact UA from already loaded local voxels. No generating,
   loading, requesting or modifying voxel data. Layout from native GetSolar result. */
static int cached_local_point(ReadLocal reader,uint64_t map,uint64_t ua,float local_out[3]) {
    uint32_t count=0;uint64_t voxels=0;
    if(!reader(map+0xa4,&count,4)||count>4096 || !reader(map+0xa8,&voxels,8)||!voxels)return 0;
    int16_t target[3]={0,0,(int8_t)(ua>>24)};
    /* X sign extension uses explicit masks below, independent of C narrowing. */
    target[0]=(int16_t)((int)(ua&4095)-((ua&2048)?4096:0));
    target[1]=(int16_t)((int)((ua>>12)&4095)-((ua&(1ULL<<23))?4096:0));
    for(uint32_t i=0;i<count;i++) {
        uint64_t voxel=0;int16_t coord[3];unsigned char busy=1;
        if(!reader(voxels+i*8,&voxel,8)||!voxel||!reader(voxel+0x260,coord,6))continue;
        if(memcmp(coord,target,6))continue;
        if(!reader(voxel+0x275,&busy,1)||busy)return 0;
        uint32_t stars=0;uint64_t addresses=0,positions=0;
        if(!reader(voxel+0x214,&stars,4)||stars>4096||!stars ||
           !reader(voxel+0x258,&addresses,8)||!addresses||!reader(voxel+0x218,&positions,8)||!positions)return 0;
        static _Thread_local uint64_t address_copy[4096];
        if(!reader(addresses,address_copy,(size_t)stars*8))return 0;
        for(uint32_t s=0;s<stars;s++) {
            uint64_t found=address_copy[s];float local[3];
            if((found&0x000fffffffffffffULL)!=ua)continue;
            if(!reader(positions+s*12,local,12)||!finite_point(local))return 0;
            uint64_t check=0;unsigned char still_busy=1;
            if(!reader(addresses+s*8,&check,8)||check!=found||
               !reader(voxel+0x275,&still_busy,1)||still_busy)return 0;
            memcpy(local_out,local,12);return 1;
        }
        return 0;
    }
    return 0;
}
static int local_world_point(ReadLocal reader,uint64_t map,uint64_t ua,const float local[3],float world[8]) {
 int16_t origin[3];if(!reader(map+0x1504,origin,6)||!finite_point(local))return 0;
 int x=(int)(ua&4095)-((ua&2048)?4096:0),z=(int)((ua>>12)&4095)-((ua&(UINT64_C(1)<<23))?4096:0),y=(int8_t)(ua>>24);
 memset(world,0,32);world[0]=local[0]+100.f*(int16_t)(x-origin[0]);world[1]=local[1]+100.f*(int16_t)(y-origin[2]);world[2]=local[2]+100.f*(int16_t)(z-origin[1]);world[3]=1;
 return finite_point(world);
}
static inline int cached_world_point(ReadLocal reader,uint64_t map,uint64_t ua,float world[8]) {
 float local[3];return cached_local_point(reader,map,ua,local)&&local_world_point(reader,map,ua,local,world);
}
static int existing_anchor(ReadLocal reader,uint64_t map,uint64_t ua,float *x,float *y,const float viewport[2]) {
    uint32_t n=0;if(!reader(map+0x13030,&n,4)||n>320)return 0;
    for(uint32_t i=0;i<n;i++) {
        uint64_t a=0;float p[4];
        if(!reader(map+0xcc30+i*8,&a,8)||(a&0x000fffffffffffffULL)!=ua)continue;
        if(!reader(map+0xb830+i*16,p,16))continue;
        if(!isfinite(p[0])||!isfinite(p[1])||!isfinite(p[2])||p[2]<=0||
           p[0]<16||p[1]<16||p[0]>viewport[0]-16||p[1]>viewport[1]-16)continue;
        *x=p[0];*y=p[1];return 1;
    }
    return 0;
}

#endif
