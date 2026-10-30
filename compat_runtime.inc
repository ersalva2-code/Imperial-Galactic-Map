#include "compat_scan.h"
static NativeBindings native;
static uint32_t native_text_rva,native_text_size;
static int data_slot(uint64_t base,const IMAGE_SECTION_HEADER *sections,unsigned n,uint32_t rva){
 for(unsigned i=0;i<n;i++){
  uint64_t start=sections[i].VirtualAddress,size=sections[i].Misc.VirtualSize;
  if(rva>=start&&size>=8&&(uint64_t)rva-start<=size-8&&
     (sections[i].Characteristics&IMAGE_SCN_MEM_READ)&&!(sections[i].Characteristics&IMAGE_SCN_MEM_EXECUTE)){
   uint64_t ignored;return read_self(base+rva,&ignored,8);
  }
 }
 return 0;
}
static int probe_compatibility(uint64_t base){
 IMAGE_DOS_HEADER dos;IMAGE_NT_HEADERS64 nt;IMAGE_SECTION_HEADER sections[64];char why[200]={0};
 if(!read_self(base,&dos,sizeof(dos))||dos.e_magic!=IMAGE_DOS_SIGNATURE||dos.e_lfanew<64||dos.e_lfanew>1048576||
 !read_self(base+(uint32_t)dos.e_lfanew,&nt,sizeof(nt))||nt.Signature!=IMAGE_NT_SIGNATURE||
 nt.FileHeader.Machine!=IMAGE_FILE_MACHINE_AMD64||nt.OptionalHeader.Magic!=IMAGE_NT_OPTIONAL_HDR64_MAGIC||
 nt.FileHeader.NumberOfSections<1||nt.FileHeader.NumberOfSections>64||nt.OptionalHeader.SizeOfImage>256*1024*1024||
 !read_self(base+(uint32_t)dos.e_lfanew+24+nt.FileHeader.SizeOfOptionalHeader,sections,nt.FileHeader.NumberOfSections*sizeof(*sections))){log_line("COMPAT REFUSED: invalid loaded PE headers");return 0;}
 for(unsigned i=0;i<nt.FileHeader.NumberOfSections;i++)if(!memcmp(sections[i].Name,".text\0\0",8)){
  native_text_rva=sections[i].VirtualAddress;native_text_size=sections[i].SizeOfRawData;
  if(!(sections[i].Characteristics&IMAGE_SCN_MEM_EXECUTE)||!native_text_size||native_text_size>96*1024*1024||
    (uint64_t)native_text_rva+native_text_size>nt.OptionalHeader.SizeOfImage){log_line("COMPAT REFUSED: invalid code range");return 0;}
 }
 if(!native_text_size){log_line("COMPAT REFUSED: no main text section");return 0;}
 unsigned char *copy=VirtualAlloc(NULL,native_text_size,MEM_COMMIT|MEM_RESERVE,PAGE_READWRITE);if(!copy){log_line("COMPAT REFUSED: scan allocation failed");return 0;}
 int ok=read_self(base+native_text_rva,copy,native_text_size)&&resolve_native(copy,native_text_size,native_text_rva,nt.OptionalHeader.SizeOfImage,&native,why,sizeof(why));
 VirtualFree(copy,0,MEM_RELEASE);
 if(!ok){log_line("COMPAT READER REFUSED: %s",why);return 0;}
 if(!data_slot(base,sections,nt.FileHeader.NumberOfSections,native.singleton)){log_line("COMPAT READER REFUSED: singleton slot not readable data");return 0;}
 if(native.render_ok&&(!data_slot(base,sections,nt.FileHeader.NumberOfSections,native.nvg)||!data_slot(base,sections,nt.FileHeader.NumberOfSections,native.renderer))){native.render_ok=0;snprintf(why,sizeof(why),"graphics globals outside readable data");}
 log_line("COMPAT READER READY profile=%s singleton_rva=%x alliance_offset=%x",native.profile?"updated":"original",native.singleton,native.alliance_offset);
 log_line("COMPAT RENDER %s: %s",native.render_ok?"VERIFIED":"REFUSED",why);
 for(int i=0;i<15;i++)if(native.rva[i])log_line("COMPAT BIND %s rva=%x",native_patterns[i].name,native.rva[i]);
 return 1;
}
/* Recheck live bytes before installing graphics hooks; no scan/fallback in a render callback. */
static int verify_active_bindings(uint64_t base){
 if(!native.render_ok)return 0;
 unsigned char bytes[1500];
 for(int i=B_MAP;i<=B_MARKER;i++){
  const BytePattern *p=&native_patterns[i].variants[native.profile];
  if(p->size>sizeof(bytes)||!read_self(base+native.rva[i],bytes,p->size)||!pattern_at(bytes,p->size,p)){
   log_line("COMPAT RENDER REFUSED: live bytes changed for %s",native_patterns[i].name);return 0;
  }
 }
 return 1;
}
