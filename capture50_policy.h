#ifndef IGM_CAPTURE50_POLICY
#define IGM_CAPTURE50_POLICY
#include <stdint.h>
enum {C50_WAIT,C50_COMMIT,C50_DISCARD,C50_TIMEOUT};
static inline int c50_result(uint64_t started,uint64_t now,int running,int exit_ok,int same_context,int foreground,int bmp_ok){if(!same_context||!foreground)return C50_DISCARD;if(running)return now>=started&&now-started<8000?C50_WAIT:C50_TIMEOUT;return exit_ok&&bmp_ok?C50_COMMIT:C50_DISCARD;}
#endif
