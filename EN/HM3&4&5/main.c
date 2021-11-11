/**
 *  https://github.com/AritonV10
 */
#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <fcntl.h>
#include "string/include/string.h"
#include "file/include/file.h"


/***************************************************************************************/
/***************************************************************************************/

#define MAIN__nInputs  0x06u
#define MAIN__nOutputs 0x05u
#define MAIN__nComb    (2 << (MAIN__nInputs - 1))

#define MAIN__nTab     0x09u
#define MAIN__nEOL     0x0Au
#define MAIN__nOne     0x31u
#define MAIN__nA       0x61u

#define MAIN__nMinSize 0x80u

#define MAIN__SubH(c) #c    
#define MAIN__Sub(c)  (const uint8_t *)"O_{" MAIN__SubH(c) "} = "




/*                 1                       2                       3  */
/* 0 1 2 3 4 5 6 7 8  9  10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 */
/* 0 1 2 3 4 5 6 7 10 11 12 13 14 15 16 17 20 21 22 23 24 25 26 27 30 */
/* Octal up to 8^3 - doesn't work for more than that                 */
#define MAIN__u8DecToOctal128(n) (10u * ( (((uint8_t)(n)/8) & 0x07u) + ((n) >= 0x40u ? 10u : 0)) + ((n) & (0x07u))) 
#define MAIN__u8OctToDec(n) (64u * ((n) >= 100) + 8 * ((uint8_t) (n)/10) + ((n) % 10))

#define MAIN__Group(n) (1u << (n))
#define MAIN__boMultiple2(n) ((n) & ((n) - 1))
#define MAIN__OctDiff(n, m) MAIN__u8DecToOctal128( (MAIN__u8OctToDec(n) ^ MAIN__u8OctToDec(m) ) )


/* Dig to char */
#define MAIN__Dig(n, p) (0x30u | (((uint8_t)n/p) % 10))
#define MAIN__Dig2(n) MAIN__Dig(n, 10)
#define MAIN__Dig1(n) MAIN__Dig(n, 1)


/***************************************************************************************/
/***************************************************************************************/


static uint8_t MAIN__au8OUT[MAIN__nComb][MAIN__nOutputs];
/* !! Don't remove !! */
static uint32_t SEG_FAULT_GUARD = 0;

static uint8_t MAIN__au8IN[MAIN__nComb][MAIN__nInputs];

/* 5x64 */
static uint16_t MAIN__au16Octets[MAIN__nOutputs][MAIN__nComb];

/* FCD optimizer */
static STRING_tstString *MAIN__astFCD[MAIN__nOutputs];
/* D mark */
static STRING_tstString *MAIN__astMD[MAIN__nOutputs];

static STRING_tstString *MAIN__astMinimized[MAIN__nOutputs];

static const uint8_t MAIN__au8LateXBar[] = {'\\', 'b', 'a', 'r', '{', '}'};
static const uint8_t MAIN__au8LateXDMark[] = 
    {'D', '_', '{', '1', ':', '(', '$', ')', '}', '^', '{', 'a', 'b', 'c', 'd', 'e', 'f', '}'};

static const uint8_t MAIN__u8LaTeXLen = (sizeof(MAIN__au8LateXBar)/sizeof(uint8_t));
static const uint8_t MAIN__u8LateXDLen = (sizeof(MAIN__au8LateXDMark)/sizeof(uint8_t));

/* Column names */
static const uint8_t *MAIN__au8OutputChar[MAIN__nOutputs] = {
        /* O_31 */
        MAIN__Sub(31),
        /* O_32 */
        MAIN__Sub(32),
        /*O_33*/
        MAIN__Sub(33),
        /*O_34*/
        MAIN__Sub(34),
        /*O_35*/
        MAIN__Sub(35)
    };



/* Used to keep of the depth of the groups formed */
static int8_t MAIN__i8Depth;

static uint32_t MAIN__u32SerieIdx;

/* Main idx */
volatile static uint32_t MAIN__u32Idx;

/* Index for MAIN__au32Minimized */
volatile static uint32_t MAIN__u32MaxMinIdx; 

/* Used to keep track of the generated series */
volatile static uint32_t MAIN__u32MinIdx;

/* Used to keep track of the maximum number of terms contained within a series */
static uint32_t MAIN__u32Terms;

/* Index for MAIN__astMinimzed */
static uint32_t MAIN__u32StrMinIdx;

/* Adjents matrix */
uint32_t MAIN__au32Adjacents[MAIN__nMinSize][MAIN__nMinSize];

/* Used to compute the series */
uint32_t MAIN__au32AdjacentSerie[MAIN__nMinSize];

/* Used to keep track of the difference of each MAIN__i8Depth */
uint32_t MAIN__au32Difference[MAIN__nMinSize];

/* Used for computing the minimzed series */
uint32_t MAIN__au32MinBuf[MAIN__nMinSize][MAIN__nMinSize];

/* Used to keep track which element was used before */
uint32_t MAIN__au32Used[MAIN__nMinSize];

/* Minimized series */
uint32_t MAIN__au32Minimized[MAIN__nMinSize][MAIN__nMinSize];

/* Differences of minimized series */
uint32_t MAIN__au32MinDifference[MAIN__nMinSize][MAIN__nMinSize];


uint32_t MAIN__au32Octets[] = {1,2,3,4,5,6,7,10,11,12,13,14,15,16,17,20,22,23,24,25,26,27,30,31,32,33,35,36,37,41,43,44,45,50,51,52,56,57,61,62,63,66,70,71,74,76,77};
int MAIN__u32OctetsLen = sizeof(MAIN__au32Octets)/sizeof(uint32_t);


/***************************************************************************************/
/***************************************************************************************/

/**
 * Populating the input signal array 
 */
static void
MAIN__vFill(void);


