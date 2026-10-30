/* Screen-space overlay; native discovery data is never changed. */
static void draw_owned56(void *ctx,unsigned char *state,float x,float y,float unit){
 IgmPathBatch batch;batch.count=0;float r=6.5f*unit,t=1.2f*unit;
 for(int i=0;i<48;i++){float a=i*.130899694f,b=(i+1)*.130899694f;float q[8]={x+cosf(a)*r,y+sinf(a)*r,x+cosf(b)*r,y+sinf(b)*r,x+cosf(b)*(r-t),y+sinf(b)*(r-t),x+cosf(a)*(r-t),y+sinf(a)*(r-t)};for(int j=0;j<4;j++){batch.commands[batch.count++]=j?1:0;batch.commands[batch.count++]=q[2*j];batch.commands[batch.count++]=q[2*j+1];}batch.commands[batch.count++]=3;}
 color(state,.12f,1.f,.48f,marker_alpha);submit_batch(ctx,&batch);
}
