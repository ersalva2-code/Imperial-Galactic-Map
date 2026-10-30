#ifndef IGM_PHOTO51_PIXELS
#define IGM_PHOTO51_PIXELS
#include "capture50_format.h"
#define P51_W 640
#define P51_H 360
#define P51_BYTES (P51_W*P51_H*4)
typedef struct {unsigned w,h;unsigned char pixels[P51_BYTES];} Photo51;
/* Full BGRA colours; area downsampling runs on the worker, never per frame. */
static int p51_read(FILE*f,Photo51*out){unsigned char h[54];if(fread(h,1,54,f)!=54)return 0;if(fseek(f,0,SEEK_END))return 0;long n=ftell(f);if(n<0||!c50_bmp_valid(h,(uint64_t)n))return 0;unsigned sw=c50_u32(h+18),sh=(unsigned)-(int32_t)c50_u32(h+22);out->w=sw<P51_W?sw:P51_W;out->h=(unsigned)((uint64_t)sh*out->w/sw);if(out->h>P51_H){out->h=P51_H;out->w=(unsigned)((uint64_t)sw*P51_H/sh);}if(!out->w||!out->h)return 0;unsigned char row[C50_MAX_DIM*4];
 for(unsigned y=0;y<out->h;y++){uint32_t sums[P51_W][3]={{0}},counts[P51_W]={0};unsigned y0=y*sh/out->h,y1=(y+1)*sh/out->h;if(fseek(f,54+(long)y0*sw*4,SEEK_SET))return 0;for(unsigned sy=y0;sy<y1;sy++){if(fread(row,4,sw,f)!=sw)return 0;for(unsigned x=0;x<out->w;x++)for(unsigned sx=x*sw/out->w;sx<(x+1)*sw/out->w;sx++){for(unsigned c=0;c<3;c++)sums[x][c]+=row[sx*4+c];counts[x]++;}}for(unsigned x=0;x<out->w;x++){size_t i=((size_t)y*out->w+x)*4;for(unsigned c=0;c<3;c++)out->pixels[i+c]=(unsigned char)((sums[x][c]+counts[x]/2)/counts[x]);out->pixels[i+3]=255;}}
 return 1;}
#endif
