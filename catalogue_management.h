#ifndef IGM_MANAGEMENT
#define IGM_MANAGEMENT
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#define MGMT_LIMIT 4096
typedef struct {uint64_t ua;unsigned flags,planets;char name[128];} ManagedSystem;
typedef struct {ManagedSystem entries[MGMT_LIMIT];size_t count;} Management;
static int managed_find(const Management *m,uint64_t ua){for(size_t i=0;i<m->count;i++)if(m->entries[i].ua==ua)return (int)i;return -1;}
static ManagedSystem *managed_get(Management *m,uint64_t ua){int i=managed_find(m,ua);if(i>=0)return &m->entries[i];if(!ua||m->count>=MGMT_LIMIT)return NULL;ManagedSystem *s=&m->entries[m->count++];memset(s,0,sizeof(*s));s->ua=ua;return s;}
static int managed_action(Management *m,uint64_t ua,int action,unsigned planet){ManagedSystem *s=managed_get(m,ua);if(!s)return 0;if(action==1)s->flags^=1;else if(action==2){s->flags=2;s->planets=0;}else if(action==3)s->flags&=~2u;else if(action==5){s->flags=(s->flags&~2u)|1u;}else if(action==6&&planet>=1&&planet<=15){s->flags&=~2u;s->planets|=1u<<planet;}else if(action==4&&planet>=1&&planet<=15)s->planets^=1u<<planet;else return 0;return 1;}
static int managed_format(const ManagedSystem *s,char line[400]){char hex[257];size_t n=strlen(s->name);for(size_t i=0;i<n;i++)sprintf(hex+i*2,"%02x",(unsigned char)s->name[i]);hex[n*2]=0;return snprintf(line,400,"%016llx\t%u\t%u\t%s\n",(unsigned long long)s->ua,s->flags,s->planets,hex)>0;}
static int managed_parse(const char *line,ManagedSystem *s){unsigned long long ua;unsigned flags,planets;int used=0;memset(s,0,sizeof(*s));if(sscanf(line,"%16llx\t%u\t%u\t%n",&ua,&flags,&planets,&used)!=3||!used||!ua||flags>3||planets>65535)return 0;const char *p=line+used;size_t n=strcspn(p,"\r\n");if(n>254||n%2)return 0;for(size_t k=0;k<n;k++)if(!((p[k]>='0'&&p[k]<='9')||(p[k]>='a'&&p[k]<='f')||(p[k]>='A'&&p[k]<='F')))return 0;for(size_t i=0;i<n;i+=2){unsigned x;if(sscanf(p+i,"%2x",&x)!=1)return 0;s->name[i/2]=(char)x;}s->ua=ua;s->flags=flags;s->planets=planets;return 1;}
static void portal_address(uint64_t ua,unsigned planet,char out[13]){uint64_t portal=((uint64_t)(planet>=1&&planet<=15?planet:1)<<44)|(((ua>>40)&4095)<<32)|(ua&0xffffffffULL);snprintf(out,13,"%012llX",(unsigned long long)portal);}
#endif
