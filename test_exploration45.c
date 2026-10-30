#include <assert.h>
#include <math.h>
#include "exploration45.h"
#include "catalogue_management.h"
#include "catalogue_ui.h"
#include "pirate_style.h"
static Ex45Store data,copy;static Management management;static Ui42Data ui;
int main(void){uint64_t ua=0x00004400a37323b6;assert(ex45_put(&data,ua,0,1,"Sistema pirata",2));assert(ex45_put(&data,ua,2,-1,"Planeta español Ñ",3));assert(!ex45_put(&data,ua,2,-1,"Nombre generado",1));assert(!strcmp(data.items[1].name,"Planeta español Ñ"));assert(ex45_put(&data,ua,0,0,NULL,0));assert(data.items[0].pirate==0&&data.items[1].pirate==-1);
 for(size_t i=0;i<data.count;i++){char line[400];Ex45Record r;assert(ex45_format(&data.items[i],line)&&ex45_parse(line,&r));assert(ex45_put(&copy,r.ua,r.planet,r.pirate,r.name,r.priority));assert(!strcmp(copy.items[i].name,data.items[i].name));}Ex45Record r;assert(!ex45_parse("00004400a37323b6\t2\t8\t1\tff",&r));
 unsigned planet=99;for(int location=3;location<=8;location++){assert(ex45_favourite_target(location,2,1,&planet)&&planet==2);}for(int location=1;location<=15;location++){if(location>=3&&location<=8)continue;assert(ex45_favourite_target(location,2,1,&planet)&&planet==0);}assert(!ex45_favourite_target(0,2,1,&planet));assert(!ex45_favourite_target(3,2,2,&planet));assert(!ex45_favourite_target(3,0,-1,&planet));
 assert(managed_action(&management,ua,5,0)&&managed_action(&management,ua,5,0));assert(management.entries[0].flags==1);assert(managed_action(&management,ua,6,2)&&managed_action(&management,ua,6,2));assert(management.entries[0].planets==4);char glyphs[13];portal_address(ua,2,glyphs);assert(!strcmp(glyphs,"2044A37323B6"));portal_address(ua,0,glyphs);assert(glyphs[0]=='1');
 Ui42System *s=&ui.systems[ui.ns++];s->ua=ua;s->flags=1;s->planets=4;snprintf(s->name,128,"Sistema conocido");snprintf(s->planet_names[2],128,"Planeta español Ñ");Ui42State state={.tab=1};Ui42Frame f;ui42_make(&ui,&state,&f,0);int named=0;for(size_t i=0;i<f.count;i++)if(f.items[i].kind==U42_SYSTEM_CARD&&f.items[i].planet==2){assert(strstr(f.items[i].text,"Planeta español Ñ / Sistema conocido"));assert(f.items[i].portal[0]=='2');named++;}assert(named);
 state.tab=2;ui.show_pirates=1;ui42_make(&ui,&state,&f,0);int toggle=0;for(size_t i=0;i<f.count;i++)if(f.items[i].action==U45_PIRATES){assert(strstr(f.items[i].text,"visibles"));toggle++;}assert(toggle==1);
 assert(PIRATE45_RATIO==.25f);unsigned pixels=0;for(unsigned y=0;y<9;y++){assert(pirate45_rows[y]<512);for(unsigned x=0;x<9;x++)pixels+=(pirate45_rows[y]>>x)&1;}assert(pixels>30&&pixels<70);
 puts("PASS v045 UTF8 planet/system name persistence, name priority, explicit pirate tri-state, planet-vs-space validation, idempotent F8 additions, verified capital glyphs, named planet favourites, pirate settings and skull 25% linear size. No NMS execution.");}
