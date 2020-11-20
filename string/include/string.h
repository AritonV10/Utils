#ifndef STRING_H_GUARD
#define STRING_H_GUARD

#include <stdint.h>

#define STRING__nStringInitialSize 8
#define STRING__nStringReallocSize 8

#define STRING__nOk   (int8_t)(1)
#define STRING__nFail (int8_t)(-1)


typedef struct {
    
    uint32_t u32Index;    
    uint32_t u32Size;
    char     *pchBuffer;
    
}STRING_tstString;


extern STRING_tstString *
STRING_stStringMake(void);

extern STRING_tstString *
STRING_stStringMakeWith(const char *);

extern int8_t
STRING_i8StringAddChar(const char, STRING_tstString *);

extern int8_t
STRING_i8StringRemoveChar(STRING_tstString *);

extern int8_t
STRING_i8StringAddCharAt(const char, const uint32_t, STRING_tstString *);

extern int8_t
STRING_i8StringRemoveCharAt(const uint32_t, STRING_tstString *);

extern uint32_t
STRING_u32StringLength(const STRING_tstString *);

#endif
