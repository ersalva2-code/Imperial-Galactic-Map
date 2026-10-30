static int marker_distance(uint64_t map,uint64_t ua,float *out){
 float world[8],camera[3];if(!pos54_world(&positions54,read_self,map,ua,world)||!read_self(map+0x1300,camera,12))return 0;
 float x=world[0]-camera[0],y=world[1]-camera[1],z=world[2]-camera[2];*out=sqrtf(x*x+y*y+z*z);return isfinite(*out);
}

/* Keep the proven native anchor path first; private-cache failures must not hide it. */
static int locate_system(uint64_t map,uint64_t ua,const float viewport[2],float *x,float *y,int *mode) {
    if(existing_anchor(read_self,map,ua,x,y,viewport)){*mode=1;return 1;}
    float world[8],camera[20],fov,screen[4]={0};uint64_t renderer=0;
    if(!pos54_world(&positions54,read_self,map,ua,world)||!read_self(map+0x12d0,camera,64)||
       !read_self(map+0x1490,&fov,4)||!isfinite(fov)||fov<0.01f||fov>180||
       !read_self(image_base+native.renderer,&renderer,8)||!renderer)return 0;
    memset(camera+16,0,16);
    /* The native projector supplies its own front/back status in output.w.
       Do not pre-cull using an assumed camera-axis sign convention. */
    project_points((void*)(uintptr_t)renderer,camera,fov,world,screen,1,0,viewport);
    if(!isfinite(screen[0])||!isfinite(screen[1])||!isfinite(screen[3])||screen[3]<=0||
       screen[0]<16||screen[1]<16||screen[0]>viewport[0]-16||screen[1]>viewport[1]-16)return 0;
    *x=screen[0];*y=screen[1];*mode=2;return 1;
}
/* Existing anchors use the engine's own world record, including its marker offset.
   An unverified private position must never fade a perfectly visible native anchor. */
static int anchor55_distance(uint64_t map,uint64_t ua,float *out){
 uint32_t n=0;if(!read_self(map+0x13030,&n,4)||n>320)return 0;
 for(uint32_t i=0;i<n;i++){uint64_t found=0;if(!read_self(map+0xcc30+i*8,&found,8)||(found&UINT64_C(0x000fffffffffffff))!=ua)continue;
  float p[8],camera[3];if(!read_self(map+0x9030+i*32,p,32)||!read_self(map+0x1300,camera,12)||!finite_point(p)||!finite_point(p+4)||!finite_point(camera))return 0;
  float x=p[0]+p[4]-camera[0],y=p[1]+p[5]-camera[1],z=p[2]+p[6]-camera[2];*out=sqrtf(x*x+y*y+z*z);return isfinite(*out);
 }return 0;
}