/**
 * Get the output signals
 */
static void
MAIN__vParse(const char *);

static void
MAIN__vAlloc(void);

static void 
MAIN__vFCD(void);

static void
MAIN__vDM(void);

static void
MAIN__vMinimize(void);

static void
MAIN__vWriteH(STRING_tstString *[MAIN__nOutputs], const char *);


static void
MAIN__vMinimize(void);

static void
MAIN__vMinimizeToString(void);

static void 
MAIN__vSaveMin(void);

static void
MAIN__vFindAdjacents(void);


/***************************************************************************************/
/***************************************************************************************/




int main(void) {
    
    int iFD;

    uint8_t u8Idx;

    /*****************************************************/
    /*****************************************************/

    (void)SEG_FAULT_GUARD;

    /* Fill the input signals */
    MAIN__vFill();


    /* Alloc the string */
    MAIN__vAlloc();

    /* Fill the output signals */
    MAIN__vParse("Tema3.txt");

    /*
        STRING_i8StringAddChar('MAIN__au32Octets', MAIN__astFCD[1]);
        printf("%s", MAIN__astFCD[1]->pchBuffer);
    */
    
    /* Solve */
    MAIN__vFCD();      /* Homework 3 */
    MAIN__vDM();       /* Homework 4 */
    MAIN__vMinimize(); /* Homework 5 */

    /* Write result to files */
    MAIN__vWriteH(MAIN__astFCD, "Tema3_Rel.txt");
    MAIN__vWriteH(MAIN__astMD, "Tema4_Rel.txt");
    MAIN__vWriteH(MAIN__astMinimized, "Tema5_Rel.txt");

    
    return(0);
}

static void
MAIN__vWriteH(STRING_tstString *astStrings[MAIN__nOutputs], const char *pcFilePath) {

    int iFD;
    uint8_t u8Idx;

    /*****************************************************/
    /*****************************************************/

    /* Write to file H3 */
    iFD = open(pcFilePath, O_CREAT|O_WRONLY);

    for(u8Idx = 0; u8Idx < MAIN__nOutputs; ++u8Idx) {

        STRING_i8StringAddChar('\n', astStrings[u8Idx]);
        write(iFD, astStrings[u8Idx]->pchBuffer, STRING_u32StringLength(astStrings[u8Idx]));
            
    }
    
    close(iFD);
}

static void
MAIN__vAlloc(void) {
    
    
    uint8_t u8Idx;

    /*****************************************************/
    /*****************************************************/

    for(u8Idx = 0; u8Idx < MAIN__nOutputs; ++u8Idx) {
        MAIN__astFCD[u8Idx] = STRING_stStringMakeWith((const char *)MAIN__au8OutputChar[u8Idx]);
        MAIN__astMD[u8Idx]  = STRING_stStringMakeWith((const char *)MAIN__au8OutputChar[u8Idx]);
        MAIN__astMinimized[u8Idx]  = STRING_stStringMakeWith((const char *)MAIN__au8OutputChar[u8Idx]);
    }

}

static void
MAIN__vParse(const char *pcChar) {

    /* File handler */
    int FD;

    uint8_t u8Byte;
    uint8_t u8Flag;

    uint8_t u8Tabs;

    uint8_t u8Line;
    uint8_t u8Column;

    u8Line   = 0;
    u8Column = 0;
    u8Flag   = 0;
    u8Tabs   = 0;
    
    /*****************************************************/
    /*****************************************************/

    FD = open(pcChar, O_RDONLY);

    if(FD < 0)
        printf("%s", "Err");
    
    for(; (read(FD, &u8Byte, 1)) != 0; ) {
        
        if(u8Tabs == (MAIN__nOutputs - 1u)) {
            if(u8Byte == MAIN__nEOL) {

                MAIN__au8OUT[u8Line][u8Column] = 0;

            } else {
                
                MAIN__au8OUT[u8Line][u8Column] = (u8Byte == MAIN__nOne);

                /* Skip the new line character */
                read(FD, &u8Byte, 1);
            }

            /* Reset column and parse the next line */
            u8Tabs   = 0;
            u8Column = 0;
            ++u8Line;
            
            continue;
        }

        /* If the line starts with a tab instead of a number */
        if(u8Byte == MAIN__nTab) {
            ++u8Tabs;
            if(u8Flag == 0)
                MAIN__au8OUT[u8Line][u8Column++] = 0;
            else
                u8Flag = 0;
        } else {
            
            /* Line started with a number */
            MAIN__au8OUT[u8Line][u8Column++] = (u8Byte == MAIN__nOne);
            
            /* Signal for the next character not to interpret it as 0 */
            u8Flag = 1;
        }
    }

    close(FD);

}

static void
MAIN__vFill(void) {

    uint8_t u8Idx;
    uint8_t u8Idxj;
    uint8_t u8Num;

    /* Skip the first line since it's all zeroes */
    u8Num = 1;
    
    /*****************************************************/
    /*****************************************************/

    for(u8Idx = 1; u8Idx < MAIN__nComb; ++u8Idx) {
        
        for(u8Idxj = 0; u8Idxj < MAIN__nInputs; ++u8Idxj) {

            MAIN__au8IN[u8Idx][u8Idxj] = ((1u & (u8Num >> ( (MAIN__nInputs - 1) - u8Idxj))) > 0); 
              
        }

        ++u8Num;
    }

}

