#include <assert.h>
#include <stdio.h>
#include "map_catalogue.h"
int main(void) {player58_set(imperial53_id);
    const uint32_t own[4]={275889132u,1121173938u,1505499062u,4230148680u};
    const uint32_t foreign[4]={3367193400u,1193069747u,4095667601u,696700356u};
    IgmRecord rows[5]={{0x0000660008957f09ULL,{0}}, {0x00005200a37323b6ULL,{0}},
        {0x00005209a37323b6ULL,{0}}, {0x00001809239d8befULL,{0}}, {0x00001300a37323b6ULL,{0}}};
    memcpy(rows[0].id,foreign,16);memcpy(rows[1].id,own,16);
    memcpy(rows[2].id,own,16);memcpy(rows[3].id,foreign,16);
    IgmRecord result[5];size_t a,b,n;
    n=select_catalogue(rows,5,0,result,5,&a,&b);
    assert(n==2&&a==1&&b==1&&result[0].ua==rows[1].ua&&result[1].ua==rows[0].ua);
    n=select_catalogue(rows,5,9,result,5,&a,&b);
    assert(n==2&&a==1&&b==1&&result[0].ua==rows[2].ua&&result[1].ua==rows[3].ua);
    n=select_catalogue(rows,5,0,result,1,&a,&b);assert(n==1&&a==1&&b==0&&result[0].ua==rows[1].ua);
    n=select_catalogue(rows,5,0,result,0,&a,&b);assert(n==0&&a==0&&b==0);
    n=select_catalogue(rows,5,20,result,5,&a,&b);assert(n==0);
    Address address={0,82,950,-93,1842,0};uint64_t ua;assert(pack_address(&address,&ua));
    size_t count;assert(valid_span(1,25,24,1,&count));
    puts("PASS: production catalogue selector keeps own and foreign IDs separate, excludes zero, filters galaxy, prioritises own and respects capacity");
}
