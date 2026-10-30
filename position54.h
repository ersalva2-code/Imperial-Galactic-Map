#ifndef IGM_POSITION54_H
#define IGM_POSITION54_H
#include "map_geometry.h"
#define POS54_SLOTS 8192
#define POS54_LIMIT 4096
/* Local stellar positions are immutable during this executable session; never retain game pointers.
   Galaxy/solar index are part of the key; recalculate world position from the live map origin. */
typedef struct {uint64_t ua;float local[3];} Pos54Entry;
typedef struct {Pos54Entry entries[POS54_SLOTS];size_t count,cursor,learn_cursor;uint64_t next_request;unsigned requests,hits,learned;} Pos54Store;
typedef struct {int16_t x,z,y;unsigned char valid,pad;} Pos54Coordinate;
typedef void *(*Pos54Request)(void *manager,unsigned char galaxy,const Pos54Coordinate *coordinate);
static size_t pos54_hash(uint64_t ua){ua^=ua>>33;ua*=UINT64_C(0xff51afd7ed558ccd);ua^=ua>>33;return (size_t)ua&(POS54_SLOTS-1);}
static Pos54Entry *pos54_get(Pos54Store*s,uint64_t ua,int create){
 if(!ua)return NULL;size_t start=pos54_hash(ua);
 for(size_t i=0;i<POS54_SLOTS;i++){Pos54Entry*p=&s->entries[(start+i)&(POS54_SLOTS-1)];if(p->ua==ua)return p;if(!p->ua){if(!create||s->count>=POS54_LIMIT)return NULL;p->ua=ua;s->count++;return p;}}return NULL;
}
static int pos54_learn(Pos54Store*s,ReadLocal reader,uint64_t map,uint64_t ua,float world[8]){
 Pos54Entry*p=pos54_get(s,ua,0);if(p){s->hits++;return local_world_point(reader,map,ua,p->local,world);}
 float local[3];if(!cached_local_point(reader,map,ua,local))return 0;
 p=pos54_get(s,ua,1);if(p){memcpy(p->local,local,12);s->learned++;}
 return local_world_point(reader,map,ua,local,world);
}
static int pos54_world(Pos54Store*s,ReadLocal reader,uint64_t map,uint64_t ua,float world[8]){Pos54Entry*p=pos54_get(s,ua,0);if(!p)return 0;s->hits++;return local_world_point(reader,map,ua,p->local,world);}
static int pos54_near_snapshot(const int16_t origin[3],const float camera[3],uint64_t ua,float range){
 int x=(int)(ua&4095)-((ua&2048)?4096:0),z=(int)((ua>>12)&4095)-((ua&(UINT64_C(1)<<23))?4096:0),y=(int8_t)(ua>>24);
 float dx=100.f*(int16_t)(x-origin[0])-camera[0],dy=100.f*(int16_t)(y-origin[2])-camera[1],dz=100.f*(int16_t)(z-origin[1])-camera[2],reach=range+200.f;return dx*dx+dy*dy+dz*dz<=reach*reach;
}
/* Round-robin cold reads, at most eight per frame. Drawing itself only consults our copies. */
static void pos54_prepare(Pos54Store*s,ReadLocal reader,uint64_t map,const uint64_t*uas,size_t n,unsigned galaxy,float range){
 if(!n||n>4096||!isfinite(range)||range<0||range>100000)return;int16_t origin[3];float camera[3];if(!reader(map+0x1504,origin,6)||!reader(map+0x1300,camera,12)||!finite_point(camera))return;unsigned probes=0;size_t start=s->learn_cursor%n;
 for(size_t k=0;k<n&&probes<8;k++){size_t i=(start+k)%n;uint64_t ua=uas[i];s->learn_cursor=(i+1)%n;if(((ua>>32)&255)!=galaxy||pos54_get(s,ua,0)||!pos54_near_snapshot(origin,camera,ua,range))continue;float world[8];probes++;pos54_learn(s,reader,map,ua,world);}
}
/* Ask the game's own asynchronous voxel cache for a nearby missing coordinate.
   No history, discovery flags, white circles, saved-game writes or networking.
   At most one request / 150 ms, round-robin; never block waiting for generation. */
static void pos54_prefetch(Pos54Store*s,ReadLocal reader,uint64_t map,const uint64_t*uas,size_t n,unsigned galaxy,float range,uint64_t now,Pos54Request request){
 if(!request||!n||n>4096||now<s->next_request||!isfinite(range)||range<0||range>100000)return;
 int16_t origin[3];float camera[3];if(!reader(map+0x1504,origin,6)||!reader(map+0x1300,camera,12)||!finite_point(camera))return;
 for(size_t k=0;k<n;k++){size_t i=(s->cursor+k)%n;uint64_t ua=uas[i];if(!ua||((ua>>32)&255)!=galaxy||pos54_get(s,ua,0))continue;
  Pos54Coordinate c={(int16_t)((int)(ua&4095)-((ua&2048)?4096:0)),(int16_t)((int)((ua>>12)&4095)-((ua&(UINT64_C(1)<<23))?4096:0)),(int8_t)(ua>>24),1,0};
  float dx=100.f*(int16_t)(c.x-origin[0])-camera[0],dy=100.f*(int16_t)(c.y-origin[2])-camera[1],dz=100.f*(int16_t)(c.z-origin[1])-camera[2];
  float reach=range+200.f;if(dx*dx+dy*dy+dz*dz>reach*reach)continue;
  /* ABI: void* RequestVoxel(cache*, uint8 galaxy, coordinate8*); returned pointer deliberately unused. */
  request((void*)(uintptr_t)(map+0x50),(unsigned char)galaxy,&c);s->requests++;s->next_request=now+150;s->cursor=(i+1)%n;return;
 }
}
#endif