static void 
MAIN__vFCD(void) {

    uint8_t u8IdxLine;
    uint8_t u8IdxCol;
    uint8_t u8Idx;
    uint8_t u8BarIdx;

    u8IdxCol = 0;

    /*****************************************************/
    /*****************************************************/

    /* [i][c] */
    for(u8IdxCol = 0; u8IdxCol < MAIN__nOutputs; ++u8IdxCol) {
        
        /* [i][c] */
        for(u8IdxLine = 0; u8IdxLine < MAIN__nComb; ++u8IdxLine) {
        
            if(MAIN__au8OUT[u8IdxLine][u8IdxCol] == 1u) {
                
                /* [c][j] & a b c d e f */
                for(u8Idx = 0; u8Idx < MAIN__nInputs; ++u8Idx) {

                    if(MAIN__au8IN[u8IdxLine][u8Idx] == 1u) {
                        
                        /* Append the input */
                        STRING_i8StringAddChar((MAIN__nA + u8Idx), MAIN__astFCD[u8IdxCol]);

                    } else {
                        
                        /* Append the \bar */
                        for(u8BarIdx = 0; u8BarIdx < (MAIN__u8LaTeXLen - 1); ++u8BarIdx) {

                            STRING_i8StringAddChar(MAIN__au8LateXBar[u8BarIdx], MAIN__astFCD[u8IdxCol]);
                        
                        } 

                        /* Append the negated input */
                        STRING_i8StringAddChar((MAIN__nA + u8Idx), MAIN__astFCD[u8IdxCol]);
                        STRING_i8StringAddChar(MAIN__au8LateXBar[MAIN__u8LaTeXLen - 1], MAIN__astFCD[u8IdxCol]);

                        /* LateX requires space between \bar elements */
                        STRING_i8StringAddChar(' ', MAIN__astFCD[u8IdxCol]);                 
                    }
                }
                STRING_i8StringAddChar('+', MAIN__astFCD[u8IdxCol]);
            }
            
        }
        
        /* Remove the last + character due to completion */
        STRING_i8StringRemoveChar(MAIN__astFCD[u8IdxCol]);

    }

}


static void
MAIN__vDM(void) {

    uint8_t u8IdxLine;
    uint8_t u8IdxCol;
    uint8_t u8IdxLateX;
    uint8_t u8IdxOctets;

    /*****************************************************/
    /*****************************************************/

    for(u8IdxCol = 0; u8IdxCol < MAIN__nOutputs; ++u8IdxCol) {
        
        /* Append the D mark */
        for(u8IdxLateX = 0; MAIN__au8LateXDMark[u8IdxLateX] != '$'; ++u8IdxLateX)
            STRING_i8StringAddChar(MAIN__au8LateXDMark[u8IdxLateX], MAIN__astMD[u8IdxCol]);

        /* Append the output */
        for(u8IdxOctets = 0, u8IdxLine = 0; u8IdxLine < MAIN__nComb; ++u8IdxLine) {
            
            if(MAIN__au8OUT[u8IdxLine][u8IdxCol] == 1u) {
                
                /* 64 = 100 to octal */
                if((u8IdxLine + 1) == 0x40) {

                    STRING_i8StringAddChar('7', MAIN__astMD[u8IdxCol]);

                    STRING_i8StringAddChar('7', MAIN__astMD[u8IdxCol]); 

                } else {
                    
                    /* Check the number of digits */
                    if( MAIN__u8DecToOctal128(u8IdxLine) >= 10u ) {
                        
                        STRING_i8StringAddChar(MAIN__Dig2(MAIN__u8DecToOctal128(u8IdxLine)), MAIN__astMD[u8IdxCol]);
                        STRING_i8StringAddChar(MAIN__Dig1(MAIN__u8DecToOctal128(u8IdxLine)), MAIN__astMD[u8IdxCol]);
                    
                    } else {

                        STRING_i8StringAddChar(MAIN__Dig1(MAIN__u8DecToOctal128(u8IdxLine)), MAIN__astMD[u8IdxCol]);
                    }
                    
                } 

                STRING_i8StringAddChar(',', MAIN__astMD[u8IdxCol]);
                STRING_i8StringAddChar(' ', MAIN__astMD[u8IdxCol]);
                MAIN__au16Octets[u8IdxCol][u8IdxOctets++] = MAIN__u8DecToOctal128(u8IdxLine);
            }
        }


        /* Remove the last 2 characters due to completion */
        STRING_i8StringRemoveChar(MAIN__astMD[u8IdxCol]);
        STRING_i8StringRemoveChar(MAIN__astMD[u8IdxCol]);

        ++u8IdxLateX;

        /* Append the rest of the characters */
        for(; u8IdxLateX < MAIN__u8LateXDLen; ++u8IdxLateX)
            STRING_i8StringAddChar(MAIN__au8LateXDMark[u8IdxLateX], MAIN__astMD[u8IdxCol]);

    }

}



