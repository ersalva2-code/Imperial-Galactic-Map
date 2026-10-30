#include <assert.h>
#include <stdio.h>
#include "map_style.h"
int main(void){assert(icon_scale(2160,0)==3);float p[2],v[2]={3840,2160};icon_center(100,200,v,3,p);assert(p[0]==172&&p[1]==200);assert(marker_fade(0,300,1200)==1);assert(marker_fade(300,300,1200)==1);assert(marker_fade(1200,300,1200)==0);assert(marker_fade(1300,300,1200)==0);assert(fabsf(marker_fade(750,300,1200)-.5f)<.0001f);float prior=1;for(int d=0;d<2000;d++){float a=marker_fade((float)d,300,1200);assert(a>=0&&a<=prior);prior=a;}assert(marker_fade(NAN,300,1200)==1);assert(marker_fade(500,1000,20)==1);puts("PASS opacity base .9 with smooth monotone fade, near/mid/far, invalid inputs; right offset preserved");}
