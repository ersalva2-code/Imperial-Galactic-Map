#ifndef IGM_MAP_CATALOGUE_H
#define IGM_MAP_CATALOGUE_H
#include "core.h"
typedef struct {uint64_t ua;uint32_t id[4];} IgmRecord;
static inline size_t select_catalogue(const IgmRecord *records,size_t count,unsigned galaxy,
        IgmRecord *out,size_t capacity,size_t *imperial,size_t *other) {
    size_t n=0;*imperial=0;*other=0;
    for(int category=1;category<=2;category++)for(size_t i=0;i<count && n<capacity;i++) {
        if(alliance_category(records[i].id)!=category||((records[i].ua>>32)&255)!=galaxy)continue;
        out[n++]=records[i];if(category==1)(*imperial)++;else(*other)++;
    }
    return n;
}
#endif
