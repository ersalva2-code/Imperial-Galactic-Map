#ifndef IGM_MAP_STYLE_H
#define IGM_MAP_STYLE_H
#include <math.h>
static float icon_scale(float viewport_height,unsigned percent) {
    float s=percent?(float)percent/100.0f:viewport_height/720.0f;
    if(!isfinite(s)||s<1)s=1;
    if(s>6)s=6;
    return s;
}
static void icon_center(float x,float y,const float viewport[2],float scale,float out[2]) {
    /* Fixed screen-space offset at the chosen icon/UI scale. Independent of map zoom.
       No edge clamp: the marker never slides away to stay inside the viewport. */
    (void)viewport;out[0]=x+24.0f*scale;out[1]=y;
}
static float marker_fade(float distance,float start,float end){
 if(!isfinite(distance)||distance<0)return 1;
 if(!isfinite(start)||!isfinite(end)||start<0||end<=start)return 1;
 if(distance<=start)return 1;
 if(distance>=end)return 0;
 float t=(distance-start)/(end-start);return 1-t*t*(3-2*t);
}
#endif
