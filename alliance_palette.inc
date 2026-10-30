#include "alliance_colors.h"
static IgmPalette alliance_palette,palette_staged;
static SRWLOCK palette_lock=SRWLOCK_INIT;
static int palette_writable=1;
static ULONGLONG palette_last_failure;
static void load_palette(void) {
    wchar_t path[32768];make_path(path,L"colores_alianzas.tsv");
    FILE *f=_wfopen(path,L"rb");
    if(!f) {
        if(GetFileAttributesW(path)!=INVALID_FILE_ATTRIBUTES){palette_writable=0;log_line("PALETTE REFUSED: existing file could not be read; preserved");}
        else log_line("PALETTE: first use, no assignments yet");
        return;
    }
    char line[256];unsigned line_number=0;
    while(fgets(line,sizeof(line),f)) {
        line_number++;if(line[0]=='#'||line[0]=='\r'||line[0]=='\n')continue;
        IgmColor color={0};
        if(!palette_parse_line(line,&color)||!palette_import(&alliance_palette,&color)) {
            palette_writable=0;log_line("PALETTE INVALID line=%u; existing file preserved; new assignments disabled",line_number);
        }
    }
    if(ferror(f))palette_writable=0;
    fclose(f);log_line("PALETTE LOADED bindings=%zu writable=%d",alliance_palette.count,palette_writable);
}
static int save_palette(const IgmPalette *p) {
    wchar_t path[32768],temporary[32768];make_path(path,L"colores_alianzas.tsv");make_path(temporary,L"colores_alianzas.tsv.tmp");
    HANDLE h=CreateFileW(temporary,GENERIC_WRITE,0,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
    if(h==INVALID_HANDLE_VALUE)return 0;
    int ok=write_handle(h,"# IGM_PALETTE_V1 id0 id1 id2 id3 slot red green blue\r\n");
    for(size_t i=0;ok&&i<p->count;i++) {
        char line[192];ok=palette_format_line(&p->entries[i],line,sizeof(line))&&write_handle(h,line);
    }
    if(ok)ok=FlushFileBuffers(h)!=0;
    CloseHandle(h);
    if(ok)ok=MoveFileExW(temporary,path,MOVEFILE_REPLACE_EXISTING|MOVEFILE_WRITE_THROUGH)!=0;
    if(!ok)DeleteFileW(temporary);
    return ok;
}
/* Only the reader thread assigns/persists colours; render thread takes read-only snapshots. */
static void sync_alliance_colors(void) {
    if(!palette_writable)return;
    ULONGLONG now=GetTickCount64();if(palette_last_failure&&now-palette_last_failure<5000)return;
    Seen snapshot[4096];size_t n;
    AcquireSRWLockShared(&known_lock);n=known_count;memcpy(snapshot,known,n*sizeof(Seen));ReleaseSRWLockShared(&known_lock);
    AcquireSRWLockShared(&palette_lock);palette_staged=alliance_palette;size_t before=alliance_palette.count;ReleaseSRWLockShared(&palette_lock);
    uint32_t player_id[4];if(player58_get(player_id)&&!own_id(player_id)){IgmColor ignored;palette_assign(&palette_staged,player_id,&ignored);}
    for(size_t i=0;i<n;i++) {
        if(!nonzero_id(snapshot[i].id)||own_id(snapshot[i].id))continue;
        IgmColor ignored;palette_assign(&palette_staged,snapshot[i].id,&ignored);
    }
    if(palette_staged.count==before)return;
    if(!save_palette(&palette_staged)) {palette_last_failure=now;log_line("PALETTE WRITE FAILED: new colours not published; retry later");return;}
    AcquireSRWLockExclusive(&palette_lock);alliance_palette=palette_staged;ReleaseSRWLockExclusive(&palette_lock);
    palette_last_failure=0;
    for(size_t i=before;i<palette_staged.count;i++) {
        IgmColor *c=&palette_staged.entries[i];
        log_line("COLOR_ASSIGNED id=%u,%u,%u,%u slot=%u rgb=#%02x%02x%02x persisted=1",c->id[0],c->id[1],c->id[2],c->id[3],c->slot,c->rgb[0],c->rgb[1],c->rgb[2]);
    }
}
static int get_alliance_color(const uint32_t id[4],unsigned char rgb[3],uint32_t *slot) {
    AcquireSRWLockShared(&palette_lock);int index=palette_find(&alliance_palette,id);
    if(index>=0){memcpy(rgb,alliance_palette.entries[index].rgb,3);*slot=alliance_palette.entries[index].slot;}
    ReleaseSRWLockShared(&palette_lock);return index>=0;
}
