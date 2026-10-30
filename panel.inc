#include "panel_model.h"

static void ui_rect(void *ctx,unsigned char *state,float x,float y,float w,float h,float r,float g,float b,float a){
 float q[8]={x,y,x+w,y,x+w,y+h,x,y+h};color(state,r,g,b,a);polygon(ctx,q,4);
}
/* The former F8 panel is disabled. F8 is now the favourite hotkey. */
static void draw_panel(void *ctx){(void)ctx;}
