#ifndef IGM_ALLIANCE_COLORS_H
#define IGM_ALLIANCE_COLORS_H
#include "core.h"
#include <math.h>
#include <stdio.h>
#define IGM_PALETTE_CAPACITY 4096
typedef struct {uint32_t id[4];uint32_t slot;unsigned char rgb[3];} IgmColor;
typedef struct {IgmColor entries[IGM_PALETTE_CAPACITY];size_t count;} IgmPalette;
static inline int palette_find(const IgmPalette *p,const uint32_t id[4]) {
    for(size_t i=0;i<p->count;i++)if(!memcmp(p->entries[i].id,id,16))return (int)i;
    return -1;
}
static inline void palette_rgb(uint32_t slot,unsigned char rgb[3]) {
    static const unsigned char fixed[16][3]={
        {12,15,21},{210,32,45},{35,95,225},{250,210,40},{24,145,70},
        {245,120,25},{112,51,190},{20,175,190},{245,245,245},{165,35,95},
        {25,65,110},{125,165,35},{105,53,25},{160,135,210},{235,145,175},{95,105,120}};
    if(slot<16){memcpy(rgb,fixed[slot],3);return;}
    uint32_t i=slot-16;
    float hue=fmodf((float)i*137.507764f,360.0f)/60.0f;
    float saturation=.60f+.10f*((i/128)%3),lightness=.42f+.08f*((i/384)%3);
    float c=(1-fabsf(2*lightness-1))*saturation,x=c*(1-fabsf(fmodf(hue,2)-1)),m=lightness-c/2;
    float r=0,g=0,b=0;
    if(hue<1){r=c;g=x;}else if(hue<2){r=x;g=c;}else if(hue<3){g=c;b=x;}
    else if(hue<4){g=x;b=c;}else if(hue<5){r=x;b=c;}else{r=c;b=x;}
    rgb[0]=(unsigned char)lroundf((r+m)*255);rgb[1]=(unsigned char)lroundf((g+m)*255);rgb[2]=(unsigned char)lroundf((b+m)*255);
}
static inline int palette_import(IgmPalette *p,const IgmColor *color) {
    if((!nonzero_id(color->id)||own_id(color->id))||color->slot>65535)return 0;
    int i=palette_find(p,color->id);
    if(i>=0)return p->entries[i].slot==color->slot&&!memcmp(p->entries[i].rgb,color->rgb,3);
    if(p->count>=IGM_PALETTE_CAPACITY)return 0;
    for(size_t j=0;j<p->count;j++)if(p->entries[j].slot==color->slot||!memcmp(p->entries[j].rgb,color->rgb,3))return 0;
    p->entries[p->count++]=*color;return 1;
}
/* 1=existing, 2=new, 0=not eligible/full. RGB values are persisted, not recomputed on reload. */
static inline int palette_assign(IgmPalette *p,const uint32_t id[4],IgmColor *result) {
    if((!nonzero_id(id)||own_id(id)))return 0;
    int i=palette_find(p,id);if(i>=0){*result=p->entries[i];return 1;}
    if(p->count>=IGM_PALETTE_CAPACITY)return 0;
    IgmColor color={0};memcpy(color.id,id,16);
    for(uint32_t slot=0;slot<=65535;slot++) {
        color.slot=slot;palette_rgb(slot,color.rgb);int free_color=1;
        for(size_t j=0;j<p->count;j++)if(p->entries[j].slot==slot||!memcmp(p->entries[j].rgb,color.rgb,3)){free_color=0;break;}
        if(free_color&&palette_import(p,&color)){*result=color;return 2;}
    }
    return 0;
}
static inline int palette_parse_line(const char *line,IgmColor *out) {
    unsigned long long v[8];char tail;
    if(sscanf(line,"%llu %llu %llu %llu %llu %llu %llu %llu %c",&v[0],&v[1],&v[2],&v[3],&v[4],&v[5],&v[6],&v[7],&tail)!=8)return 0;
    for(int i=0;i<4;i++){if(v[i]>UINT32_MAX)return 0;out->id[i]=(uint32_t)v[i];}
    if(v[4]>65535||v[5]>255||v[6]>255||v[7]>255)return 0;
    out->slot=(uint32_t)v[4];for(int i=0;i<3;i++)out->rgb[i]=(unsigned char)v[5+i];return (nonzero_id(out->id)&&!own_id(out->id));
}
static inline int palette_format_line(const IgmColor *c,char *out,size_t capacity) {
    int n=snprintf(out,capacity,"%u\t%u\t%u\t%u\t%u\t%u\t%u\t%u\n",c->id[0],c->id[1],c->id[2],c->id[3],c->slot,c->rgb[0],c->rgb[1],c->rgb[2]);
    return n>0&&(size_t)n<capacity;
}
#endif
