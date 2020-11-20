#include <stdlib.h>
#include <stdio.h>

#include "string.h"


/******************************** Internal Functions Declarations ********************************/
/*************************************************************************************************/

static void
STRING__vExit(int32_t, const char *);
static void *
STRING__pvMalloc(uint32_t);

static void
STRING__vStringRealloc(STRING_tstString *const);


/******************************** Internal Functions Definition **********************************/
/*************************************************************************************************/


static void
STRING__vStringRealloc(STRING_tstString *const stString) {
    
    stString->pchBuffer = realloc((void *)stString->pchBuffer, (stString->u32Size + STRING__nStringReallocSize));
        
    if(stString == NULL) {
        STRING__vExit(EXIT_FAILURE, "Could not reallocate the String");
    }
        
    stString->u32Size = stString->u32Size + STRING__nStringReallocSize;
}

static void
STRING__vExit(int32_t i32Status, const char *pchMessage) {
    printf("\n%s\n", pchMessage);
    exit(i32Status);
}

static void *
STRING__pvMalloc(uint32_t u32Size) {
    
    void *pvAddress;
    
    pvAddress = malloc(u32Size);
    
    if(pvAddress == NULL) {
        STRING__vExit(EXIT_FAILURE, "Could not allocate memory for the string");
    }
    
    return(pvAddress);
}

/******************************** Interface Functions Definitions ********************************/
/*************************************************************************************************/

STRING_tstString *
STRING_stStringMake(void) {
    
    STRING_tstString *stString;
    
    stString            = STRING__pvMalloc(sizeof(STRING_tstString));
    stString->u32Size   = STRING__nStringInitialSize;
    stString->u32Index  = 0;
    stString->pchBuffer = STRING__pvMalloc(sizeof(char) * stString->u32Size);
    
    stString->pchBuffer[0] = 0;
    
    return(stString);
    
}

STRING_tstString *
STRING_stStringMakeWith(const char *pchString) {
    
    uint32_t    u32StringLength;
    char *pchTemp;
    const char *pchStr;
    
    
    STRING_tstString *stString;
    
    pchStr = pchString;
    
    for(u32StringLength; *pchStr != 0; ++pchStr, ++u32StringLength)
        ;
    
    
    stString            = STRING__pvMalloc(sizeof(STRING_tstString));
    stString->u32Size   = (STRING__nStringInitialSize + u32StringLength);
    stString->u32Index  = u32StringLength;
    stString->pchBuffer = STRING__pvMalloc(sizeof(char) * stString->u32Size);
    
    
    
    for(pchStr = pchString, pchTemp = stString->pchBuffer; pchTemp <= (stString->pchBuffer + (u32StringLength - 1)); ++pchTemp, ++pchStr) {
        *pchTemp = *pchStr;
    }
    
    stString->pchBuffer[stString->u32Index] = 0;
    
    return(stString);
}

int8_t
STRING_i8StringAddChar(const char chChar, STRING_tstString *const stString) {
    
    /* Check if we can add the character */
    if((stString->u32Index + 1) == stString->u32Size) {
        STRING__vStringRealloc(stString);
    }
    
    /* Add the character */
    stString->pchBuffer[stString->u32Index] = chChar;
    
    /* Increment the tail and set the terminator */
    stString->u32Index = stString->u32Index + 1;
    stString->pchBuffer[stString->u32Index] = 0;
    
    return(STRING__nOk);
    
}

int8_t
STRING_i8StringAddCharAt(const char chChar, const uint32_t u32Position, STRING_tstString *const stString) {
    
    char *pchHead;
    
    if(u32Position > stString->u32Index) {
        return(STRING__nFail);
    }
    
    /* Check if we can add the character */
    if((stString->u32Index + 1) == stString->u32Size) {
        STRING__vStringRealloc(stString);
    }
    
    /* Move the characters and add the new character */
    for(pchHead = &(stString->pchBuffer[stString->u32Index]); pchHead >= &(stString->pchBuffer[u32Position - 1]); --pchHead) {
        *(pchHead + 1) = *pchHead;
    }
    
    stString->u32Index = stString->u32Index + 1;
    *(pchHead + 1) = chChar;
    
}

int8_t
STRING_i8StringRemoveChar(STRING_tstString *const stString) {
    
    if(stString->u32Index == 0) {
        return(STRING__nFail);
    }
    
    stString->pchBuffer[stString->u32Index - 1] = 0;
    stString->u32Index = stString->u32Index - 1;
    
    return(STRING__nOk);
    
}

int8_t
STRING_i8StringRemoveCharAt(const uint32_t u32Position, STRING_tstString *const stString) {
    
    char *pchHead;
    
    if(u32Position > stString->u32Index) {
        return(STRING__nFail);
    }
    
    for(pchHead = (stString->pchBuffer + (u32Position - 1)); pchHead <= (stString->pchBuffer + (stString->u32Index - 1)); ++pchHead) {
        *pchHead = *(pchHead + 1);
    }
    
    stString->u32Index = stString->u32Index - 1;
    
    return(STRING__nOk);
}

uint32_t
STRING_u32StringLength(const STRING_tstString *const stString) {
    return((stString->pchBuffer + stString->u32Index) - stString->pchBuffer);
}
