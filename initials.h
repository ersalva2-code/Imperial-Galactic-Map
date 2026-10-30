#ifndef IGM_INITIALS_H
#define IGM_INITIALS_H
#include <string.h>
#include <stdint.h>
#include <math.h>
/* Latin ASCII word initials; unsupported UTF-8 words are skipped, never guessed. */
static void make_initials(const char *name,char out[7]) {
    size_t n=0;int start=1;
    for(size_t i=0;name[i]&&i<127;i++) {
        unsigned char c=(unsigned char)name[i];
        if(c==' '||c=='-'||c=='_'||c=='/'||c=='\t'){start=1;continue;}
        if(start && n<6 && ((c>='A'&&c<='Z')||(c>='a'&&c<='z')))out[n++]=(char)(c>='a'?c-32:c);
        start=0;
    }
    out[n]=0;
}
static float initials_unit(size_t n){return n?18.0f/(6.0f*(float)n+6.0f):0;}
static int initials_light(const unsigned char rgb[3]) {
    float l=0;
    for(int i=0;i<3;i++){float s=rgb[i]/255.0f;s=s<=.04045f?s/12.92f:powf((s+.055f)/1.055f,2.4f);l+=s*(i==0?.2126f:i==1?.7152f:.0722f);}
    return l<.179f;
}
static const unsigned char initial_font[26][7]={
{14,17,17,31,17,17,17},
{30,17,17,30,17,17,30},
{15,16,16,16,16,16,15},
{30,17,17,17,17,17,30},
{31,16,16,30,16,16,31},
{31,16,16,30,16,16,16},
{15,16,16,23,17,17,15},
{17,17,17,31,17,17,17},
{31,4,4,4,4,4,31},
{7,2,2,2,18,18,12},
{17,18,20,24,20,18,17},
{16,16,16,16,16,16,31},
{17,27,21,21,17,17,17},
{17,25,21,19,17,17,17},
{14,17,17,17,17,17,14},
{30,17,17,30,16,16,16},
{14,17,17,17,21,18,13},
{30,17,17,30,20,18,17},
{15,16,16,14,1,1,30},
{31,4,4,4,4,4,4},
{17,17,17,17,17,17,14},
{17,17,17,17,17,10,4},
{17,17,17,21,21,27,17},
{17,17,10,4,10,17,17},
{17,17,10,4,4,4,4},
{31,1,2,4,8,16,31}
};
#endif
