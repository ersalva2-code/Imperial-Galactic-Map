#ifndef IGM_ICON52_FORMAT
#define IGM_ICON52_FORMAT
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "capture50_format.h"
#define ICON52_DIM 180
#define ICON52_BYTES (ICON52_DIM*ICON52_DIM*4)
/* Companion exports a full-colour, top-down BGRA32 BMP with straight alpha. */
static int icon52_bmp(FILE*f,unsigned char*out){unsigned char h[54];if(fread(h,1,54,f)!=54||fseek(f,0,SEEK_END))return 0;long n=ftell(f);if(n<0||!c50_bmp_valid(h,(uint64_t)n)||c50_u32(h+18)!=ICON52_DIM||c50_u32(h+22)!=(uint32_t)-ICON52_DIM)return 0;if(fseek(f,54,SEEK_SET))return 0;return fread(out,1,ICON52_BYTES,f)==ICON52_BYTES;}
/* Historical exports remain readable; no vectors-per-pixel on the render thread. */
static int icon52_legacy(FILE*f,unsigned char*out){char line[160],extra;unsigned count,dim=24;int used=0;if(!fgets(line,sizeof(line),f))return 0;if(sscanf(line,"IGMICON2 %u %u %n",&count,&dim,&used)!=2||dim!=96){used=0;dim=24;if(sscanf(line,"IGMICON1 %u %n",&count,&used)!=1)return 0;}if(!used||line[used]||count>dim*dim)return 0;unsigned char raw[96*96*4]={0};for(unsigned k=0;k<count;k++){unsigned x,y,n,c;if(!fgets(line,sizeof(line),f)||sscanf(line,"%u %u %u %x %c",&x,&y,&n,&c,&extra)!=4||x>=dim||y>=dim||!n||n>dim-x)return 0;for(unsigned i=0;i<n;i++){unsigned char*p=raw+((size_t)y*dim+x+i)*4;p[0]=(unsigned char)(c>>8);p[1]=(unsigned char)(c>>16);p[2]=(unsigned char)(c>>24);p[3]=(unsigned char)c;}}while(fgets(line,sizeof(line),f)){if(sscanf(line," %c",&extra)==1)return 0;}for(unsigned y=0;y<ICON52_DIM;y++)for(unsigned x=0;x<ICON52_DIM;x++)memcpy(out+((size_t)y*ICON52_DIM+x)*4,raw+((size_t)(y*dim/ICON52_DIM)*dim+x*dim/ICON52_DIM)*4,4);return !ferror(f);}
#endif
