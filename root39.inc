#include "root39_patterns.h"
#include <math.h>
typedef void (*RootPageFn)(void*);
typedef unsigned char (*RootClickFn)(void*,void*,unsigned char,unsigned char,const void*,unsigned char,unsigned char);
typedef void *(*NodeCloneFn)(void*);
typedef void *(*NodeAddFn)(void*,void*,unsigned char);
typedef void (*RequestPageFn)(int,unsigned char);
typedef void (*PopPageFn)(unsigned char);
typedef void *(*Root42AssignFn)(void*,const char*);
static Root42AssignFn root42_assign;
static RootPageFn root39_original;
static RootClickFn root39_click_original;
static NodeCloneFn root39_clone;
static NodeAddFn root39_add;
static RequestPageFn root39_request;
static PopPageFn root39_pop;
static _Thread_local int root39_scope;
static _Thread_local uint64_t root39_template,root39_buttons[32];
static _Thread_local unsigned root39_button_count;
static uint64_t root39_refused_parent;
static unsigned root39_attempts,root39_traces;
static _Atomic unsigned root39_calls,root39_last_buttons,root39_label_calls;
static int root39_logged_buttons;
static _Atomic unsigned root43_label_attempts;
static _Atomic int root39_owned_page;
static int root39_pointer(uint64_t p){
#ifdef IGM_ROOT_MODEL_ONLY
 return p!=0;
#else
 return p>=image_base&&p<image_base+0x10000000;
#endif
}
static int root39_type(uint64_t node){uint64_t v=0,fn=0;if(!read_self(node,&v,8)||!read_self(v+8,&fn,8)||!root39_pointer(fn))return -1;return ((int(*)(void*))(uintptr_t)fn)((void*)(uintptr_t)node);}
static int root39_children(uint64_t node,uint64_t out[256],unsigned *n){uint32_t cap=0,count=0;uint64_t a=0;if(!read_self(node+0x58,&cap,4)||!read_self(node+0x5c,&count,4)||count>cap||count>256||!read_self(node+0x60,&a,8)||(count&&!a))return 0;*n=count;return !count||read_self(a,out,count*8);}
static int root39_private(uint64_t original,uint64_t clone,unsigned depth){
 uint64_t om=0,cm=0;if(depth>8||original==clone||!read_self(original+0x48,&om,8)||!read_self(clone+0x48,&cm,8)||!om||!cm||om==cm)return 0;
 int type=root39_type(original);if(type<0||type!=root39_type(clone))return 0;
 if(type==0){uint64_t a[256],b[256];unsigned na=0,nb=0;if(!root39_children(original,a,&na)||!root39_children(clone,b,&nb)||na!=nb)return 0;for(unsigned i=0;i<na;i++)if(!root39_private(a[i],b[i],depth+1))return 0;}
 return 1;
}
static void root41_texts(uint64_t node,unsigned depth,unsigned *nearest,uint64_t out[64],unsigned *n){
 (void)nearest;if(depth>8||*n>=64)return;int type=root39_type(node);
 if(type==1||type==2){out[(*n)++]=node;return;}
 if(type!=0)return;
 uint64_t children[256];unsigned count=0;if(!root39_children(node,children,&count))return;
 for(unsigned i=0;i<count;i++)root41_texts(children[i],depth+1,nearest,out,n);
}
static void root41_dump(uint64_t node,unsigned depth,unsigned *budget){
 if(!*budget||depth>8)return;
 (*budget)--;
 uint64_t m=0;char id[17]={0};if(read_self(node+0x48,&m,8)&&m)read_self(m+0x48,id,16);
 for(unsigned i=0;i<16&&id[i];i++)if((unsigned char)id[i]<32||(unsigned char)id[i]>126)id[i]='?';
 int type=root39_type(node);log_line("MENU42 LABEL TREE depth=%u type=%d id=%s",depth,type,id);
 if(type==0){uint64_t a[256];unsigned n=0;if(root39_children(node,a,&n))for(unsigned i=0;i<n&&*budget;i++)root41_dump(a[i],depth+1,budget);}
}
static unsigned root41_label(uint64_t row,uint64_t target){
 uint64_t nodes[64],functions[64];unsigned n=0,nearest=9;int fallback=1;
 (void)target;root41_texts(row,0,&nearest,nodes,&n);
 if(atomic_fetch_add(&root43_label_attempts,1)<3)log_line("MENU43 LABEL CANDIDATES count=%u depth=%u row_fallback=%d",n,nearest,fallback);
 if(!n||n>=64){unsigned budget=24;root41_dump(row,0,&budget);return 0;}
 for(unsigned i=0;i<n;i++){uint64_t v=0;if(!read_self(nodes[i],&v,8)||!read_self(v+0x90,&functions[i],8)||!root39_pointer(functions[i]))return 0;}
 for(unsigned i=0;i<n;i++){
#if !defined(IGM_MENU_ROUTING_TEST)
  uint64_t m=0,tm=0;if(root42_assign&&read_self(nodes[i]+0x48,&m,8)&&m&&read_self(nodes[i]+0x188,&tm,8)&&m==tm){if(atomic_fetch_add(&root39_label_calls,1)<3)log_line("MENU43 ROOT LABEL: refresh private text metadata");root42_assign((void*)(uintptr_t)(m+0x88),"Imperial Galactic Map");}
#endif
  ((void(*)(void*,const char*))(uintptr_t)functions[i])((void*)(uintptr_t)nodes[i],"Imperial Galactic Map");
 }
 return n;
}
static uint64_t root39_find(uint64_t parent){
 uint64_t a[256];unsigned n=0;if(!root39_children(parent,a,&n))return 0;
 for(unsigned i=0;i<n;i++){uint64_t m=0;char id[16];if(read_self(a[i]+0x48,&m,8)&&m&&read_self(m+0x48,id,16)&&!memcmp(id,"IGM_MENU42\0",11))return a[i];}return 0;
}
/* Click targets can sit inside distinct row layers. Find the closest shared
   list through ancestry, counting distinct immediate rows, not repeated calls. */
