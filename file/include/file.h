#ifndef FILE_LOCK_H
#define FILE_LOCK_H

#include <stdint.h>


typedef enum {
    
    NEW_LINE,
    SPACE
    
}FILE_tenReadType;


extern uint32_t *
FILE_pu32ReadU32Integers(uint32_t *, const char *, FILE_tenReadType);

#endif
