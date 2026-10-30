#include <assert.h>
#include <stdio.h>
#include <stdarg.h>
#include <math.h>
#include "core.h"
#include "alliance_colors.h"
#include "icon52_format.h"
#define ICON52_LIMIT 128
static struct{uint32_t id[4];unsigned char pixels[ICON52_BYTES];}icons52[ICON52_LIMIT];static _Atomic size_t icons52_count;
static int icon52_find(const uint32_t id[4]){for(size_t i=0;i<atomic_load(&icons52_count);i++)if(!memcmp(id,icons52[i].id,16))return (int)i;return -1;}
typedef struct{uint64_t ua;uint32_t id[4];}Seen;static Seen known[3];static size_t known_count;static int known_lock;static void AcquireSRWLockShared(int*p){(void)p;}static void ReleaseSRWLockShared(int*p){(void)p;}
static IgmPalette palette;
static int get_alliance_color(const uint32_t id[4],unsigned char rgb[3],uint32_t*slot){IgmColor c;if(!palette_assign(&palette,id,&c))return 0;memcpy(rgb,c.rgb,3);*slot=c.slot;return 1;}
static void log_line(const char*s,...){(void)s;}
#include "auto58_icons.inc"
int main(void){assert(SYMBOL58_COUNT==178);unsigned char black[3]={0},white[3]={255,255,255},yellow[3]={250,210,40};assert(auto58_ink(black)==255&&auto58_ink(white)==0&&auto58_ink(yellow)==0);
for(unsigned i=0;i<256;i++){unsigned char rgb[3]={i,(unsigned char)(i*37),(unsigned char)(i*71)};double l=.2126*auto58_channel(rgb[0])+.7152*auto58_channel(rgb[1])+.0722*auto58_channel(rgb[2]);double ratio=auto58_ink(rgb)?1.05/(l+.05):(l+.05)/.05;assert(ratio>=4.5);}
const uint32_t a[4]={1,2,3,4},custom[4]={9,8,7,6};auto58_one(imperial53_id);assert(!atomic_load(&icons52_count));auto58_one(a);assert(atomic_load(&icons52_count)==1);unsigned char saved[ICON52_BYTES];memcpy(saved,icons52[0].pixels,sizeof(saved));auto58_one(a);assert(atomic_load(&icons52_count)==1&&!memcmp(saved,icons52[0].pixels,sizeof(saved)));
memcpy(icons52[1].id,custom,16);memset(icons52[1].pixels,77,ICON52_BYTES);atomic_store(&icons52_count,2);auto58_one(custom);assert(icons52[1].pixels[200]==77&&atomic_load(&icons52_count)==2);
player58_set(a);assert(palette_assign(&palette,a,&(IgmColor){0})==1);known_count=2;memcpy(known[0].id,imperial53_id,16);memcpy(known[1].id,a,16);auto58_sync();assert(atomic_load(&icons52_count)==2);
FILE*f=fopen("/tmp/auto58-reference.rgba","wb");assert(f);fwrite(saved,1,sizeof(saved),f);fclose(f);
printf("PASS auto58: 178 supplied symbols, hash=%u symbol=%u stable repeat, black-white/yellow-black, contrast>=4.5 on256 backgrounds, custom override retained, IESP never auto-generated, own foreign ID remains colour-eligible.\n",auto58_hash(a),auto58_hash(a)%SYMBOL58_COUNT);}
