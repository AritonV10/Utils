#include <stdio.h>
#include <stdint.h>


/****************************************************************************/
/****************************************************************************/

#define AND(a, b) ((a) & (b))
#define OR(a, b)  ((a) | (b))
#define NEG(a)    ( ( (~(a)) & 0x01u) )


/* 
 * Modifica mai jos cum arata functia J_a, K_a, J_b, J_k si Z pe care o ai
 * AND = SI
 * OR = SAU
 * NEG = Negatie
 * Operatorii se aplica pentru expresia din stanga si dreapta, ex OR(A, B)
 * AB + C => OR(AND(A, B), C)
 * A~B + AB => OR(AND(A, NEG(B), AND(A, B))
 * AB + X  => OR(AND(A, B), X)
 */
#define J_A(X, A, B) 
#define K_A(X, A, B) 

#define J_B(X, A, B) 
#define K_B(X, A, B) 

#define Z(X, A, B) 

/* Starea initializa precizata in problema, [0, 1, 2, 3] */
static const uint8_t StareInitiala = ;

/* Setul X din problema */
static const uint8_t Intrare[] = { };

/****************************************************************************/
/****************************************************************************/


#define MAIN__nA (1u)
#define MAIN__nB (0)

#define JKA_J(X, A, B) J_A(X, A, B)
#define JKA_K(X, A, B) K_A(X, A, B)

#define JKB_J(X, A, B) J_B(X, A, B)
#define JKB_K(X, A, B) K_B(X, A, B)

#define MAIN__nStateA(a) ( ((a) & 0x02u) >> 1u )
#define MAIN__nStateB(a) ( (a) & 0x01u )

#define MAIN__nGray(a) (( (a) & 0x02u) | ( (( (a) & 0x02u ) >> 1u) ^ ( (a) & 0x01u ) ) )
#define MAIN__nGTB(a) ((a) ^ ((a) >> 1u))

/****************************************************************************/
/****************************************************************************/


typedef uint8_t(*JK_Func)(uint8_t, uint8_t, uint8_t);

typedef struct {
    
    uint8_t A;
    uint8_t B;
    
} MAIN__tstGray;

typedef struct {
    uint8_t J;
    uint8_t K;
} MAIN__tstJK;


typedef enum MAIN__Tag_JKState {
    MAIN__nSET,     /* 1  */
    MAIN__nUNSET,   /* 0  */
    MAIN__nCURRENT, /* Q  */
    MAIN__nRESET    /* ~Q */
}MAIN__tenJKState;

typedef struct {
    MAIN__tstJK JK;
    MAIN__tenJKState enState;
}MAIN__tstJKState;

/****************************************************************************/
/****************************************************************************/

/* JK Table */
static const MAIN__tstJKState MAIN__astJKTable[] = {
    { {0, 0}, MAIN__nCURRENT},
    { {0, 1}, MAIN__nUNSET},
    { {1, 0}, MAIN__nSET},
    { {1, 1}, MAIN__nRESET}
};

static uint8_t MAIN__au8AF[2][4];
static uint8_t MAIN__au8BF[2][4];

/* Gray code */
static const MAIN__tstGray MAIN__astGray[] = { 
    {0, 0}, {0, 1}, {1, 1}, {1, 0} 
}; 

static uint8_t MAIN__au8JKStates[4][2];

/* f(x, A, B) = [1, 0] */
static MAIN__tstJK MAIN__ast32JK_A[2][4];

/* 00 01 11 10 */
static MAIN__tstJK MAIN__ast32JK_B[2][4]; 

static const uint32_t MAIN__u32InLen = sizeof(Intrare)/sizeof(uint8_t);

static uint8_t MAIN__au8Output[32];

/****************************************************************************/
/****************************************************************************/

static uint8_t
MAIN__u8JKA_J(uint8_t, uint8_t, uint8_t);

static uint8_t
MAIN__u8JKA_K(uint8_t, uint8_t, uint8_t);