static void
MAIN__vMinimize(void)
{ 

    uint32_t u32PrevLeadingLine;

    uint32_t u32Idx;

    uint32_t u32MinTerm;
    
    uint8_t u8Group;
    
    uint8_t u8Flag;

    /*****************************************************/
    /*****************************************************/


    for(MAIN__u32StrMinIdx = 0; MAIN__u32StrMinIdx < MAIN__nOutputs; ++MAIN__u32StrMinIdx) {
        
        /* Reset */
        MAIN__u32Idx       = 0;
        MAIN__i8Depth      = 0;
        MAIN__u32MaxMinIdx = 0;
        MAIN__u32Terms     = 0;
        MAIN__u32MinIdx    = 0;
        
        /* Reset */
        for(u32Idx = 0; u32Idx < MAIN__nMinSize; ++u32Idx) {

            MAIN__au32Used[u32Idx]          = 0;
            MAIN__au32AdjacentSerie[u32Idx] = 0;
            MAIN__au32Difference[u32Idx]    = 0;

            for(uint32_t u32J = 0; u32J < MAIN__nMinSize; ++u32J) {

                MAIN__au32Adjacents[u32Idx][u32J]     = 0;
                MAIN__au32Minimized[u32Idx][u32J]     = 0;
                MAIN__au32MinDifference[u32Idx][u32J] = 0;
                MAIN__au32MinBuf[u32Idx][u32J]        = 0;
                
            }
        
        }
        
        /* 
            4 6 [2] -> 000 1/0 ->~a~b~c d~f 
            11 [0] -> 001001 -> ~a~b c ~d~e f
            2 3 6 7 [1, 4] -> 000 /1/ -> ~a ~b ~c e 
            45 65 [20] -> 1/0101 -> a ~c d ~e f
            46 -> 100110 -> a ~b ~c d e ~f
            MAIN__astMinimized
        */

        /* Fill the adjancets matrix */
        MAIN__vFindAdjacents();
        
        for(; MAIN__u32Idx < MAIN__u32OctetsLen; ++MAIN__u32Idx) {

            
            /* Check if it has been used in a series; skip if yes */
            if(MAIN__au32Used[MAIN__au16Octets[MAIN__u32StrMinIdx][MAIN__u32Idx]] == 1u) 
                    continue;

            /* Check if it doesn't have any adjacents */
            if(MAIN__au32Adjacents[MAIN__au16Octets[MAIN__u32StrMinIdx][MAIN__u32Idx]][0] == 0) {
                /* Signal as used */
                MAIN__au32Used[MAIN__au16Octets[MAIN__u32StrMinIdx][MAIN__u32Idx]] = 1u;
                MAIN__au32Minimized[MAIN__u32MaxMinIdx++][0] = MAIN__au16Octets[MAIN__u32StrMinIdx][MAIN__u32Idx];

                continue;
            }

            
            /* Clear the buffer for the next series */
            for(u32Idx = 0; u32Idx < MAIN__nMinSize; ++u32Idx)
                MAIN__au32AdjacentSerie[u32Idx] = 0;

            MAIN__u32SerieIdx                            = 0;
            u32PrevLeadingLine                           = MAIN__u32MinIdx;
            MAIN__au32AdjacentSerie[MAIN__u32SerieIdx++] = MAIN__au16Octets[MAIN__u32StrMinIdx][MAIN__u32Idx];
            
        
            #define MAIN__au32Adjacents(n, i) MAIN__au32Adjacents[MAIN__au16Octets[MAIN__u32StrMinIdx][n]][i]
                    
            /* Do all possible combinations */
            for(u32Idx = 0; MAIN__au32Adjacents(MAIN__u32Idx, u32Idx) != 0; ++u32Idx) {
                        
                        
                uint32_t u32PrimeAdj;
                
                u32PrimeAdj                                  = MAIN__au32Adjacents(MAIN__u32Idx, u32Idx);
                /* Construct the primary group */
                MAIN__au32AdjacentSerie[MAIN__u32SerieIdx++] = u32PrimeAdj;
                
                /* MAIN__i8Depth 1 since we have the primary group */
                MAIN__i8Depth = 1u;
                
                /* Iterate through each element up to 2^n, where n = MAIN__i8Depth */
                for(int32_t i32TermIdx = 0; (i32TermIdx < MAIN__Group(MAIN__i8Depth)) && (MAIN__i8Depth > 0); ++i32TermIdx) {
                    
                    /* If multiple of 2^n => leading element */
                    if(MAIN__boMultiple2(MAIN__u32SerieIdx) == 0) {
                        
                        /* By default 1 for when there are no more leading elements and it can skip the loop and save the minimization */
                        uint8_t u8LeadingUsed = 1u;

                        /* Check for values on the right of the one used in the primary group */
                        uint32_t u8LeadingIdx = u32Idx + 1u;
                        
                        int8_t MAIN__i8Depth_copy;
                
                        /*
                        * Find a leading element that wasn't used before in previous minimization
                        * The leading element of each group dictates the bit location
                        * They are located at positions multiple of 2^n
                        * Group 1(x2) {{x1, y1}, x2, y2}                   = 2
                        * Group 2(x3) {{{x1, y2}, x2, y2}, x3, x4, y3, y4} = 4
                        */
                        for(; MAIN__au32Adjacents[MAIN__au16Octets[MAIN__u32StrMinIdx][MAIN__u32Idx]][u8LeadingIdx] != 0; ++u8LeadingIdx) {
                            
                            /* Reset */
                            u8LeadingUsed      = 0;
                            MAIN__i8Depth_copy = MAIN__i8Depth;

                            /* Check if the leading term has been used on the same position before */
                            for(uint32_t u32PrevLeadingIdx = u32PrevLeadingLine; u32PrevLeadingIdx <= MAIN__u32MinIdx; ++u32PrevLeadingIdx) {
                                
                                if( (MAIN__au32MinBuf[u32PrevLeadingIdx][MAIN__u32SerieIdx] == MAIN__au32Adjacents[MAIN__au16Octets[MAIN__u32StrMinIdx][MAIN__u32Idx]][u8LeadingIdx]) ){
                                        
                                    /* Check if the prime term was the same - we can have same leading term but different prime */
                                    if( MAIN__au32MinBuf[u32PrevLeadingIdx][1] == u32PrimeAdj) {
                                        /* Same prime and leading term */
                                        u8LeadingUsed = 1u;
                                        break;
                                        }
                                }   
                            }
                            
                            /* No point if the previous loop found it */
                            if(u8LeadingUsed != 1u) {
                                /* Check if the leading term is being used in the current serie */
                                for(; MAIN__i8Depth_copy != 0; --MAIN__i8Depth_copy) {
                                    if(MAIN__au32AdjacentSerie[MAIN__Group(MAIN__i8Depth_copy)] == MAIN__au32Adjacents[MAIN__au16Octets[MAIN__u32StrMinIdx][MAIN__u32Idx]][u8LeadingIdx]) {
                                        u8LeadingUsed = 1u;
                                        break;
                                    }
                                }
                            }
                                    
                            /* Found one that wasn't used */
                            if(u8LeadingUsed == 0) {
                                /* Get the type of difference */
                                MAIN__au32Difference[MAIN__i8Depth] = MAIN__OctDiff(MAIN__au16Octets[MAIN__u32StrMinIdx][MAIN__u32Idx], MAIN__au32Adjacents[MAIN__au16Octets[MAIN__u32StrMinIdx][MAIN__u32Idx]][u8LeadingIdx]);
                                /* Add the leading element */
                                MAIN__au32AdjacentSerie[MAIN__u32SerieIdx++] = MAIN__au32Adjacents[MAIN__au16Octets[MAIN__u32StrMinIdx][MAIN__u32Idx]][u8LeadingIdx];
                                break;
                            }
                            
                        }
                        
                        /* No leading numbers could be found => decrease MAIN__i8Depth */
                        if(u8LeadingUsed == 1u) {
                            
                            /* If it gets into here it implies that we cannot minimize any further */
                            /* Decrease the MAIN__i8Depth and save the elements up to it */      
                            MAIN__vSaveMin();
                            i32TermIdx = -1;
                            continue;
                        } 
                        
                    } else {
                        
                        /* Non leading terms */
                        uint32_t u32NonLeadIdx;
                        uint8_t u8ContainsNonLeading;
                        
                        u8ContainsNonLeading = 0;

                        /* Find an adjacent term for the current term that is not contained within the current series */
                        for(u32NonLeadIdx = 0; MAIN__au32Adjacents[MAIN__au32AdjacentSerie[i32TermIdx]][u32NonLeadIdx] != 0; ++u32NonLeadIdx) {
                            
                            
                            uint32_t u32AdjTerm;
                            uint32_t u32Oct;
                            
                            u8ContainsNonLeading = 0;
                            /* Adjacent term of current term */
                            u32AdjTerm           = MAIN__au32Adjacents[MAIN__au32AdjacentSerie[i32TermIdx]][u32NonLeadIdx];
                            /* Difference */
                            u32Oct               = MAIN__OctDiff(u32AdjTerm, MAIN__au32AdjacentSerie[i32TermIdx]);
                            
                            for(int8_t i8DiffIdx = 0; i8DiffIdx <= (MAIN__i8Depth + 1); ++i8DiffIdx) {
                                
                                /* Check if the difference is the same as the leading term */
                                if(MAIN__au32Difference[i8DiffIdx] == u32Oct){

                                    u8ContainsNonLeading = 1;

                                    /* Check if the term is already within the current serie */
                                    for(uint32_t u32IdxC = 0; u32IdxC < MAIN__u32SerieIdx; ++u32IdxC) {

                                        if(MAIN__au32AdjacentSerie[u32IdxC] == u32AdjTerm) {
                                            
                                            /* Already exists in current serie */
                                            u8ContainsNonLeading = 0;

                                            break;
                                        }
                                    }
                                    
                                    break;
                                }   
                            }
                            
                            if(u8ContainsNonLeading == 1) {

                                /* Adjcent term added */
                                MAIN__au32AdjacentSerie[MAIN__u32SerieIdx++] = MAIN__au32Adjacents[MAIN__au32AdjacentSerie[i32TermIdx]][u32NonLeadIdx];
                                break;
                            }
                                
                            
                        }

                        /* No elements - decrease MAIN__i8Depth and save the minimization */
                        if(u8ContainsNonLeading == 0) {
                            
                            
                            MAIN__vSaveMin();

                            /* Reset index due to depth decrease */
                            i32TermIdx = -1;

                            continue;
                            

                        }

                    }
                    
                    /* Increase the MAIN__i8Depth */
                    if(MAIN__boMultiple2(MAIN__u32SerieIdx) == 0) {
                        
                        ++MAIN__i8Depth;
                        
                        /* Reset and find new elements for next MAIN__i8Depth */
                        i32TermIdx = -1;
                    }
                    
                }
                
            }

            /* Get the different between main term and prime to avoid overflow*/
            MAIN__au32MinDifference[MAIN__u32MaxMinIdx][0] =
                MAIN__OctDiff(MAIN__au16Octets[MAIN__u32StrMinIdx][MAIN__u32Idx], MAIN__au32Minimized[MAIN__u32MaxMinIdx][1u]);   

            /* Get the difference between the rest of the terms */
            for(u32Idx = 2; MAIN__au32Minimized[MAIN__u32MaxMinIdx][u32Idx] != 0; ++u32Idx) {
                if(MAIN__boMultiple2(u32Idx) == 0) {
                    
                    /* Get the power of 2 */
                    for(; u32Idx >>= 1u; )
                        ++u8Group;
                    u32Idx = 1u;
                    u32Idx <<= u8Group;
                    MAIN__au32MinDifference[MAIN__u32MaxMinIdx][u8Group] = 
                        MAIN__OctDiff(MAIN__au16Octets[MAIN__u32StrMinIdx][MAIN__u32Idx], MAIN__au32Minimized[MAIN__u32MaxMinIdx][u32Idx]);

                    u8Group = 0;
                }
                    
            }
                
            /* Set the terms that were used in the longest series as used */
            for(u32Idx = 0; MAIN__au32Minimized[MAIN__u32MaxMinIdx][u32Idx] != 0; ++u32Idx)
                MAIN__au32Used[MAIN__au32Minimized[MAIN__u32MaxMinIdx][u32Idx]] = 1u;
            
                
                
            /* Reset for next terms */
            MAIN__u32Terms = 0;
            ++MAIN__u32MaxMinIdx;

        }

        /* Min matrix filled */
        for(uint32_t u32J = 0; u32J < (MAIN__u32MaxMinIdx - 1); ++u32J) {
            u32MinTerm = MAIN__au32Minimized[u32J][0];
            u8Flag     = 0;
            /* Check if it's a single term in the series */
           
                /* abcdef */
                for(int32_t i32K = (MAIN__nInputs - 1); i32K >= 0; --i32K) {
                    /* 010101*/
                    for(uint32_t u32O = 0; MAIN__au32MinDifference[u32J][u32O] != 0; ++u32O) {
                        if( ((1u << i32K) & MAIN__u8OctToDec(MAIN__au32MinDifference[u32J][u32O])) ) {
                            u8Flag = 1u;
                            break;
                        }
                    }
                    
                    /* Letter not reduced */
                    if(u8Flag != 1u) {
                        
                        
                        if( ((1u << i32K) & MAIN__u8OctToDec(u32MinTerm)) > 0) {
                            /*
                                A + 6 - (k + 1)
                            */
                            /* Not negated */    
                            STRING_i8StringAddChar((MAIN__nA + (MAIN__nInputs - (i32K + 1))), MAIN__astMinimized[MAIN__u32StrMinIdx]);

                        } else {
                            /* Negated */

                            /* Append the \bar */
                            for(uint8_t u8BarIdx = 0; u8BarIdx < (MAIN__u8LaTeXLen - 1); ++u8BarIdx) {

                                STRING_i8StringAddChar(MAIN__au8LateXBar[u8BarIdx], MAIN__astMinimized[MAIN__u32StrMinIdx]);
                            
                            } 

                            /* Append the negated input */
                            STRING_i8StringAddChar((MAIN__nA + (MAIN__nInputs - (i32K + 1))), MAIN__astMinimized[MAIN__u32StrMinIdx]);
                            STRING_i8StringAddChar(MAIN__au8LateXBar[MAIN__u8LaTeXLen - 1u], MAIN__astMinimized[MAIN__u32StrMinIdx]);

                            /* LateX requires space between \bar elements */
                            STRING_i8StringAddChar(' ', MAIN__astMinimized[MAIN__u32StrMinIdx]); 
                        }

                        
                    } 
                    u8Flag  = 0;
                    
                }
            
            STRING_i8StringAddChar('+', MAIN__astMinimized[MAIN__u32StrMinIdx]); 
       
        }
        
    }
    
    
}

