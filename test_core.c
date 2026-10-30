#include <assert.h>
#include <stdio.h>
#include "core.h"
int main(void) {
    Address genning={0,122,951,-93,1842,0};uint64_t ua=0;
    assert(sizeof(Address)==24);
    assert(pack_address(&genning,&ua)&&ua==0x7a00a37323b7ULL);
    genning.planet=5;assert(pack_address(&genning,&ua)&&ua==0x7a00a37323b7ULL);
    genning.galaxy=9;assert(pack_address(&genning,&ua)&&ua==0x7a09a37323b7ULL);
    genning.galaxy=256;assert(!pack_address(&genning,&ua));
    genning.galaxy=-1;assert(!pack_address(&genning,&ua));
    genning.galaxy=0;genning.solar=4096;assert(!pack_address(&genning,&ua));
    genning.solar=-1;assert(!pack_address(&genning,&ua));
    genning.solar=122;genning.x=2048;assert(!pack_address(&genning,&ua));
    genning.x=-2048;assert(!pack_address(&genning,&ua));
    genning.x=951;genning.y=128;assert(!pack_address(&genning,&ua));
    genning.y=-128;assert(!pack_address(&genning,&ua));
    genning.y=-93;genning.z=2048;assert(!pack_address(&genning,&ua));
    genning.z=-2048;assert(!pack_address(&genning,&ua));
    Address zero={0};assert(!pack_address(&zero,&ua));
    uint32_t own[4]={275889132u,1121173938u,1505499062u,4230148680u},other[4]={1,2,3,4},empty[4]={0};
    assert(nonzero_id(own)&&own_id(own));assert(nonzero_id(other)&&!own_id(other));assert(!nonzero_id(empty)&&!own_id(empty));
    size_t n=999;
    assert(valid_span(0x10000,0x10030,24,32768,&n)&&n==2);
    assert(!valid_span(0,0,24,32768,&n));
    assert(!valid_span(0x10000,0xffff,24,32768,&n));
    assert(!valid_span(0x10000,0x10001,24,32768,&n));
    assert(!valid_span(0x10000,0x10030,24,1,&n));
    puts("PASS: native address layout, Genning UA, planet exclusion, galaxy distinction, bounds, alliance classification, cache-span limits");
}
