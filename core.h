#ifndef IGM_CORE_H
#define IGM_CORE_H
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdatomic.h>
#include "imperial53.h"
/* These are the six int32 fields of cGcUniverseAddressData in the supplied EXE. */
typedef struct { int32_t planet, solar, x, y, z, galaxy; } Address;
static int pack_address(const Address *a, uint64_t *out) {
    if(a->solar<0 || a->solar>4095 || a->x< -2047 || a->x>2047 ||
       a->z< -2047 || a->z>2047 || a->y< -127 || a->y>127 ||
       a->galaxy<0 || a->galaxy>255) return 0;
    *out=((uint64_t)a->solar<<40)|((uint64_t)a->galaxy<<32)|
         ((uint64_t)(a->y&255)<<24)|((uint64_t)(a->z&4095)<<12)|(a->x&4095);
    return *out!=0; /* Reject uninitialised menu data. Planet intentionally excluded. */
}
static int nonzero_id(const uint32_t id[4]) { return (id[0]|id[1]|id[2]|id[3])!=0; }
static int own_id(const uint32_t id[4]) {
    const uint32_t own[4]={275889132u,1121173938u,1505499062u,4230148680u};
    return memcmp(id,own,sizeof(own))==0;
}
/* Player identity is independent of the immutable imperial emblem identity. */
static _Atomic uint32_t player58_words[4];static _Atomic unsigned player58_seq;
static void player58_set(const uint32_t id[4]){atomic_fetch_add(&player58_seq,1);for(int i=0;i<4;i++)atomic_store(&player58_words[i],id?id[i]:0);atomic_fetch_add(&player58_seq,1);}
static int player58_get(uint32_t id[4]){for(int k=0;k<3;k++){unsigned a=atomic_load(&player58_seq);if(a&1)continue;for(int i=0;i<4;i++)id[i]=atomic_load(&player58_words[i]);if(a==atomic_load(&player58_seq))return nonzero_id(id);}memset(id,0,16);return 0;}
static int player58_own(const uint32_t id[4]){uint32_t p[4];return nonzero_id(id)&&player58_get(p)&&!memcmp(p,id,16);}
/* An unknown/zero ID is never treated as another alliance or as a pirate. */
static inline int alliance_category(const uint32_t id[4]) {
    return nonzero_id(id)?(player58_own(id)?1:2):0;
}
static int valid_span(uint64_t begin, uint64_t end, size_t stride, size_t limit, size_t *count) {
    if(!begin || end<begin || (end-begin)%stride || (end-begin)/stride>limit) return 0;
    *count=(size_t)((end-begin)/stride); return 1;
}
#endif
