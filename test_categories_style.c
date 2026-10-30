#include <assert.h>
#include <stdio.h>
#include "core.h"
#include "map_style.h"
int main(void) {player58_set(imperial53_id);assert(marker_fade(0,300,1200)==1);
    uint32_t own[4]={275889132u,1121173938u,1505499062u,4230148680u};
    uint32_t other[4]={3367193400u,1193069747u,4095667601u,696700356u},zero[4]={0};
    assert(alliance_category(own)==1);assert(alliance_category(other)==2);assert(alliance_category(zero)==0);
    uint32_t almost[4];memcpy(almost,own,16);almost[3]--;assert(alliance_category(almost)==2);
    /* Name, system name, bases and ownership are not inputs to this classifier. */
    assert(icon_scale(2160,0)==3);assert(icon_scale(1080,0)==1.5f);assert(icon_scale(720,0)==1);
    assert(icon_scale(2160,400)==4);assert(icon_scale(2160,10)==1);assert(icon_scale(2160,9999)==6);
    assert(icon_scale(NAN,0)==1);
    float viewport[2]={3840,2160},center[2];icon_center(2100,800,viewport,3,center);
    assert(center[0]==2172&&center[1]==800);
    icon_center(40,40,viewport,3,center);assert(center[0]==112&&center[1]==40);
    icon_center(4000,2500,viewport,3,center);assert(center[0]==4072&&center[1]==2500);
    /* Zoomed camera projections: marker displacement is constant in screen space across camera/zoom views. */
    float projections[4][2]={{1800,900},{1700,850},{1550,780},{20,40}};
    for(int i=0;i<4;i++)for(unsigned scale=1;scale<=6;scale++) {
        icon_center(projections[i][0],projections[i][1],viewport,(float)scale,center);
        assert(center[0]==projections[i][0]+24*(float)scale&&center[1]==projections[i][1]);
    }
    Address a={0,82,950,-93,1842,0};uint64_t ua=0;assert(pack_address(&a,&ua));
    size_t count=0;assert(valid_span(1,25,24,8,&count)&&count==1);
    puts("PASS: exact alliance IDs/UNKNOWN; 4K scale, overrides; marker stays 24*scale pixels right across 4 camera positions x 6 scales, without edge pinning");
}
