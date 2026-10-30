#ifndef IGM_EXPLORE45
#define IGM_EXPLORE45
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#define EX45_LIMIT 8192
typedef struct {uint64_t ua;unsigned planet,priority;int pirate;char name[128];} Ex45Record;
typedef struct {Ex45Record items[EX45_LIMIT];size_t count;} Ex45Store;
static int ex45_find(const Ex45Store *s,uint64_t ua,unsigned planet){for(size_t i=0;i<s->count;i++)if(s->items[i].ua==ua&&s->items[i].planet==planet)return (int)i;return -1;}
static int ex45_put(Ex45Store *s,uint64_t ua,unsigned planet,int pirate,const char *name,unsigned priority){if(!ua||planet>15||pirate< -1||pirate>1||priority>3)return 0;int at=ex45_find(s,ua,planet);if(at<0){if(s->count==EX45_LIMIT)return 0;at=(int)s->count++;memset(&s->items[at],0,sizeof(Ex45Record));s->items[at].ua=ua;s->items[at].planet=planet;s->items[at].pirate=-1;}Ex45Record *r=&s->items[at];int changed=0;if(!planet&&pirate>=0&&pirate!=r->pirate){r->pirate=pirate;changed=1;}if(name&&name[0]&&priority>=r->priority&&(strcmp(name,r->name)||priority!=r->priority)){snprintf(r->name,128,"%s",name);r->priority=priority;changed=1;}return changed;}
static int ex45_format(const Ex45Record *r,char out[400]){char hex[257];size_t n=strlen(r->name);for(size_t i=0;i<n;i++)sprintf(hex+2*i,"%02x",(unsigned char)r->name[i]);hex[n*2]=0;return snprintf(out,400,"%016llx\t%u\t%d\t%u\t%s\n",(unsigned long long)r->ua,r->planet,r->pirate,r->priority,hex)>0;}
static int ex45_parse(const char *line,Ex45Record *r){unsigned long long ua;int used=0;memset(r,0,sizeof(*r));if(sscanf(line,"%16llx\t%u\t%d\t%u\t%n",&ua,&r->planet,&r->pirate,&r->priority,&used)!=4||!used||!ua||r->planet>15||r->pirate< -1||r->pirate>1||r->priority>3)return 0;size_t n=strcspn(line+used,"\r\n");if(n>254||n%2)return 0;for(size_t i=0;i<n;i++){char c=line[used+i];if(!((c>='0'&&c<='9')||(c>='A'&&c<='F')||(c>='a'&&c<='f')))return 0;}for(size_t i=0;i<n;i+=2){unsigned b;if(sscanf(line+used+i,"%2x",&b)!=1||!b)return 0;r->name[i/2]=(char)b;}r->ua=ua;return 1;}
/* Stable environment location 3..8 is planetary; space/stations/freighters are system targets. */
static inline int ex45_favourite_target(int location,int address_planet,int nearest,unsigned *planet){if(location<1||location>15)return 0;if(location>=3&&location<=8){if(nearest<0||nearest>14||address_planet!=nearest+1)return 0;*planet=(unsigned)address_planet;}else *planet=0;return 1;}
#endif