static void
MAIN__vFindAdjacents(void) {
    
    uint32_t u32Idx;
    uint32_t u32Jdx;
    uint32_t u32Adj;
    uint32_t u32AdjIdx;

    u32AdjIdx = 0;

    /*****************************************************/
    /*****************************************************/


    for(u32Idx = 0; u32Idx < MAIN__u32OctetsLen; ++u32Idx) {
        
        for(u32Jdx = u32Idx + 1; u32Jdx < MAIN__u32OctetsLen; ++u32Jdx) {
        
            u32Adj = MAIN__u8OctToDec(MAIN__au16Octets[MAIN__u32StrMinIdx][u32Idx]) ^ MAIN__u8OctToDec(MAIN__au16Octets[MAIN__u32StrMinIdx][u32Jdx]);

            /* Check if power of 2 */    
            if(MAIN__boMultiple2(u32Adj) == 0) {
                MAIN__au32Adjacents[MAIN__au16Octets[MAIN__u32StrMinIdx][u32Idx]][u32AdjIdx++] = MAIN__au16Octets[MAIN__u32StrMinIdx][u32Jdx];
            }
        }

        u32AdjIdx = 0;
    
    }

}

static void 
MAIN__vSaveMin(void) {
    
    uint32_t u32Idx;

    /*****************************************************/
    /*****************************************************/

    /* Longest series? */
    if(MAIN__Group(MAIN__i8Depth) >= MAIN__u32Terms) {
        MAIN__u32Terms = MAIN__Group(MAIN__i8Depth);

        for(u32Idx = MAIN__u32Terms; u32Idx < MAIN__nMinSize; ++u32Idx)
            MAIN__au32Minimized[MAIN__u32MaxMinIdx][u32Idx] = 0;
    }
        

    for(u32Idx = 0; u32Idx < MAIN__Group(MAIN__i8Depth); ++u32Idx) {
        
        MAIN__au32MinBuf[MAIN__u32MinIdx][u32Idx] = MAIN__au32AdjacentSerie[u32Idx];

        if(MAIN__Group(MAIN__i8Depth) == MAIN__u32Terms) 
            MAIN__au32Minimized[MAIN__u32MaxMinIdx][u32Idx] = MAIN__au32AdjacentSerie[u32Idx];

        //MAIN__au32Used[MAIN__au32AdjacentSerie[u32Idx]] = 1u;
    }

    /* Clear the difference buffer */
    for(u32Idx = 0; u32Idx <= MAIN__i8Depth; ++u32Idx)
        MAIN__au32Difference[u32Idx] = 0;
    
    
    
    /* Lower the MAIN__i8Depth */   
    --MAIN__i8Depth; 

    /* No more terms - can reset */
    if(MAIN__i8Depth < 0)
        MAIN__i8Depth = 0;

    /* Clear the matrix for the next minimization */
    for(u32Idx = MAIN__u32SerieIdx; u32Idx >= MAIN__Group(MAIN__i8Depth); --u32Idx)
        MAIN__au32AdjacentSerie[u32Idx] = 0;


    MAIN__u32SerieIdx = MAIN__Group(MAIN__i8Depth);

    ++MAIN__u32MinIdx;

}


