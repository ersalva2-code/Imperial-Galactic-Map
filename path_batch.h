#ifndef IGM_PATH_BATCH
#define IGM_PATH_BATCH
#include <stddef.h>
#define IGM_BATCH_FLOATS 32768
typedef struct {float commands[IGM_BATCH_FLOATS];size_t count;} IgmPathBatch;
static int batch_rect(IgmPathBatch *b,float x,float y,float w,float h){
 if(b->count>IGM_BATCH_FLOATS-13)return 0;
 const float q[13]={0,x,y,1,x+w,y,1,x+w,y+h,1,x,y+h,3};
 for(size_t i=0;i<13;i++)b->commands[b->count++]=q[i];
 return 1;
}
#endif
