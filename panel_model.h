#ifndef IGM_PANEL_MODEL
#define IGM_PANEL_MODEL
#include <stddef.h>
typedef struct {int open,tab;size_t page;int visibility;} IgmPanel;
static int panel_category_visible(const IgmPanel *p,int category){return p->visibility==0||(p->visibility==1&&category==1)||(p->visibility==2&&category==2);}
#endif
