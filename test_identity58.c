#include <assert.h>
#include <stdio.h>
#include <stdarg.h>
#include "core.h"
#include "compat_scan.h"
#include <stdlib.h>

static uint32_t native_text_rva,native_text_size;static struct {uint32_t alliance_offset;}native={0x1000};static unsigned char live[32];static int good=1,torn;
static int code_read_fail,alloc_fail;static unsigned code_reads;
static int read_self(uint64_t p,void*out,size_t n){if(p==0x100000+0x1000&&n==200){code_reads++;if(code_read_fail)return 0;memset(out,0xcc,n);extern const unsigned char *fixture59_pattern;memcpy(out,fixture59_pattern,172);return 1;}if(!good||p!=0x10000+native.alliance_offset+0x144||n!=32)return 0;memcpy(out,live,n);if(torn)((unsigned char*)out)[0]^=torn++&1;return 1;}
static void log_line(const char*s,...){(void)s;}
static void remember_name(const uint32_t id[4],const char*n){(void)id;(void)n;}
#define MEM_COMMIT 1
#define MEM_RESERVE 2
#define PAGE_READWRITE 4
#define MEM_RELEASE 8
static unsigned allocations,frees;
static void *VirtualAlloc(void*p,size_t n,int a,int b){(void)p;(void)a;(void)b;if(alloc_fail)return NULL;allocations++;return malloc(n);}
static int VirtualFree(void*p,size_t n,int f){(void)n;(void)f;frees++;free(p);return 1;}
#include "identity58_live.inc"
const unsigned char *fixture59_pattern=player58_bytes;
int main(void){native_text_rva=0x1000;native_text_size=200;identity58_init(0x100000);assert(identity58_verified&&code_reads==1&&allocations==1&&frees==1);code_read_fail=1;identity58_init(0x100000);assert(!identity58_verified&&frees==2);code_read_fail=0;alloc_fail=1;identity58_init(0x100000);assert(!identity58_verified&&code_reads==2);alloc_fail=0;identity58_init(0);assert(!identity58_verified);native_text_size=0;identity58_init(0x100000);assert(!identity58_verified);native_text_size=200;
const uint32_t foreign[4]={9,8,7,6};uint32_t id[4];assert(!player58_get(id)&&alliance_category(imperial53_id)==2);identity58_verified=1;
memcpy(live,imperial53_id,16);memcpy(live+16,foreign,16);identity58_poll(0x10000);assert(!player58_get(id));identity58_poll(0x10000);assert(player58_get(id)&&!memcmp(id,foreign,16));assert(alliance_category(foreign)==1&&alliance_category(imperial53_id)==2&&own_id(imperial53_id));
memcpy(live+16,imperial53_id,16);identity58_poll(0x10000);assert(!player58_get(id));identity58_poll(0x10000);assert(player58_get(id)&&own_id(id)&&alliance_category(imperial53_id)==1);
memset(live+16,0,16);memcpy(live,foreign,16);identity58_poll(0x10000);identity58_poll(0x10000);assert(player58_own(foreign));good=0;identity58_poll(0x10000);assert(!player58_get(id));good=1;torn=1;identity58_poll(0x10000);assert(!player58_get(id));torn=0;memset(live,0,32);identity58_poll(0x10000);identity58_poll(0x10000);assert(!player58_get(id));identity58_verified=0;memcpy(live+16,foreign,16);identity58_poll(0x10000);assert(!player58_get(id));
puts("PASS identity59 startup before hooks: explicit base, copied scan, invalid range/read/allocation refusal and cleanup; identity58: unknown is not imperial, primary GUID over founded, two stable observations, player-vs-emblem independence, account change invalidates old identity, founder fallback, read/torn/signature failure safely clears.");}
