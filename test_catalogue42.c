/* Production draw+input harness. Native APIs are stubs; not an NMS execution. */
#include <assert.h>
#include <math.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdatomic.h>
#include "core.h"
#include "map_catalogue.h"
typedef IgmRecord Seen;
#include "catalogue_management.h"
#include "path_batch.h"
#include "catalogue_ui.h"
typedef unsigned long DWORD;typedef unsigned long long ULONGLONG;typedef void *HWND;typedef int SRWLOCK;
typedef struct {long x,y;} POINT;typedef struct {long left,top,right,bottom;} RECT;
#define VK_LBUTTON 1
#define VK_RETURN 13
#define VK_TAB 9
#define VK_SHIFT 16
#define VK_UP 38
#define VK_DOWN 40
#define VK_LEFT 37
#define VK_RIGHT 39
static unsigned char statebytes[264];static int keys[256],menu_mode=1,root39_owned_page=1,catalogue42_design=1,drawing,panel_font_index,root_valid=1;
static float marker_alpha=.73f,rgba[4];static unsigned native_text_count,native_fill_count;
static struct {uint32_t singleton,vr_offset,viewport_offset;} native={1,2,3};static uint64_t image_base=100;
static struct {int visibility;} panel;
static ULONGLONG now=20000;static POINT mouse={0};static int management_lock;
static Management management;static Seen known[3];static size_t known_count=3;
static FILE *svg;static unsigned svg_id;static int yellow_prefix45;
static float viewport_size[2]={1600,900};static long client_w=1600,client_h=900,window_left;
static int read_self(uint64_t at,void *dst,size_t n){memset(dst,0,n);if(at==101&&n==8){*(uint64_t*)dst=10000;return 1;}if(at==10002&&n==1)return 1;if(at==10000+0xc1a0&&n==sizeof(Address)){Address a={2,82,950,-93,1842,0};memcpy(dst,&a,n);return 1;}if(at==0x30000+0x42c8&&n==8){*(uint64_t*)dst=0x20000;return 1;}if(at==0x20000+0x74&&n==4){*(int*)dst=3;return 1;}return 0;}
static int native_root(void){return root_valid;}
static ULONGLONG GetTickCount64(void){return now;}
static HWND GetForegroundWindow(void){return (HWND)1;}
static void GetWindowThreadProcessId(HWND h,DWORD *p){(void)h;*p=1;}
static DWORD GetCurrentProcessId(void){return 1;}
static int GetCursorPos(POINT *p){*p=mouse;p->x+=window_left;return 1;}
static int ScreenToClient(HWND h,POINT *p){(void)h;p->x-=window_left;return 1;}
static int GetClientRect(HWND h,RECT *r){(void)h;*r=(RECT){0,0,client_w,client_h};return 1;}
static int GetAsyncKeyState(int k){return keys[k]?0x8000:0;}
static DWORD XInputGetState(DWORD i,void *p){(void)i;(void)p;return 1;}
static void get_viewport(void *p,float out[2]){(void)p;out[0]=viewport_size[0];out[1]=viewport_size[1];}
static void AcquireSRWLockShared(SRWLOCK *p){(void)p;}static void ReleaseSRWLockShared(SRWLOCK *p){(void)p;}
static size_t catalogue_snapshot(Seen out[4096]){memcpy(out,known,known_count*sizeof(*known));return known_count;}
static ManagedSystem metadata(uint64_t ua){ManagedSystem m={0};int i=managed_find(&management,ua);if(i>=0)m=management.entries[i];return m;}
static unsigned writes;
static int management_change(uint64_t ua,int action,unsigned planet,const char *name){if(name){ManagedSystem *m=managed_get(&management,ua);snprintf(m->name,128,"%s",name);return 1;}writes++;return managed_action(&management,ua,action,planet);}
static void get_initials(const uint32_t id[4],char out[7]){snprintf(out,7,"%s",own_id(id)?"IESP":id[0]==9?"RSSH":"APF");}
static void alliance_label(const uint32_t id[4],char out[180]){snprintf(out,180,"%s",own_id(id)?"Imperio Español":id[0]==9?"Royal Space Society Hub (RSSH)":"Aurelis Prime Federation (APF)");}
static int get_alliance_color(const uint32_t id[4],unsigned char rgb[3],uint32_t *slot){(void)slot;memcpy(rgb,id[0]==9?(unsigned char[]){40,122,235}:(unsigned char[]){225,61,66},3);return 1;}
static void navigate(int mode){menu_mode=mode;}
static void log_line(const char *f,...){(void)f;}
static void set_float(unsigned char *p,float v){memcpy(p,&v,4);}
static void color(unsigned char *s,float r,float g,float b,float a){(void)s;rgba[0]=r;rgba[1]=g;rgba[2]=b;rgba[3]=a;}
static int valid_context(void *ctx,unsigned char **state){(void)ctx;*state=statebytes;return 1;}
static void svg_poly(const float *xy,int n){if(!svg)return;float t[6];memcpy(t,statebytes+0xb4,24);fprintf(svg,"<polygon fill='rgb(%d,%d,%d)' fill-opacity='%.3f' points='",(int)(rgba[0]*255),(int)(rgba[1]*255),(int)(rgba[2]*255),rgba[3]);for(int i=0;i<n;i++)fprintf(svg,"%.3f,%.3f ",t[0]*xy[i*2]+t[4],t[3]*xy[i*2+1]+t[5]);fputs("'/>\n",svg);}
static void polygon(void *ctx,const float *xy,int n){(void)ctx;native_fill_count++;svg_poly(xy,n);}
static void ui_rect(void *ctx,unsigned char *s,float x,float y,float w,float h,float r,float g,float b,float a){color(s,r,g,b,a);float xy[8]={x,y,x+w,y,x+w,y+h,x,y+h};polygon(ctx,xy,4);}
static void draw_cross(void *ctx,unsigned char *s,float x,float y){float q[]={x,y-13,x+13,y,x,y+13,x-13,y};color(s,.85f,.85f,.88f,1);polygon(ctx,q,4);float white[]={x,y-11.5f,x+11.5f,y,x,y+11.5f,x-11.5f,y};color(s,1,1,1,1);polygon(ctx,white,4);color(s,.8f,.05f,.13f,1);float a[]={x-6,y-8,x+8,y+6,x+6,y+8,x-8,y-6},b[]={x-8,y+6,x+6,y-8,x+8,y-6,x-6,y+8};polygon(ctx,a,4);polygon(ctx,b,4);}
static void draw_favourite(void *ctx,unsigned char *s,float x,float y){float p[20];for(int i=0;i<10;i++){float a=i*.62831853f-1.5707963f,r=i%2?3:7;p[i*2]=x+cosf(a)*r;p[i*2+1]=y+sinf(a)*r;}color(s,1,.77f,.1f,1);polygon(ctx,p,10);}
static void submit_batch(void *ctx,IgmPathBatch *b){(void)ctx;if(!b->count)return;native_fill_count++;float q[24];int n=0;for(size_t i=0;i<b->count;){int op=(int)b->commands[i++];if(op==0||op==1){assert(i+2<=b->count&&n<12);q[n*2]=b->commands[i++];q[n*2+1]=b->commands[i++];n++;}else {assert(op==3&&n>=3);svg_poly(q,n);n=0;}}assert(!n);b->count=0;}
static void escaped(FILE *f,const char *s){for(;*s;s++)if(*s=='&')fputs("&amp;",f);else if(*s=='<')fputs("&lt;",f);else if(*s=='>')fputs("&gt;",f);else fputc(*s,f);}
static float mock_text(void *ctx,float x,float y,const char *text,const char *end){(void)ctx;(void)end;assert(!strstr(text,"<IMG>"));if(!strcmp(text,"(IESP) ")){assert(fabsf(rgba[0]-.98f)<.001f&&fabsf(rgba[1]-.85f)<.001f&&fabsf(rgba[2]-.16f)<.001f);yellow_prefix45=1;}if(svg){float size,t[6],sc[8];memcpy(&size,statebytes+0xf0,4);memcpy(t,statebytes+0xb4,24);memcpy(sc,statebytes+0xcc,32);unsigned id=svg_id++;fprintf(svg,"<defs><clipPath id='c%u'><rect x='%.2f' y='%.2f' width='%.2f' height='%.2f'/></clipPath></defs>",id,sc[4]-sc[6]*t[0],sc[5]-sc[7]*t[3],sc[6]*2*t[0],sc[7]*2*t[3]);fprintf(svg,"<text clip-path='url(#c%u)' dominant-baseline='hanging' font-family='Arial,sans-serif' font-weight='300' x='%.2f' y='%.2f' font-size='%.2f' fill='rgb(%d,%d,%d)'>",id,x*t[0]+t[4],y*t[3]+t[5],size*t[0],(int)(rgba[0]*255),(int)(rgba[1]*255),(int)(rgba[2]*255));escaped(svg,text);fputs("</text>\n",svg);}return x;}
static float (*native_text)(void*,float,float,const char*,const char*)=mock_text;
#define IGM_CATALOGUE_HOST_TEST 1
#include "catalogue42.inc"
static void frame(void){unsigned char before[264];memcpy(before,statebytes,264);now+=16;catalogue42_signal();assert(catalogue42_draw((void*)0x30000));assert(!memcmp(before,statebytes,264)&&marker_alpha==.73f&&!drawing);unsigned fills=native_fill_count,texts=native_text_count;assert(!catalogue42_draw((void*)0x30000));assert(fills==native_fill_count&&texts==native_text_count);}
static int find(int action,int value){for(size_t i=0;i<catalogue42_frame.count;i++)if(catalogue42_frame.items[i].action==action&&(value<0||catalogue42_frame.items[i].value==value))return (int)i;return -1;}
static void click_action(int action,int value){int i=find(action,value);assert(i>=0);Ui42Item r=catalogue42_frame.items[i];mouse.x=(long)(r.x+r.w/2);mouse.y=(long)(r.y+r.h/2);keys[1]=1;frame();keys[1]=0;frame();}
int main(int argc,char **argv){
 uint32_t own[4]={275889132,1121173938,1505499062,4230148680u};known[0]=(Seen){0x00005200a37323b6,{0}};memcpy(known[0].id,own,16);known[1]=(Seen){0x0000b400fb004005,{9,2,3,4}};known[2]=(Seen){0x0000ad00fb004005,{10,2,3,4}};
 management_change(known[0].ua,0,0,"Territorio imperial español - Minería II");management_change(known[1].ua,0,0,"Nenyosmi");management_change(known[2].ua,0,0,"Daeleo");
 memset(statebytes,0x5a,sizeof(statebytes));catalogue42_reset();frame();assert(catalogue42_data.na==3&&catalogue42_data.alliances[0].own&&!strcmp(catalogue42_data.alliances[0].initials,"IESP"));assert(native_fill_count<100&&native_text_count<40&&yellow_prefix45);
 if(argc>1){svg=fopen(argv[1],"w");assert(svg);fputs("<svg xmlns='http://www.w3.org/2000/svg' width='1600' height='900' viewBox='0 0 1600 900'><defs><radialGradient id='bg'><stop stop-color='#583158'/><stop offset='1' stop-color='#090c16'/></radialGradient></defs><rect width='1600' height='900' fill='url(#bg)'/><text x='70' y='40' fill='#ddd' font-family='Arial' font-size='16'>HOST PREVIEW · NO EJECUTADO EN NMS</text>",svg);frame();fputs("</svg>",svg);fclose(svg);svg=NULL;}
 viewport_size[0]=3840;viewport_size[1]=2160;client_w=1920;client_h=1080;mouse=(POINT){600,468};keys[1]=1;frame();keys[1]=0;frame();assert(catalogue42_state.alliance[0]==9);
 memcpy(catalogue42_state.alliance,own,16);catalogue42_state.system=0;viewport_size[0]=5120;client_w=2560;window_left=3840;frame();mouse=(POINT){920,468};keys[1]=1;frame();keys[1]=0;frame();assert(catalogue42_state.alliance[0]==9);
 viewport_size[0]=1600;viewport_size[1]=900;client_w=1600;client_h=900;window_left=0;frame();
 puts("PASS catalogue pointer mapping: 4K rendering with scaled client; ultrawide letterbox; window on second monitor");
 int i=-1;for(size_t k=0;k<catalogue42_frame.count;k++)if(catalogue42_frame.items[k].action==U42_ALLIANCE&&catalogue42_frame.items[k].id[0]==9)i=(int)k;assert(i>=0);Ui42Item r=catalogue42_frame.items[i];mouse.x=(long)(r.x+250);mouse.y=(long)(r.y+30);keys[1]=1;frame();keys[1]=0;frame();assert(catalogue42_state.alliance[0]==9&&catalogue42_state.system==known[1].ua);
 click_action(U42_FAVOURITE,-1);assert(metadata(known[1].ua).flags&1);click_action(U42_TAB,1);assert(find(U42_SYSTEM,-1)>=0);click_action(U42_SYSTEM,-1);click_action(U42_DELETE,-1);assert(catalogue42_frame.modal&&!(metadata(known[1].ua).flags&2));assert(ui42_hit(&catalogue42_frame,110,280)==-1);click_action(U42_CONFIRM,-1);assert(metadata(known[1].ua).flags&2);
 click_action(U42_TAB,2);click_action(U42_TRASH,1);click_action(U42_SYSTEM,-1);click_action(U42_RESTORE,-1);assert(!(metadata(known[1].ua).flags&2)&&known_count==3);
 click_action(U42_TRASH,0);click_action(U42_VISIBILITY,2);assert(panel.visibility==2);
 click_action(U42_TAB,1);click_action(U42_PLANET,-1);assert(metadata(known[0].ua).planets&4);assert(writes==4);
 unsigned fills=native_fill_count;root_valid=0;catalogue42_signal();assert(!catalogue42_draw((void*)0x30000)&&fills==native_fill_count);root_valid=1;
 static Ui42Data many;many=catalogue42_data;for(unsigned k=0;k<7;k++){Ui42System *r=&many.systems[many.ns++];*r=many.systems[0];r->ua+=k+1;r->flags=r->planets=0;snprintf(r->name,128,"Sistema de prueba %u",k);}
 Ui42State paged={0};memcpy(paged.alliance,own,16);Ui42Frame pf;ui42_make(&many,&paged,&pf,0);unsigned cards=0;float end=0;for(size_t k=0;k<pf.count;k++){Ui42Item *r=&pf.items[k];if(r->kind==U42_SYSTEM_CARD&&r->x>=790){cards++;assert(strlen(r->portal)==12&&!strstr(r->text,"<IMG>"));if(r->y+r->h>end)end=r->y+r->h;}if(r->action==U42_DELETE)assert(r->y+r->h<=737);}assert(cards==4&&end==630);paged.spage=1;ui42_make(&many,&paged,&pf,0);assert(paged.spage==1);paged.spage=2;ui42_make(&many,&paged,&pf,0);assert(!paged.spage);
 puts("PASS full system pages: four cards without action/footer overlap, second page and out-of-range recovery");
 catalogue42_reset();catalogue42_signal();assert(catalogue42_offer());now+=1100;assert(!catalogue42_offer()&&atomic_load(&catalogue42_failed));catalogue42_reset();assert(catalogue42_offer());
 puts("PASS native-list fallback after no valid renderer; reset permits retry");
 for(int g=0;g<16;g++){assert(portal42_offsets[g]<portal42_offsets[g+1]);for(unsigned k=portal42_offsets[g];k<portal42_offsets[g+1];k++)for(int j=0;j<6;j++)assert(isfinite(portal42_triangles[k][j])&&portal42_triangles[k][j]>=0&&portal42_triangles[k][j]<=1);}
 for(int g=0;g<16;g++){float lo[2]={1,1},hi[2]={0,0};for(unsigned k=portal42_offsets[g];k<portal42_offsets[g+1];k++)for(int j=0;j<6;j++){float v=portal42_triangles[k][j];if(v<lo[j%2])lo[j%2]=v;if(v>hi[j%2])hi[j%2]=v;}assert(fabsf(hi[0]-lo[0]-.92f)<.00001f&&fabsf(hi[1]-lo[1]-.92f)<.00001f);}
 puts("PASS v044 all sixteen glyphs retain uniform visible bounding boxes");
 puts("PASS production catalogue42 draw/input: two columns, IESP first, names separate from 12 glyph symbols, alliance selection, favourites, modal deletion, restore, visibility, planet favourite, native state restore, no draw outside owned page, exactly one submission per UI frame; 16 bounded glyph shapes. Native APIs stubbed; NMS not executed.");
}
