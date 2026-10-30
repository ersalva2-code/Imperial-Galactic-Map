#include "player58_pattern.h"
static int identity58_verified;static uint64_t identity58_state;static uint32_t identity58_pending[4];static unsigned identity58_ticks;
static void identity58_reset(void){player58_set(NULL);identity58_state=0;identity58_ticks=0;memset(identity58_pending,0,16);}
/* Called BEFORE map hooks: do not depend on map_render's image_base global.
   Copy through the checked reader; a failed/changed guard disables identity only. */
static void identity58_init(uint64_t base){
 identity58_verified=0;identity58_reset();size_t hit=0;int variant=0;
 if(!base||!native_text_rva||!native_text_size||native_text_size>96u*1024u*1024u||base>UINT64_MAX-(uint64_t)native_text_rva-native_text_size){log_line("IDENTITY59 REFUSED invalid image/code span; identity remains unknown");return;}
 unsigned char *copy=VirtualAlloc(NULL,native_text_size,MEM_COMMIT|MEM_RESERVE,PAGE_READWRITE);
 if(!copy){log_line("IDENTITY59 REFUSED scan allocation; identity remains unknown");return;}
 NamedPattern pattern={"MyAlliances_loader",{{player58_bytes,player58_mask,sizeof(player58_bytes)},{player58_bytes,player58_mask,sizeof(player58_bytes)}}};
 if(read_self(base+native_text_rva,copy,native_text_size))identity58_verified=scan_named(copy,native_text_size,&pattern,&hit,&variant);
 VirtualFree(copy,0,MEM_RELEASE);
 log_line("IDENTITY59 guard=%d loader=%llx primary=manager+154 founded=manager+144 (checked copy, no hook-base dependency)",identity58_verified,identity58_verified?(unsigned long long)(native_text_rva+hit):0);
}
static void identity58_name(uint64_t state,const uint32_t id[4]){uint64_t root=state+native.alliance_offset,pointer=0;uint32_t count=0;if(!read_self(root+0x14,&count,4)||count>4096||!read_self(root+0x18,&pointer,8)||pointer<0x10000)return;for(uint32_t i=0;i<count;i++){unsigned char row[168],twice[168];uint64_t at=pointer+(uint64_t)i*168;if(!read_self(at,row,168)||memcmp(row,id,16))continue;if(!read_self(at,twice,168)||memcmp(row,twice,168))return;char name[128];memcpy(name,row+17,127);name[127]=0;if(memchr(row+17,0,127)&&name[0])remember_name(id,name);return;}}
static void identity58_poll(uint64_t state){unsigned char a[32],b[32];uint32_t id[4]={0};if(!identity58_verified||!state||!read_self(state+native.alliance_offset+0x144,a,32)||!read_self(state+native.alliance_offset+0x144,b,32)||memcmp(a,b,32)){identity58_reset();return;}
 /* Primary first; founded only when primary is explicitly empty. Never use current territory. */
 memcpy(id,a+16,16);if(!nonzero_id(id))memcpy(id,a,16);if(!nonzero_id(id)){identity58_reset();return;}
 if(state!=identity58_state||memcmp(id,identity58_pending,16)){player58_set(NULL);identity58_state=state;memcpy(identity58_pending,id,16);identity58_ticks=1;return;}
 identity58_name(state,id);if(identity58_ticks++==1){player58_set(id);log_line("IDENTITY58 player=%u_%u_%u_%u imperial=%d stable=2",id[0],id[1],id[2],id[3],own_id(id));}
}
