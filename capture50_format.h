#ifndef IGM_CAPTURE50_FORMAT
#define IGM_CAPTURE50_FORMAT
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#define C50_MAX_DIM 8192u
#define C50_MAX_BYTES (128u*1024u*1024u)
static inline void c50_copy_bgra_row(unsigned char*out,const unsigned char*in,unsigned width){memcpy(out,in,(size_t)width*4);for(unsigned x=0;x<width;x++)out[x*4+3]=255;}
static inline int c50_dims(unsigned w,unsigned h,size_t pitch){return w>=64&&h>=36&&w<=C50_MAX_DIM&&h<=C50_MAX_DIM&&pitch>=(size_t)w*4&&(uint64_t)w*h*4<=C50_MAX_BYTES;}
static inline void c50_le32(unsigned char*p,uint32_t n){for(int i=0;i<4;i++)p[i]=(unsigned char)(n>>(i*8));}
static inline uint32_t c50_u32(const unsigned char*p){return (uint32_t)p[0]|(uint32_t)p[1]<<8|(uint32_t)p[2]<<16|(uint32_t)p[3]<<24;}
static inline int c50_bmp_header(unsigned char out[54],unsigned w,unsigned h){if(!c50_dims(w,h,(size_t)w*4))return 0;memset(out,0,54);out[0]='B';out[1]='M';c50_le32(out+2,54+w*h*4);c50_le32(out+10,54);c50_le32(out+14,40);c50_le32(out+18,w);c50_le32(out+22,(uint32_t)(-(int32_t)h));out[26]=1;out[28]=32;c50_le32(out+34,w*h*4);return 1;}
static inline int c50_bmp_valid(const unsigned char b[54],uint64_t size){if(b[0]!='B'||b[1]!='M'||c50_u32(b+10)!=54||c50_u32(b+14)!=40||b[26]!=1||b[27]||b[28]!=32||b[29]||c50_u32(b+30))return 0;unsigned w=c50_u32(b+18);int32_t sh=(int32_t)c50_u32(b+22);if(sh>=0||sh==INT32_MIN)return 0;unsigned h=(unsigned)-sh;return c50_dims(w,h,(size_t)w*4)&&size==54+(uint64_t)w*h*4&&c50_u32(b+2)==size&&c50_u32(b+34)==(uint64_t)w*h*4;}
static inline int c50_nonblack(const unsigned char*p,unsigned w,unsigned h,size_t pitch){if(!p||!c50_dims(w,h,pitch))return 0;unsigned lit=0,sampled=0,max=0;for(unsigned y=0;y<h;y+=8)for(unsigned x=0;x<w;x+=8){const unsigned char*q=p+y*pitch+x*4;for(int c=0;c<3;c++)if(q[c]>max)max=q[c];if(q[0]>8||q[1]>8||q[2]>8)lit++;sampled++;}return max>16&&lit*1000>sampled;}
#endif
