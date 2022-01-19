#include <stdio.h>
#include <unistd.h>
#include <stdint.h>


/* Scrie aici sirul, intre { } si cu virgula intre */
uint32_t MAIN__au32Octets[] = { };

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



uint32_t MAIN__u32OctetsLen = sizeof(MAIN__au32Octets)/sizeof(uint32_t);


static void
MAIN__vMinimizeSingle(uint32_t *, uint32_t);

static void
MAIN__vFindAdjacentsSingle(uint32_t *, uint32_t);

static void 
MAIN__vSaveMin(void);


int main(void)
{
    
    MAIN__vMinimizeSingle(MAIN__au32Octets, MAIN__u32OctetsLen);
    printf("\n%s", "Done!");

    return(0);
}



static void
MAIN__vMinimizeSingle(uint32_t *au32Octets, uint32_t u32OctetsLen) { 

    uint32_t u32PrevLeadingLine;

    uint32_t u32Idx;

    
    uint8_t u8Group;
    
    /*****************************************************/
    /*****************************************************/

    /* Fill the adjancets matrix */
    MAIN__vFindAdjacentsSingle(au32Octets, u32OctetsLen);
    
    for(; MAIN__u32Idx < u32OctetsLen; ++MAIN__u32Idx) {

        
        /* Check if it has been used in a series; skip if yes */
        if(MAIN__au32Used[au32Octets[MAIN__u32Idx]] == 1u) 
                continue;

        /* Check if it doesn't have any adjacents */
        if(MAIN__au32Adjacents[au32Octets[MAIN__u32Idx]][0] == 0) {
            /* Signal as used */
            MAIN__au32Used[au32Octets[MAIN__u32Idx]] = 1u;
            MAIN__au32Minimized[MAIN__u32MaxMinIdx++][0] = au32Octets[MAIN__u32Idx];

            continue;
        }

        
        /* Clear the buffer for the next series */
        for(u32Idx = 0; u32Idx < MAIN__nMinSize; ++u32Idx)
            MAIN__au32AdjacentSerie[u32Idx] = 0;

        MAIN__u32SerieIdx                            = 0;
        u32PrevLeadingLine                           = MAIN__u32MinIdx;
        MAIN__au32AdjacentSerie[MAIN__u32SerieIdx++] = au32Octets[MAIN__u32Idx];
        
    
        #define MAIN__au32Adjacents(n, i) MAIN__au32Adjacents[au32Octets[n]][i]
                
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
                    for(; MAIN__au32Adjacents[au32Octets[MAIN__u32Idx]][u8LeadingIdx] != 0; ++u8LeadingIdx) {
                        
                        /* Reset */
                        u8LeadingUsed      = 0;
                        MAIN__i8Depth_copy = MAIN__i8Depth;

                        /* Check if the leading term has been used on the same position before */
                        for(uint32_t u32PrevLeadingIdx = u32PrevLeadingLine; u32PrevLeadingIdx <= MAIN__u32MinIdx; ++u32PrevLeadingIdx) {
                            
                            if( (MAIN__au32MinBuf[u32PrevLeadingIdx][MAIN__u32SerieIdx] == MAIN__au32Adjacents[au32Octets[MAIN__u32Idx]][u8LeadingIdx]) ){
                                    
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
                                if(MAIN__au32AdjacentSerie[MAIN__Group(MAIN__i8Depth_copy)] == MAIN__au32Adjacents[au32Octets[MAIN__u32Idx]][u8LeadingIdx]) {
                                    u8LeadingUsed = 1u;
                                    break;
                                }
                            }
                        }
                                
                        /* Found one that wasn't used */
                        if(u8LeadingUsed == 0) {
                            /* Get the type of difference */
                            MAIN__au32Difference[MAIN__i8Depth] = MAIN__OctDiff(au32Octets[MAIN__u32Idx], MAIN__au32Adjacents[au32Octets[MAIN__u32Idx]][u8LeadingIdx]);
                            /* Add the leading element */
                            MAIN__au32AdjacentSerie[MAIN__u32SerieIdx++] = MAIN__au32Adjacents[au32Octets[MAIN__u32Idx]][u8LeadingIdx];
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
            MAIN__OctDiff(au32Octets[MAIN__u32Idx], MAIN__au32Minimized[MAIN__u32MaxMinIdx][1u]);   

        /* Get the difference between the rest of the terms */
        for(u32Idx = 2; MAIN__au32Minimized[MAIN__u32MaxMinIdx][u32Idx] != 0; ++u32Idx) {
            if(MAIN__boMultiple2(u32Idx) == 0) {
                
                /* Get the power of 2 */
                for(; u32Idx >>= 1u; )
                    ++u8Group;
                u32Idx = 1u;
                u32Idx <<= u8Group;
                MAIN__au32MinDifference[MAIN__u32MaxMinIdx][u8Group] = 
                    MAIN__OctDiff(au32Octets[MAIN__u32Idx], MAIN__au32Minimized[MAIN__u32MaxMinIdx][u32Idx]);

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


    for(uint32_t u32I = 0; u32I < MAIN__u32MaxMinIdx; ++u32I) {
        for(uint32_t u32J = 0; MAIN__au32Minimized[u32I][u32J] != 0; ++u32J)
            printf("%d ", MAIN__au32Minimized[u32I][u32J]);

        printf("%s[ ", "\t\t");
        for(uint32_t u32K = 0; MAIN__au32MinDifference[u32I][u32K] != 0; ++u32K)
            printf("%d ", MAIN__au32MinDifference[u32I][u32K]);

        printf("%s\n", "]");
    }
}


static void
MAIN__vFindAdjacentsSingle(uint32_t *au32Octets, uint32_t u32OctetsLen) {
    
    uint32_t u32Idx;
    uint32_t u32Jdx;
    uint32_t u32Adj;
    uint32_t u32AdjIdx;

    u32AdjIdx = 0;

    /*****************************************************/
    /*****************************************************/


    for(u32Idx = 0; u32Idx < u32OctetsLen; ++u32Idx) {
        
        for(u32Jdx = u32Idx + 1; u32Jdx < u32OctetsLen; ++u32Jdx) {
        
            u32Adj = MAIN__u8OctToDec(au32Octets[u32Idx]) ^ MAIN__u8OctToDec(au32Octets[u32Jdx]);

            /* Check if power of 2 */    
            if(MAIN__boMultiple2(u32Adj) == 0) {
                MAIN__au32Adjacents[au32Octets[u32Idx]][u32AdjIdx++] = au32Octets[u32Jdx];
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


