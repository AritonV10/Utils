#include "file.h"

#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>


/******************************** Internal Functions Declarations ********************************/
/*************************************************************************************************/

static uint32_t
FILE__u32Stoi(const char *);

static uint32_t 
FILE__u32FileSize(int, FILE_tenReadType);

static void
FILE__vExit(int32_t, const char *);

static void *
FILE__pvMalloc(uint32_t);



/******************************** Internal Functions Definition **********************************/
/*************************************************************************************************/


static void
FILE__vExit(int32_t i32Status, const char *pchMessage) {
    printf("\n%s\n", pchMessage);
    exit(i32Status);
}

static void *
FILE__pvMalloc(uint32_t u32Size) {
    
    void *pvAddress;
    
    pvAddress = malloc(u32Size);
    
    if(pvAddress == NULL) {
        FILE__vExit(EXIT_FAILURE, "ERROR: Could not allocate memory");
    }
    
    return(pvAddress);
}

static uint32_t 
FILE__u32FileSize(int FD, FILE_tenReadType enType) {
    
    char     cByte;
    uint32_t u32Size;
    
    u32Size = 0;
    
    for(; read(FD, &cByte, 1) != 0; ) {
        
        switch(enType) {
            case NEW_LINE:
                if(cByte == '\n') {
                    ++u32Size;
                }
            break;
            case SPACE:
                if(cByte == '\n') {
                    ++u32Size;
                }
            break;
        }
        
    }

    (void)lseek(FD, 0, SEEK_SET);
    
    return(u32Size + 1);
}


static uint32_t
FILE__u32Stoi(const char *pcInteger) {
    
    uint32_t u32Value;
    
    u32Value = 0;
    
    for(; *pcInteger != '\0'; ++pcInteger) {
        u32Value = (u32Value * 10) + (*pcInteger & 0x0F);
    }
    
    
    
    return(u32Value);
    
}


/******************************** Interface Functions Definitions ********************************/
/*************************************************************************************************/


extern uint32_t *
FILE_pu32ReadU32Integers(uint32_t *u32Size, const char *pcFilePath, FILE_tenReadType enType) {
    
    char cByte;
    char cBytes[11];
    int  iFD;
    
    uint32_t u32Index;
    uint32_t u32BufIdx;
    
    uint32_t *u32Out;
    
    /* Open the file */
    iFD = open(pcFilePath, O_RDONLY);
    switch(enType){
        case NEW_LINE:
            /* Get the number of integers and alloc the mem */
            u32Out = FILE__pvMalloc(sizeof(uint32_t) * FILE__u32FileSize(iFD, NEW_LINE));
            
            for(u32Index = 0, u32BufIdx = 0; ; ) {
                
                if((read(iFD, &cByte, 1)) != 0) {
                    
                    if(cByte == '\n') {
                        
                        cBytes[u32Index] = 0;
                        
                        /* Convert to integer */
                        u32Out[u32BufIdx++] = FILE__u32Stoi((const char *)&cBytes);
                        
                        u32Index = 0;
                        
                    } else {
                        
                        cBytes[u32Index++] = cByte;
                        
                    }
                    
                } else {
                    
                    cBytes[u32Index] = 0;
                    
                    u32Out[u32BufIdx++] = FILE__u32Stoi((const char *)&cBytes);
                    
                    break;
                }
                
            }
            
        break;
        
    }
    
    close(iFD);
    
    *u32Size = u32BufIdx;
    
    return(u32Out);
    
}
