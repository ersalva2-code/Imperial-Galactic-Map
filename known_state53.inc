static int changed_unlocked(uint64_t ua,const uint32_t id[4]) {
    if(imperial53_find(ua))id=imperial53_id;
    for(size_t i=0;i<known_count;i++) if(known[i].ua==ua) {
        if(!memcmp(known[i].id,id,16))return 0;
        memcpy(known[i].id,id,16); return 1;
    }
    if(known_count==4096)return 0;
    known[known_count].ua=ua; memcpy(known[known_count++].id,id,16); return 1;
}
static int changed(uint64_t ua,const uint32_t id[4]) {
    AcquireSRWLockExclusive(&known_lock);int result=changed_unlocked(ua,id);ReleaseSRWLockExclusive(&known_lock);return result;
}
static int needs_record(uint64_t ua,const uint32_t id[4]) {
    if(imperial53_find(ua))return 0;
    for(size_t i=0;i<known_count;i++)if(known[i].ua==ua)return memcmp(known[i].id,id,16)!=0;
    return known_count<4096;
}