static uint8_t
MAIN__u8JKB_J(uint8_t, uint8_t, uint8_t);

static uint8_t
MAIN__u8JKB_K(uint8_t, uint8_t, uint8_t);

static void
MAIN__vFillJK(MAIN__tstJK [][4], JK_Func, JK_Func);

static void
MAIN__vFillF(MAIN__tstJK [][4], uint8_t [][4], uint8_t);

static void
MAIN__vFillStates(void);

static void
MAIN__vFillOutput(void);

/****************************************************************************/
/****************************************************************************/

int main(void) {
    
    uint8_t u8I;
    
    printf("%s", "======================= Ex. 2 =============================\n\n");
    
    printf("%s", "\tA\tB\n");
    for(u8I = 0; u8I < 4; ++u8I)
        printf("s%d:\t%d\t%d\n", u8I, MAIN__nStateA(u8I), MAIN__nStateB(u8I));
    
    printf("%s", "\n\n======================= Ex. 3 =============================\n\n");
    printf("%s", "\t\tJa - Ka\n");
    MAIN__vFillJK(MAIN__ast32JK_A, MAIN__u8JKA_J, MAIN__u8JKA_K);
    printf("%s", "\n\n\t\tJb - Kb\n");
    MAIN__vFillJK(MAIN__ast32JK_B, MAIN__u8JKB_J, MAIN__u8JKB_K);
    
    
    printf("%s", "\n======================= Ex. 4 =============================\n\n");
    
    MAIN__vFillF(MAIN__ast32JK_A, MAIN__au8AF, MAIN__nA);
    MAIN__vFillF(MAIN__ast32JK_B, MAIN__au8BF, 0);
    
    printf("%s", "\n======================= Ex. 5 =============================\n\n");
    MAIN__vFillStates();
    
    printf("%s", "\n======================= Ex. 6 =============================\n\n");
    
    MAIN__vFillOutput();
    
    return 0;
}



static void
MAIN__vFillOutput(void) {
    
    
    uint8_t u8CurrentState;
    uint32_t u8I;
    
    u8CurrentState = StareInitiala;
    printf("Stare initiala: s%d\n\n", u8CurrentState);
    for(u8I = 0; u8I < MAIN__u32InLen; ++u8I) {
        MAIN__au8Output[u8I] = ( (MAIN__au8JKStates[u8CurrentState][Intrare[u8I]]) & 0x0Fu );
        printf("Intrare: %d\n", Intrare[u8I]);
        printf("Iesire: %d\n", MAIN__au8Output[u8I]);
        u8CurrentState = ( (MAIN__au8JKStates[u8CurrentState][Intrare[u8I]]) >> 4u);
        printf("Urmatoarea stare: s%d\n============\n", u8CurrentState);
    }
    
    printf("%s", "Z: [ ");
    for(int i = 0; i < MAIN__u32InLen; ++i)
        printf("%d ", MAIN__au8Output[i]);
    printf("%s", "]\n");
}

static void
MAIN__vFillStates(void) {
    
    uint8_t u8I;
    uint8_t u8J;
    uint8_t u8Bin;
    
    
    for(u8I = 0; u8I < 4; ++u8I) {
        
        u8Bin = MAIN__nGTB(u8I);
        
        for(u8J = 0; u8J < 2; ++u8J) {
            
            /* [Next state][Value] */
            MAIN__au8JKStates[u8I][u8J]= 
                ( ( ( ( (MAIN__au8AF[u8J][u8Bin]) << 1u ) | (MAIN__au8BF[u8J][u8Bin]) ) << 4u )
                | (Z(u8J, MAIN__nStateA(u8I), MAIN__nStateB(u8I))) );
            
        }
    }
    
    printf("%s","\t  0\t  1\n");
    for(u8I = 0; u8I < 4u; ++u8I) {
        printf("s%d: ", u8I);
        for(u8J = 0; u8J < 2u; ++u8J) {
            printf("\ts%d/%d", MAIN__au8JKStates[u8I][u8J] >> 4u, MAIN__au8JKStates[u8I][u8J] & 0x0Fu);
        }
        printf("%s", "\n");
    }
    
}