static int root40_list(uint64_t clicked,uint64_t *row,uint64_t *parent,uint64_t peers[32],unsigned *count){
 uint64_t candidate=clicked;
 for(unsigned level=0;level<6&&candidate;level++){
  uint64_t p=0;if(!read_self(candidate+0x40,&p,8)||!p)break;
  unsigned n=0;
  for(unsigned i=0;i<root39_button_count;i++){
   uint64_t child=root39_buttons[i];
   for(unsigned d=0;d<6&&child;d++){
    uint64_t q=0;if(!read_self(child+0x40,&q,8)||!q)break;
    if(q==p){unsigned k=0;while(k<n&&peers[k]!=child)k++;if(k==n&&n<32)peers[n++]=child;break;}
    if(q==child)break;
    child=q;
   }
  }
  if(n>=5){*row=candidate;*parent=p;*count=n;return 1;}
  if(p==candidate)break;
  candidate=p;
 }
 return 0;
}
static uint64_t root40_corresponding(uint64_t original,uint64_t copy,uint64_t target,unsigned depth){
 if(original==target)return copy;
 if(depth>8||root39_type(original)!=0)return 0;
 uint64_t a[256],b[256];unsigned na=0,nb=0;if(!root39_children(original,a,&na)||!root39_children(copy,b,&nb)||na!=nb)return 0;
 for(unsigned i=0;i<na;i++){uint64_t found=root40_corresponding(a[i],b[i],target,depth+1);if(found)return found;}return 0;
}
static uint64_t root40_action(uint64_t row,unsigned depth){
 if(depth>8)return 0;
 uint64_t m=0;char id[16];if(read_self(row+0x48,&m,8)&&m&&read_self(m+0x48,id,16)&&!memcmp(id,"IGM_ACTION40\0",13))return row;
 if(root39_type(row)!=0)return 0;
 uint64_t a[256];unsigned n=0;if(!root39_children(row,a,&n))return 0;
 for(unsigned i=0;i<n;i++){uint64_t found=root40_action(a[i],depth+1);if(found)return found;}return 0;
}
static uint64_t root39_entry(uint64_t source){
 uint64_t parent=0,sm=0,clicked=source,peers[32];unsigned peer_count=0;float xy[2],height=0;
 if(!source||!root40_list(clicked,&source,&parent,peers,&peer_count)){if(root39_refused_parent!=clicked)log_line("MENU42 REFUSED no verified common list");root39_refused_parent=clicked;return 0;}
 uint64_t existing=root39_find(parent);if(existing){uint64_t target=root40_action(existing,0);root41_label(existing,target?target:existing);return existing;}
 if(parent==root39_refused_parent||root39_attempts>=4)return 0;
 if(!read_self(source+0x48,&sm,8)||!sm||!read_self(sm+0x2c,xy,8)||!read_self(sm+0x24,&height,4)||!isfinite(xy[0])||!isfinite(xy[1])||!isfinite(height)||height<=0||height>512){root39_refused_parent=parent;log_line("MENU42 REFUSED layout not validated");return 0;}
 float maxy=xy[1],pitch=height;
 for(unsigned i=0;i<peer_count;i++){uint64_t p=0,m=0;float y=0;if(read_self(peers[i]+0x40,&p,8)&&p==parent&&read_self(peers[i]+0x48,&m,8)&&m&&read_self(m+0x30,&y,4)&&isfinite(y)&&y>=xy[1]&&y<4096){if(y>xy[1]&&y-xy[1]>=height&&y-xy[1]<pitch*2)pitch=y-xy[1];if(y>maxy)maxy=y;}}
 uint64_t copy=(uint64_t)(uintptr_t)root39_clone((void*)(uintptr_t)source);root39_attempts++;
 if(!copy||!root39_private(source,copy,0)){root39_refused_parent=parent;log_line("MENU42 REFUSED clone private-data validation; originals untouched");return 0;}
 uint64_t action_node=root40_corresponding(source,copy,clicked,0);
 if(!action_node){root39_refused_parent=parent;log_line("MENU42 REFUSED missing independent click target");return 0;}
 uint64_t cm=0;if(!read_self(copy+0x48,&cm,8)||!cm||!root41_label(copy,action_node)){root39_refused_parent=parent;log_line("MENU42 REFUSED no validated primary label; inspect LABEL CANDIDATES");return 0;}
 /* Only the newly allocated node and its independent metadata are changed. */
 memset((void*)(uintptr_t)(cm+0x48),0,16);memcpy((void*)(uintptr_t)(cm+0x48),"IGM_MENU42",10);
 if(action_node!=copy){uint64_t am=0;if(!read_self(action_node+0x48,&am,8)||!am){root39_refused_parent=parent;return 0;}memset((void*)(uintptr_t)(am+0x48),0,16);memcpy((void*)(uintptr_t)(am+0x48),"IGM_ACTION40",12);}
 float y=maxy+pitch;memcpy((void*)(uintptr_t)(cm+0x30),&y,4);
 *(unsigned char*)(uintptr_t)(cm+0x60)=0;*(unsigned char*)(uintptr_t)(cm+0x61)=0;
 *(unsigned char*)(uintptr_t)(copy+0x51)=1;
 uint64_t added=(uint64_t)(uintptr_t)root39_add((void*)(uintptr_t)parent,(void*)(uintptr_t)copy,1);
 if(added!=copy||root39_find(parent)!=copy){root39_refused_parent=parent;log_line("MENU42 ADD not confirmed; originals not replaced");return 0;}
 root39_attempts--;
 log_line("MENU42 APPENDED common-list peers=%u nested_target=%d parent=%llx y=%.2f height=%.2f append_only=1",peer_count,action_node!=copy,(unsigned long long)parent,y,height);return copy;
}
static unsigned char root39_click(void *page,void *node,unsigned char a,unsigned char b,const void *action,unsigned char c,unsigned char d){
 if(root39_scope&&node){if(!root39_template)root39_template=(uint64_t)(uintptr_t)node;if(root39_button_count<32)root39_buttons[root39_button_count++]=(uint64_t)(uintptr_t)node;}
 return root39_click_original(page,node,a,b,action,c,d);
}
static void root39_page(void *page){
#if !defined(IGM_ROOT_MODEL_ONLY) && !defined(IGM_MENU_ROUTING_TEST)
 if(root39_owned_page&&menu_mode&&native_root())return; /* No invisible root buttons or repeated open/reset under our page. */
#endif
 int before=root39_scope;root39_scope=1;root39_template=0;root39_button_count=0;
 root39_original(page);root39_scope=before;
 atomic_fetch_add(&root39_calls,1);atomic_store(&root39_last_buttons,root39_button_count);
 if(root39_button_count&&!root39_logged_buttons){root39_logged_buttons=1;log_line("MENU42 FIRST ROOT BUTTONS captured=%u",root39_button_count);}
 if(root39_traces++<4)log_line("MENU42 ROOT CONTROLLER observed_buttons=%u",root39_button_count);
 if(!root39_template)return;
 uint64_t node=root39_entry(root39_template);if(!node)return;
 const unsigned char action[16]={0,0,0,0,0,0,0,0,0xf6,0xcc,0xae,0x18,1,0,0,0};
 uint64_t target=root40_action(node,0);if(!target)target=node;
 if(root39_click_original(page,(void*)(uintptr_t)target,1,0,action,0,1)){
  navigate(1);root39_owned_page=1;
#if !defined(IGM_ROOT_MODEL_ONLY) && !defined(IGM_MENU_ROUTING_TEST)
  catalogue42_reset();
#endif
 root39_request(6,0);log_line("MENU42 OPEN native catalogue requested through new button");
 }
}
