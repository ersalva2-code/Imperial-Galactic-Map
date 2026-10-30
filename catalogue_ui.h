#ifndef IGM_CATALOGUE_UI42
#define IGM_CATALOGUE_UI42
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "catalogue_management.h"
#include "imperial53.h"
#include "language48.h"
#define UI42_LIMIT 4096
#define UI42_ITEMS 64
/* Design-space coordinates are independent of resolution and window position. */
typedef struct {uint32_t id[4];char name[180],initials[7];unsigned char rgb[3];int own;} Ui42Alliance;
typedef struct {uint64_t ua;uint32_t id[4];char name[128],planet_names[16][128];unsigned flags,planets;} Ui42System;
typedef struct {Ui42Alliance alliances[UI42_LIMIT];Ui42System systems[UI42_LIMIT];size_t na,ns;int show_pirates;char player_name[180];} Ui42Data;
typedef struct {int tab,trash,focus;size_t apage,spage;uint32_t alliance[4];uint64_t system,confirmation;unsigned planet;} Ui42State;
enum {U42_ALLIANCE=1,U42_SYSTEM,U42_FAVOURITE,U42_DELETE,U42_CONFIRM,U42_CANCEL,U42_RESTORE,U42_APAGE,U42_SPAGE,U42_TAB,U42_TRASH,U42_VISIBILITY,U42_PLANET,U42_CLOSE,U45_PIRATES,U48_LANGUAGE};
enum {U42_LABEL=0,U42_ALLIANCE_CARD,U42_SYSTEM_CARD,U42_BUTTON,U42_HEADING,U57_PHOTO};
typedef struct {float x,y,w,h;int kind,action,value,selected;uint64_t ua;unsigned planet;uint32_t id[4];char text[400],subtitle[160],portal[13];unsigned char rgb[3];char initials[7];int own;} Ui42Item;
typedef struct {Ui42Item items[UI42_ITEMS];size_t count;int modal;char right[200],left[80];} Ui42Frame;
static Ui42Item *ui42_add(Ui42Frame *f,int kind,float x,float y,float w,float h,const char *text,int action){if(f->count>=UI42_ITEMS)return NULL;Ui42Item *r=&f->items[f->count++];memset(r,0,sizeof(*r));r->kind=kind;r->x=x;r->y=y;r->w=w;r->h=h;r->action=action;snprintf(r->text,sizeof(r->text),"%s",text);return r;}
static int ui42_group(const Ui42Data *d,const uint32_t id[4]){for(size_t i=0;i<d->na;i++)if(!memcmp(d->alliances[i].id,id,16))return (int)i;return -1;}
static int ui42_record(const Ui42Data *d,uint64_t ua){for(size_t i=0;i<d->ns;i++)if(d->systems[i].ua==ua)return (int)i;return -1;}
static void ui42_system_name(const Ui42System *s,unsigned planet,const Ui42Alliance *a,char out[400]){unsigned g=(unsigned)((s->ua>>32)&255);char galaxy[40];if(!g)snprintf(galaxy,40,"Euclides");else if(g==9)snprintf(galaxy,40,"Eissentam");else snprintf(galaxy,40,tr48("Galaxia %u"),g+1);if(planet){char fallback[40];snprintf(fallback,40,tr48("Planeta %u"),planet);snprintf(out,400,"%s / %s (%s)",s->planet_names[planet][0]?s->planet_names[planet]:fallback,s->name[0]?s->name:tr48("Nombre pendiente"),galaxy);}else snprintf(out,400,"%s%s%s%s (%s)",a&&a->initials[0]?"(":"",a?a->initials:"",a&&a->initials[0]?") ":"",s->name[0]?s->name:tr48("Nombre pendiente"),galaxy);}
static void ui42_card(Ui42Frame *f,const Ui42Data *d,const Ui42System *s,unsigned planet,float x,float y,float w,int selected){char name[400];int a=ui42_group(d,s->id);ui42_system_name(s,planet,a>=0?&d->alliances[a]:NULL,name);Ui42Item *r=ui42_add(f,U42_SYSTEM_CARD,x,y,w,78,name,U42_SYSTEM);if(!r)return;r->ua=s->ua;r->planet=planet;r->selected=selected;if(a>=0)snprintf(r->initials,7,"%s",d->alliances[a].initials);portal_address(s->ua,planet,r->portal);snprintf(r->subtitle,160,"%s",(s->flags&1)||(planet&&(s->planets&(1u<<planet)))?tr48("Favorito"):"");}
static void ui42_pages(Ui42Frame *f,int action,float x,float y,size_t page,size_t total,size_t per){if(total<=per)return;Ui42Item *r=ui42_add(f,U42_BUTTON,x,y,135,36,tr48("Anterior"),action);r->value=-1;r=ui42_add(f,U42_BUTTON,x+310,y,135,36,tr48("Siguiente"),action);r->value=1;char s[80];snprintf(s,80,"%zu / %zu",page+1,(total+per-1)/per);ui42_add(f,U42_LABEL,x+166,y+8,120,24,s,0);}
static void ui42_make(Ui42Data *d,Ui42State *s,Ui42Frame *f,int visibility){
 /* Stable capital-first ordering; do not reorder the binary-search core table. */
 for(size_t i=1;i<d->ns;i++)if(d->systems[i].ua==UINT64_C(0x00004400a37323b6)){Ui42System capital=d->systems[i];memmove(d->systems+1,d->systems,i*sizeof(capital));d->systems[0]=capital;break;}
 memset(f,0,sizeof(*f));snprintf(f->left,80,"%s",s->tab==0?tr48("CATÁLOGO"):s->tab==1?tr48("FAVORITOS"):tr48("AJUSTES"));
 int group=ui42_group(d,s->alliance);if(group<0&&d->na){group=0;memcpy(s->alliance,d->alliances[0].id,16);s->system=0;s->planet=0;s->spage=0;}
 if(s->tab==0){
  if(s->apage*5>=d->na)s->apage=0;
  for(size_t i=s->apage*5;i<d->na&&i<s->apage*5+5;i++){const Ui42Alliance *a=&d->alliances[i];Ui42Item *r=ui42_add(f,U42_ALLIANCE_CARD,105,270+(float)(i%5)*86,610,74,a->name,U42_ALLIANCE);memcpy(r->id,a->id,16);memcpy(r->rgb,a->rgb,3);snprintf(r->initials,7,"%s",a->initials);r->own=a->own;r->selected=(int)i==group;size_t count=0;for(size_t k=0;k<d->ns;k++)if(!(d->systems[k].flags&2)&&!memcmp(a->id,d->systems[k].id,16))count++;snprintf(r->subtitle,160,tr48("%zu sistemas conocidos"),count);}
  ui42_pages(f,U42_APAGE,105,737,s->apage,d->na,5);
  if(group>=0)snprintf(f->right,200,"%s",d->alliances[group].name);else snprintf(f->right,200,"%s",tr48("Sin alianzas observadas"));
  size_t n=0;for(size_t i=0;i<d->ns;i++)if(!(d->systems[i].flags&2)&&!memcmp(d->systems[i].id,s->alliance,16))n++;
  if(s->spage*4>=n)s->spage=0;
  size_t at=0;int selected=0;for(size_t i=0;i<d->ns;i++){const Ui42System *r=&d->systems[i];if(r->flags&2||memcmp(r->id,s->alliance,16))continue;if(r->ua==s->system)selected=1;if(!s->system)s->system=r->ua;if(at>=s->spage*4&&at<s->spage*4+4)ui42_card(f,d,r,0,790,294+(float)(at%4)*86,710,r->ua==s->system);at++;}
  if(!n)ui42_add(f,U42_LABEL,810,330,650,45,tr48("No hay sistemas visibles en esta alianza."),0);
  if(!selected&&s->system&&ui42_record(d,s->system)<0)s->system=0;
  ui42_pages(f,U42_SPAGE,790,737,s->spage,n,4);
 }else if(s->tab==1){
  snprintf(f->right,200,"%s",tr48("Sistema favorito"));size_t total=0;for(size_t i=0;i<d->ns;i++){const Ui42System *r=&d->systems[i];if(r->flags&2)continue;for(unsigned p=0;p<16;p++){if(p?!(r->planets&(1u<<p)):!(r->flags&1))continue;if(total>=s->apage*5&&total<s->apage*5+5)ui42_card(f,d,r,p,105,270+(float)(total%5)*86,610,r->ua==s->system&&p==s->planet);total++;}}
  if(s->apage*5>=total)s->apage=0;
  if(!total)ui42_add(f,U42_LABEL,125,300,550,50,tr48("Todavía no has guardado favoritos."),0);
  ui42_pages(f,U42_APAGE,105,737,s->apage,total,5);
  int index=ui42_record(d,s->system);if(index>=0){ui42_card(f,d,&d->systems[index],s->planet,790,294,710,1);Ui42Item *photo=ui42_add(f,U57_PHOTO,802,392,686,274,"",0);photo->ua=s->system;photo->planet=s->planet;}
 }else{
  Ui42Item *r=ui42_add(f,U42_ALLIANCE_CARD,105,270,610,74,tr48("Visibilidad de marcadores"),U42_TRASH);r->value=0;r->selected=!s->trash;
  r=ui42_add(f,U42_ALLIANCE_CARD,105,356,610,74,tr48("Entradas eliminadas"),U42_TRASH);r->value=1;r->selected=s->trash;
  ui42_add(f,U42_LABEL,105,447,610,28,tr48("Idioma del mod"),0);for(int l=0;l<6;l++){r=ui42_add(f,U42_BUTTON,105,480+l*40,610,34,language48_names[l],U48_LANGUAGE);r->value=l;r->selected=atomic_load(&language48)==l;}
  if(!s->trash){ui42_add(f,U42_LABEL,790,247,710,30,d->player_name[0]?d->player_name:tr48("Alianza propia sin identificar"),0);snprintf(f->right,200,"%s",tr48("Visibilidad en el mapa"));const char *labels[]={ (visibility==0||visibility==1)?tr48("Mi alianza: visible"):tr48("Mi alianza: oculta"),(visibility==0||visibility==2)?tr48("Otras alianzas: visibles"):tr48("Otras alianzas: ocultas")};for(int i=0;i<2;i++){r=ui42_add(f,U42_BUTTON,790,294+i*90,710,58,labels[i],U42_VISIBILITY);r->value=i==0?(visibility==0?2:visibility==1?3:visibility==2?0:1):(visibility==0?1:visibility==1?0:visibility==2?3:2);r->selected=i==0?(visibility==0||visibility==1):(visibility==0||visibility==2);}r=ui42_add(f,U42_BUTTON,790,474,710,58,d->show_pirates?tr48("Sistemas piratas: visibles"):tr48("Sistemas piratas: ocultos"),U45_PIRATES);r->selected=d->show_pirates;}
  else{snprintf(f->right,200,"%s",tr48("Restaurar sistemas"));size_t at=0;for(size_t i=0;i<d->ns;i++){if(!(d->systems[i].flags&2))continue;if(at>=s->spage*4&&at<s->spage*4+4)ui42_card(f,d,&d->systems[i],0,790,294+(float)(at%4)*86,710,d->systems[i].ua==s->system);at++;}if(!at)ui42_add(f,U42_LABEL,810,330,650,45,tr48("No hay entradas eliminadas."),0);ui42_pages(f,U42_SPAGE,790,737,s->spage,at,4);}
 }
 float bottom=372;int visible_cards=0;for(size_t i=0;i<f->count;i++)if(f->items[i].kind==U42_SYSTEM_CARD&&f->items[i].x>=790){float b=f->items[i].y+f->items[i].h;if(b>bottom)bottom=b;visible_cards++;}
 float action_y=bottom+28;if(action_y<410)action_y=410;if(action_y>650)action_y=650;
 if(s->tab==1)action_y=686;
 int index=ui42_record(d,s->system);
 if(index>=0&&(s->tab<2||s->trash)){
  const Ui42System *r=&d->systems[index];
  if(s->tab==2){Ui42Item *b=ui42_add(f,U42_BUTTON,790,action_y,710,48,tr48("Restaurar este sistema"),U42_RESTORE);b->ua=s->system;}
  else if(!(r->flags&2)){
   int fav=s->planet?(r->planets&(1u<<s->planet)):(r->flags&1);
   Ui42Item *b=ui42_add(f,U42_BUTTON,790,action_y,710,s->tab==1?40:visible_cards>=4?43:52,fav?tr48("Quitar de favoritos"):tr48("Añadir a favoritos"),U42_FAVOURITE);b->ua=s->system;b->planet=s->planet;
   if(!imperial53_find(s->system)){b=ui42_add(f,U42_BUTTON,790,action_y+(s->tab==1?52:visible_cards>=4?49:62),710,s->tab==1?40:visible_cards>=4?32:44,tr48("Eliminar del catálogo"),U42_DELETE);b->ua=s->system;}
  }
 }
 const char *tabs[]={tr48("Alianzas"),tr48("Favoritos"),tr48("Ajustes")};for(int i=0;i<3;i++){Ui42Item *r=ui42_add(f,U42_BUTTON,105+i*215,818,200,40,tabs[i],U42_TAB);r->value=i;r->selected=s->tab==i;}
 ui42_add(f,U42_BUTTON,1280,818,220,40,tr48("Volver a Opciones"),U42_CLOSE);
 if(s->confirmation){f->modal=1;for(size_t i=0;i<f->count;i++)f->items[i].action=0;ui42_add(f,U42_HEADING,450,367,700,45,tr48("¿Eliminar este sistema del catálogo?"),0);ui42_add(f,U42_LABEL,450,422,700,48,tr48("Podrás recuperarlo desde Ajustes / Entradas eliminadas."),0);Ui42Item *r=ui42_add(f,U42_BUTTON,450,493,300,52,tr48("Cancelar"),U42_CANCEL);r->selected=1;r=ui42_add(f,U42_BUTTON,835,493,300,52,tr48("Confirmar eliminación"),U42_CONFIRM);r->ua=s->confirmation;}
}
static int ui42_hit(const Ui42Frame *f,float x,float y){for(size_t k=f->count;k>0;k--){const Ui42Item *r=&f->items[k-1];if(r->action&&x>=r->x&&y>=r->y&&x<r->x+r->w&&y<r->y+r->h)return (int)k-1;}return -1;}
static int ui42_next(const Ui42Frame *f,int current,int direction){if(!f->count)return -1;for(size_t k=1;k<=f->count;k++){int i=(current+(int)f->count+direction*(int)k)%(int)f->count;if(i<0)i+=(int)f->count;if(f->items[i].action)return i;}return -1;}
static int ui42_focus_direction(const Ui42Frame *f,int current,int dx,int dy){if(current<0||(size_t)current>=f->count)return ui42_next(f,-1,1);const Ui42Item *a=&f->items[current];float x=a->x+a->w/2,y=a->y+a->h/2,best=1e30f;int found=current;for(size_t i=0;i<f->count;i++){const Ui42Item *r=&f->items[i];if(!r->action||(int)i==current)continue;float xx=r->x+r->w/2-x,yy=r->y+r->h/2-y,forward=dx?xx*dx:yy*dy,side=dx?yy:xx;if(forward<=1)continue;float score=forward*forward+side*side*4;if(score<best){best=score;found=(int)i;}}return found;}
#endif