static void
MAIN__vFillJK(MAIN__tstJK ast32JK[][4], JK_Func JKFunc1, JK_Func JKFunc2) {
    
    uint8_t u8I;
    uint8_t u8J;
    
    for(u8I = 0; u8I < 2u; ++u8I) {
        for(u8J = 0; u8J < 4u; ++u8J) {
            ast32JK[u8I][u8J].J = JKFunc1(u8I, MAIN__astGray[u8J].A, MAIN__astGray[u8J].B);
            ast32JK[u8I][u8J].K = JKFunc2(u8I, MAIN__astGray[u8J].A, MAIN__astGray[u8J].B);
            printf("{%c %c} ", (ast32JK[u8I][u8J].J ? 'J' : '0'), (ast32JK[u8I][u8J].K ? 'K': '0'));
        }
        printf("%s", "\n");
    }
    
}


static void
MAIN__vFillF(MAIN__tstJK ast32JK[][4], uint8_t au8F[][4], uint8_t u8Variable) {
    
    uint8_t u8I;
    uint8_t u8J;
    uint8_t u8K;
    
    for(u8I = 0; u8I < 2u; ++u8I) {

        for(u8J = 0; u8J < 4u; ++u8J) {
            
            /* Go through the JK table */
            for(u8K = 0; u8K < 4u; ++u8K) {
            
                /* Find the state in the table of the JK */
                if( (MAIN__astJKTable[u8K].JK.J == ast32JK[u8I][u8J].J) 
                    && (MAIN__astJKTable[u8K].JK.K == ast32JK[u8I][u8J].K)) {
                        
                        /* {0, *}, {0, *}, {1, *}, {1, *}  */
                        switch(MAIN__astJKTable[u8K].enState) {
                            case MAIN__nSET:
                                au8F[u8I][u8J] = 1u;
                                break;
                                
                            case MAIN__nUNSET:
                            
                                
                                au8F[u8I][u8J] = 0; 
                                break;
                                
                            case MAIN__nCURRENT:
                            
                                if(u8Variable == 1u)
                                    au8F[u8I][u8J] = MAIN__astGray[u8J].A;
                                else
                                    au8F[u8I][u8J] = MAIN__astGray[u8J].B;
                                    
                                break;
                                
                            case MAIN__nRESET:
                                /* ~ */
                                
                                if(u8Variable == 1u)
                                    au8F[u8I][u8J] = NEG(MAIN__astGray[u8J].A);
                                else
                                    au8F[u8I][u8J] = NEG(MAIN__astGray[u8J].B);
                                    
                                break;
                        }
                    }
                    
            }
        }
        
    }
    
    if(u8Variable == 1u)
        printf("%s", "\t\t\n A+ =\n");
    else
        printf("%s", "\t\t\n B+ =\n");
    for(u8I = 0; u8I < 2; ++u8I) {
        for(u8J = 0; u8J < 4; ++u8J) {
            printf("\t%d ", au8F[u8I][u8J]);
        }
        printf("%s", "\n");
    }
    
    
}

static uint8_t
MAIN__u8JKA_J(uint8_t u8X, uint8_t u8A, uint8_t u8B) {
    return(JKA_J(u8X, u8A, u8B));
}

static uint8_t
MAIN__u8JKA_K(uint8_t u8X, uint8_t u8A, uint8_t u8B) {
    return(JKA_K(u8X, u8A, u8B));
}

static uint8_t
MAIN__u8JKB_J(uint8_t u8X, uint8_t u8A, uint8_t u8B) {
    return(JKB_J(u8X, u8A, u8B));
}

static uint8_t
MAIN__u8JKB_K(uint8_t u8X, uint8_t u8A, uint8_t u8B) {
    return(JKB_K(u8X, u8A, u8B));
}