/* TODO: Make it minimize a single array passed as parameter */

// static void
// MAIN__vMinimizeSingle(void)
// { 

//     uint32_t u32PrevLeadingLine;

//     uint32_t u32Idx;

//     uint32_t u32MinTerm;
    
//     uint8_t u8Group;
    
//     /*****************************************************/
//     /*****************************************************/

//     /* Fill the adjancets matrix */
//     MAIN__vFindAdjacents();
    
//     for(; MAIN__u32Idx < MAIN__u32OctetsLen; ++MAIN__u32Idx) {

        
//         /* Check if it has been used in a series; skip if yes */
//         if(MAIN__au32Used[MAIN__au32Octets[MAIN__u32Idx]] == 1u) 
//                 continue;

//         /* Check if it doesn't have any adjacents */
//         if(MAIN__au32Adjacents[MAIN__au32Octets[MAIN__u32Idx]][0] == 0) {
//             /* Signal as used */
//             MAIN__au32Used[MAIN__au32Octets[MAIN__u32Idx]] = 1u;
//             MAIN__au32Minimized[MAIN__u32MaxMinIdx++][0] = MAIN__au32Octets[MAIN__u32Idx];

//             continue;
//         }

        
//         /* Clear the buffer for the next series */
//         for(u32Idx = 0; u32Idx < MAIN__nMinSize; ++u32Idx)
//             MAIN__au32AdjacentSerie[u32Idx] = 0;

//         MAIN__u32SerieIdx                            = 0;
//         u32PrevLeadingLine                           = MAIN__u32MinIdx;
//         MAIN__au32AdjacentSerie[MAIN__u32SerieIdx++] = MAIN__au32Octets[MAIN__u32Idx];
        
    
//         #define MAIN__au32Adjacents(n, i) MAIN__au32Adjacents[MAIN__au32Octets[n]][i]
                
//          /* Do all possible combinations */
//         for(u32Idx = 0; MAIN__au32Adjacents(MAIN__u32Idx, u32Idx) != 0; ++u32Idx) {
                    
                    
//             uint32_t u32PrimeAdj;
            
//             u32PrimeAdj                                  = MAIN__au32Adjacents(MAIN__u32Idx, u32Idx);
//             /* Construct the primary group */
//             MAIN__au32AdjacentSerie[MAIN__u32SerieIdx++] = u32PrimeAdj;
            
//             /* MAIN__i8Depth 1 since we have the primary group */
//             MAIN__i8Depth = 1u;
            
//             /* Iterate through each element up to 2^n, where n = MAIN__i8Depth */
//             for(int32_t i32TermIdx = 0; (i32TermIdx < MAIN__Group(MAIN__i8Depth)) && (MAIN__i8Depth > 0); ++i32TermIdx) {
                
//                 /* If multiple of 2^n => leading element */
//                 if(MAIN__boMultiple2(MAIN__u32SerieIdx) == 0) {
                    
//                     /* By default 1 for when there are no more leading elements and it can skip the loop and save the minimization */
//                     uint8_t u8LeadingUsed = 1u;

//                     /* Check for values on the right of the one used in the primary group */
//                     uint32_t u8LeadingIdx = u32Idx + 1u;
                    
//                     int8_t MAIN__i8Depth_copy;
            
//                     /*
//                     * Find a leading element that wasn't used before in previous minimization
//                     * The leading element of each group dictates the bit location
//                     * They are located at positions multiple of 2^n
//                     * Group 1(x2) {{x1, y1}, x2, y2}                   = 2
//                     * Group 2(x3) {{{x1, y2}, x2, y2}, x3, x4, y3, y4} = 4
//                     */
//                     for(; MAIN__au32Adjacents[MAIN__au32Octets[MAIN__u32Idx]][u8LeadingIdx] != 0; ++u8LeadingIdx) {
                        
//                         /* Reset */
//                         u8LeadingUsed      = 0;
//                         MAIN__i8Depth_copy = MAIN__i8Depth;

//                         /* Check if the leading term has been used on the same position before */
//                         for(uint32_t u32PrevLeadingIdx = u32PrevLeadingLine; u32PrevLeadingIdx <= MAIN__u32MinIdx; ++u32PrevLeadingIdx) {
                            
//                             if( (MAIN__au32MinBuf[u32PrevLeadingIdx][MAIN__u32SerieIdx] == MAIN__au32Adjacents[MAIN__au32Octets[MAIN__u32Idx]][u8LeadingIdx]) ){
                                    
//                                 /* Check if the prime term was the same - we can have same leading term but different prime */
//                                 if( MAIN__au32MinBuf[u32PrevLeadingIdx][1] == u32PrimeAdj) {
//                                     /* Same prime and leading term */
//                                     u8LeadingUsed = 1u;
//                                     break;
//                                     }
//                             }   
//                         }
                        
//                         /* No point if the previous loop found it */
//                         if(u8LeadingUsed != 1u) {
//                             /* Check if the leading term is being used in the current serie */
//                             for(; MAIN__i8Depth_copy != 0; --MAIN__i8Depth_copy) {
//                                 if(MAIN__au32AdjacentSerie[MAIN__Group(MAIN__i8Depth_copy)] == MAIN__au32Adjacents[MAIN__au32Octets[MAIN__u32Idx]][u8LeadingIdx]) {
//                                     u8LeadingUsed = 1u;
//                                     break;
//                                 }
//                             }
//                         }
                                
//                         /* Found one that wasn't used */
//                         if(u8LeadingUsed == 0) {
//                             /* Get the type of difference */
//                             MAIN__au32Difference[MAIN__i8Depth] = MAIN__OctDiff(MAIN__au32Octets[MAIN__u32Idx], MAIN__au32Adjacents[MAIN__au32Octets[MAIN__u32Idx]][u8LeadingIdx]);
//                             /* Add the leading element */
//                             MAIN__au32AdjacentSerie[MAIN__u32SerieIdx++] = MAIN__au32Adjacents[MAIN__au32Octets[MAIN__u32Idx]][u8LeadingIdx];
//                             break;
//                         }
                        
//                     }
                    
//                     /* No leading numbers could be found => decrease MAIN__i8Depth */
//                     if(u8LeadingUsed == 1u) {
                        
//                         /* If it gets into here it implies that we cannot minimize any further */
//                         /* Decrease the MAIN__i8Depth and save the elements up to it */      
//                         MAIN__vSaveMin();
//                         i32TermIdx = -1;
//                         continue;
//                     } 
                    
//                 } else {
                    
//                     /* Non leading terms */
//                     uint32_t u32NonLeadIdx;
//                     uint8_t u8ContainsNonLeading;
                    
//                     u8ContainsNonLeading = 0;

//                     /* Find an adjacent term for the current term that is not contained within the current series */
//                     for(u32NonLeadIdx = 0; MAIN__au32Adjacents[MAIN__au32AdjacentSerie[i32TermIdx]][u32NonLeadIdx] != 0; ++u32NonLeadIdx) {
                        
                        
//                         uint32_t u32AdjTerm;
//                         uint32_t u32Oct;
                        
//                         u8ContainsNonLeading = 0;
//                         /* Adjacent term of current term */
//                         u32AdjTerm           = MAIN__au32Adjacents[MAIN__au32AdjacentSerie[i32TermIdx]][u32NonLeadIdx];
//                         /* Difference */
//                         u32Oct               = MAIN__OctDiff(u32AdjTerm, MAIN__au32AdjacentSerie[i32TermIdx]);
                        
//                         for(int8_t i8DiffIdx = 0; i8DiffIdx <= (MAIN__i8Depth + 1); ++i8DiffIdx) {
                            
//                             /* Check if the difference is the same as the leading term */
//                             if(MAIN__au32Difference[i8DiffIdx] == u32Oct){

//                                 u8ContainsNonLeading = 1;

//                                 /* Check if the term is already within the current serie */
//                                 for(uint32_t u32IdxC = 0; u32IdxC < MAIN__u32SerieIdx; ++u32IdxC) {

//                                     if(MAIN__au32AdjacentSerie[u32IdxC] == u32AdjTerm) {
                                        
//                                         /* Already exists in current serie */
//                                         u8ContainsNonLeading = 0;

//                                         break;
//                                     }
//                                 }
                                
//                                 break;
//                             }   
//                         }
                        
//                         if(u8ContainsNonLeading == 1) {

//                             /* Adjcent term added */
//                             MAIN__au32AdjacentSerie[MAIN__u32SerieIdx++] = MAIN__au32Adjacents[MAIN__au32AdjacentSerie[i32TermIdx]][u32NonLeadIdx];
//                             break;
//                         }
                            
                        
//                     }

//                     /* No elements - decrease MAIN__i8Depth and save the minimization */
//                     if(u8ContainsNonLeading == 0) {
                        
                        
//                         MAIN__vSaveMin();

//                         /* Reset index due to depth decrease */
//                         i32TermIdx = -1;

//                         continue;
                        

//                     }

//                 }
                
//                 /* Increase the MAIN__i8Depth */
//                 if(MAIN__boMultiple2(MAIN__u32SerieIdx) == 0) {
                    
//                     ++MAIN__i8Depth;
                    
//                     /* Reset and find new elements for next MAIN__i8Depth */
//                     i32TermIdx = -1;
//                 }
                
//             }
            
//         }

//         /* Get the different between main term and prime to avoid overflow*/
//         MAIN__au32MinDifference[MAIN__u32MaxMinIdx][0] =
//             MAIN__OctDiff(MAIN__au32Octets[MAIN__u32Idx], MAIN__au32Minimized[MAIN__u32MaxMinIdx][1u]);   

//         /* Get the difference between the rest of the terms */
//         for(u32Idx = 2; MAIN__au32Minimized[MAIN__u32MaxMinIdx][u32Idx] != 0; ++u32Idx) {
//             if(MAIN__boMultiple2(u32Idx) == 0) {
                
//                 /* Get the power of 2 */
//                 for(; u32Idx >>= 1u; )
//                     ++u8Group;
//                 u32Idx = 1u;
//                 u32Idx <<= u8Group;
//                 MAIN__au32MinDifference[MAIN__u32MaxMinIdx][u8Group] = 
//                     MAIN__OctDiff(MAIN__au32Octets[MAIN__u32Idx], MAIN__au32Minimized[MAIN__u32MaxMinIdx][u32Idx]);

//                 u8Group = 0;
//             }
                
//         }
            
//         /* Set the terms that were used in the longest series as used */
//         for(u32Idx = 0; MAIN__au32Minimized[MAIN__u32MaxMinIdx][u32Idx] != 0; ++u32Idx)
//             MAIN__au32Used[MAIN__au32Minimized[MAIN__u32MaxMinIdx][u32Idx]] = 1u;
        
            
            
//         /* Reset for next terms */
//         MAIN__u32Terms = 0;
//         ++MAIN__u32MaxMinIdx;

//     }
// }


// static void
// MAIN__vFindAdjacentsSingle(void) {
    
//     uint32_t u32Idx;
//     uint32_t u32Jdx;
//     uint32_t u32Adj;
//     uint32_t u32AdjIdx;

//     u32AdjIdx = 0;

//     /*****************************************************/
//     /*****************************************************/


//     for(u32Idx = 0; u32Idx < MAIN__u32OctetsLen; ++u32Idx) {
        
//         for(u32Jdx = u32Idx + 1; u32Jdx < MAIN__u32OctetsLen; ++u32Jdx) {
        
//             u32Adj = MAIN__u8OctToDec(MAIN__au32Octets[u32Idx]) ^ MAIN__u8OctToDec(MAIN__au32Octets[u32Jdx]);

//             /* Check if power of 2 */    
//             if(MAIN__boMultiple2(u32Adj) == 0) {
//                 MAIN__au32Adjacents[MAIN__au32Octets[u32Idx]][u32AdjIdx++] = MAIN__au32Octets[u32Jdx];
//             }
//         }

//         u32AdjIdx = 0;
    
//     }

// }
