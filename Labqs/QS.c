/****************************************************************/
/* QS.c                                                         */
/* Author : F. Morain                                           */
/* morainr@lix.polytechnique.fr                                 */
/* Last modification October 24, 2017                           */
/****************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>

#include "utilities.h"

#include "gmp.h"
#include "hash.h"
#include "utils.h"
#include "QS.h"

#define DEBUG 0
#define FIND_RELATIONS_USING_TD 1 /* 1 for TD; 0 for sieve */

int trial_div(char *tabex, mpz_t cof, const mpz_t Px, int* B, int cardB){
    int status = FACTOR_NOT_FOUND;
/* to be filled in */
    int i = 1;
    int exp = 0;
    mpz_t next, tmp, rest, abs_Px;
    mpz_inits(next, tmp, abs_Px, NULL);

    if(mpz_cmp_si(Px, 0) < 0){//if it's negetive
        tabex[0] = 1;
        mpz_abs(abs_Px, Px);
    }else{
        tabex[0] = 0;
        mpz_abs(abs_Px, Px);
    }

    mpz_init_set(rest, abs_Px);
    while(i < cardB){
        exp = 0;
        mpz_set_si(next, B[i]); // next = B[i]
        mpz_mod(tmp, abs_Px, next); //Px mod next

        if(mpz_get_ui(tmp) == 0){
            exp = 1;
            mpz_set(tmp, next);
            status = FACTOR_FOUND;
            while(mpz_cmp(tmp, abs_Px)<0){
                exp++; // add first
                mpz_pow_ui(tmp, next, exp);//next^exponent
                mpz_mod(tmp, abs_Px, tmp); //Px mod next^cnt 
                if(mpz_get_ui(tmp)!=0){
                    exp--;
                    break;
                }
            }

            mpz_pow_ui(tmp, next, exp);//next^exponent
            mpz_cdiv_q(rest, rest, tmp);
            tabex[i] = exp;
        }else{
            tabex[i] = 0;
        }
        i++;
    }
    
    if(mpz_cmp_ui(rest, 1) != 0)
        status = 0;
    else{
        mpz_set(cof, rest);
        status = 1;
    }
    return status;
}

void StoreRelation(relation_t *rel, mpz_t kN, mpz_t g, 
		   int cardB, int x, char *tabex){
    char *tmp = (char *)malloc(cardB * sizeof(char));

    memcpy(tmp, tabex, cardB);
    mpz_init_set_si(rel->y, x);
    mpz_add(rel->y, rel->y, g);
    mpz_mod(rel->y, rel->y, kN);
    rel->tabex = tmp;
}

void AddRelation(relation_t *tabrels, mpz_t kN, mpz_t g, 
		 int cardB, int i, int x, char *tabex){
    StoreRelation(tabrels+i, kN, g, cardB, x, tabex);
#if DEBUG >= 0
    printf("%d:", i);
    for(i = 0; i < cardB; i++)
	printf(" %d", tabex[i]);
    printf("\n");
#endif
}

/* OUTPUT: the actual number of relations found <= nrelsmax */
int FindRelationsUsingTrialDivision(relation_t *tabrels, mpz_t kN, mpz_t g,
				    int *B, int cardB, int M, int nrelsmax){
    char *tabex = (char *)malloc(cardB * sizeof(char));
    int x, nrels = 0;
    mpz_t Px, cof;

    mpz_inits(Px, cof, NULL);
    for(x = -M; x <= M; x++){
	mpz_set_si(Px, x);
	mpz_add(Px, Px, g);
	mpz_mul(Px, Px, Px);
	mpz_sub(Px, Px, kN);
	if(trial_div(tabex, cof, Px, B, cardB) != 0){
	    gmp_printf("x=%d Px=%Zd\n", x, Px);
	    AddRelation(tabrels, kN, g, cardB, nrels, x, tabex);
	    nrels++;
	    if(nrels == nrelsmax)
		break;
	}
    }
    free(tabex);
    mpz_clears(Px, cof, NULL);
    return nrels;
}

/* to be filled in */

/* Sieving over [-M, M].
   OUTPUT: the actual number of relations found <= nrelsmax */
int FindRelationsUsingSieving(relation_t *tabrels, mpz_t kN, mpz_t g,
			      int *B, int cardB, int lpB, int M, int nrelsmax){
    int nrels = 1;
/* to be filled in */
    return nrels;
}

/* OUTPUT: the actual number of relations found <= nrelsmax */
int FindRelations(relation_t *tabrels, mpz_t kN, mpz_t g, int *B, 
		  int cardB, int M, int nrelsmax){
#if FIND_RELATIONS_USING_TD == 1
    return FindRelationsUsingTrialDivision(tabrels, kN, g, B, cardB, M, nrelsmax);
#else
    int lpB = (mpz_cmp_ui(kN, 2000) <= 0 ? 0 : 100 * B[cardB-1]);
    return FindRelationsUsingSieving(tabrels,kN,g,B,cardB,lpB,M,nrelsmax);
#endif
}

void PrintMatrix(char **mat, int nrows, int ncols){
    int i, j;

    for(i = 0; i < nrows; i++){
	for(j = 0; j < ncols; j++)
	    printf(" %d", mat[i][j]);
	printf("\n");
    }
}

void PrintMatrices(char **mat, char **C, int nrows, int ncols){
/* to be filled in */
    for(int i = 0; i < nrows; i++){
        for(int j = 0; j < ncols; j++)
            printf("%d ", mat[i][j]&1);
        printf("\t");
        for(int k = 0; k < (i+1); k++)
            printf("%d ", C[i][k]);
        printf("\n");
    }
}

/* mat[i1] += mat[i2]; C[i1] += C[i2]. */
void AddRows(char **mat, char **C, int ncols, int i1, int i2, int j)
{
    int i;
    for (i = 0; i < ncols; i++)
    {
        mat[i1][i] ^= mat[i2][i];
    }
    for (i = 0; i < j; i++)
    {
        C[i1][i] ^= C[i2][i];
    }
}

void Gauss(char **mat, char **C, int nrows, int ncols)
{
    char *usedPivot = malloc((nrows) * sizeof(char));
    int piv_counter = 0;
    int i, j, k, l;
    int flag;

    for (j = 0; j < ncols; j++){ // col
        for (i = 0; i < nrows; i++){ // row
            if (mat[i][j] == 0) // jump the process
                continue;

            flag = 1;
            for (k = 0; k < piv_counter; k++)
                if (i == usedPivot[k])
                    flag = 0;
            if (!flag)
                continue;

            printf("pivot[%d]=%d\n", j, i);
            usedPivot[piv_counter] = i;
            piv_counter += 1;
            break;
        }

        if (i == nrows){
            printf("pivot[%d]=%d\n", j, i);
            PrintMatrices(mat, C, nrows, ncols);// ouput the info
            continue;
        }

        for (l = 0; l < nrows; l++){
            if (mat[l][j] == 0)
                continue;

            flag = 1;
            for (k = 0; k < piv_counter; k++)
                if (l == usedPivot[k])
                    flag = 0;
            if (!flag)
                continue;

            AddRows(mat, C, ncols, l, i, i + 1);// put them into the mat
        }
        PrintMatrices(mat, C, nrows, ncols);
    }
}

char **MatrixFromRelations(relation_t *tabrels, int nrows, int ncols){
    char **mat = (char **)malloc(nrows * sizeof(char *));
    for(int i = 0; i< nrows; i++){
        mat[i] = (char *)malloc((i+1) * sizeof(char));
    }
/* to be filled in */
    int i, j;
    for(i = 0; i < nrows; i++){
        for(j = 0; j < ncols; j++)
            mat[i][j] = tabrels[i].tabex[j] % 2;// even number = 0, odd number = 1;
    }
    return mat;
}


char **BuildCompanionMatrix(int nrows){
    char **C = (char **)malloc(nrows * sizeof(char *));
    for(int i = 0; i< nrows; i++){
        C[i] = (char *)malloc((i+1) * sizeof(char));
    }
/* to be filled in */
    int j = 0; 
    for(int i = 0; i < nrows; i++){
        C[i][j] = 1;
        j++;
    }
    return C;
}

int FinishFactorization(factor_t *tabf, int *nf, mpz_t N, mpz_t kN, mpz_t g, 
			relation_t *tabrels, char **mat, char **C,
			int nrelsmax, int *B, int cardB){

    mpz_t X, Y; // the factor need to show
    mpz_inits(X, Y, NULL);
    int status = 0;
    int nrows = nrelsmax;
    int ncols = cardB;
    int ZeroFlag, i, j, k;


    for (i = 0; i < nrows; i++)
    {
        ZeroFlag = 1;
        for (j = 0; j < ncols; j++)
        {
            if (mat[i][j] != 0)
            {
                ZeroFlag = 0;
            }
        }
        if (!ZeroFlag)
            continue;

        //init
        char *exp = malloc((ncols) * sizeof(char));
        char *coef = malloc((ncols) * sizeof(char));
        for (k = 0; k < i; k++)
        {
            exp[k] = 0;
            coef[k] = 0;
        }


        // Standardize the ouput
        printf("dep: ");
        mpz_set_si(X, 1);
        mpz_set_si(Y, 1);
        for (j = 0; j < i + 1; j++)
        {
            if (C[i][j] == 1)
            {
                mpz_mul(X, X, tabrels[j].y); // Set as mult of primes
                if (i != j)
                    printf("%d ", j);

                for (k = 0; k < cardB; k++)
                {
                    exp[k] += tabrels[j].tabex[k];
                }
            }
        }

        printf("%d -> ", i);

        for (k = 0; k < cardB; k++)
            printf("%x ", exp[k]);
        printf("-> ");
        

        mpz_mod(X, X, kN);

        // Calculate the values of X and Y and find solution
        long tmp = 1;
        for (j = 0; j < ncols; j++){
            tmp = tmp * pow(B[j], exp[j]);
        }
        mpz_set_ui(Y, tmp);
        mpz_sqrt(Y, Y);
        mpz_mod(Y, Y, kN);
        gmp_printf("X=%Zd, Y=%Zd -> ", X, Y);
        mpz_mod(Y, Y, N);
        mpz_sub(X, X, Y);
        mpz_gcd(X, X, N);
        gmp_printf("%Zd\n", X);
        status = 1;

        int FoundFlag = 0;
        for (k = 0; k < ncols; k++)
        {
            if (mpz_cmp(X, tabf[k].f) == 0)
            {
                FoundFlag++;
                tabf[k].e += 1;
                break;
            }
        }

        if (FoundFlag == 0)
        {
            AddFactor((tabf + *nf), X, 1, status);
            *nf = *nf + 1;
        }

    }
    mpz_clears(X, Y, NULL);
    return status;
}

int QS_aux(factor_t *tabf, int *nf, mpz_t N, mpz_t kN, mpz_t g, int *B,
	   int cardB, int M, int phase){
    int nrelsmax = cardB+2, nrels, i, status = FACTOR_NOT_FOUND;
    relation_t *tabrels = (relation_t *)malloc(nrelsmax * sizeof(relation_t));
    char **mat, **C;

    nrels = FindRelations(tabrels, kN, g, B, cardB, M, nrelsmax);
    implementation_check("FindRelations", nrels);
    if(nrels < cardB){
	printf("Not enough relations: %d // %d\n", nrels, nrelsmax);
	return -1;
    }
    if(phase == 1)
	return 0;
    mat = MatrixFromRelations(tabrels, nrels, cardB);
#if DEBUG >= 0
    PrintMatrix(mat, nrels, cardB);
#endif
    if(phase == 2)
	return 0;
    C = BuildCompanionMatrix(nrels);
#if DEBUG >= 0
    PrintMatrices(mat, C, nrels, cardB);
#endif
    if(phase == 3)
	return 0;
    Gauss(mat, C, nrels, cardB);
    if(phase == 4)
	return 0;
    status = FinishFactorization(tabf, nf, N, kN, g, tabrels, mat, C, nrels, B, cardB);
    free(tabrels);
    for(i = 0; i < nrels; i++){
	free(mat[i]);
	free(C[i]);
    }
    free(mat);
    free(C);
    return status;
}

/* Source: Silverman87. */
int FindMultiplier(mpz_t N){
    int kopt = 1;
/* to be filled in */
    return kopt;
}

/* OUTPUT: NULL if some problem occurred, a factor base otherwise of size
   cardB, starting {-1, 2, ...}. */
int *BuildFactorBase(mpz_t kN, int k, int cardB, FILE *file){
    int *B = NULL;
/* to be filled in */
    return B;
}

/* This is from Silverman87, but for MPQS. */
void ChooseParameters(int *cardB, int *M, mpz_t N){
    size_t dd = mpz_sizeinbase(N, 10);
    int thresh[] = {10, 24, 30, 36, 42, 48, 54, 60, 66, 0};
    int tcardB[] = {50, 100, 200, 400, 900, 1200, 2000, 3000, 4500, 0};
    int tM[] = {1000, 5000, 25000, 25000, 50000, 100000, 250000, 350000, 500000, 0};
    int i;

    *cardB = -1; *M = -1;
    for(i = 0; thresh[i] != 0; i++){
	if(dd <= thresh[i]){
	    *cardB = tcardB[i];
	    *M = tM[i];
	    break;
	}
    }
}

int QS(factor_t *tabf, int *nf, mpz_t N, int k, int cardB, int M, FILE *file,
       int phase){
    int *B, status = FACTOR_NOT_FOUND;
    mpz_t kN, g;

    mpz_inits(kN, g, NULL);
    if(k == 0){
	k = FindMultiplier(N);
	printf("Best multiplier: %d\n", k);
    }
    mpz_mul_ui(kN, N, k);
    /* g = trunc(sqrt(k*N)) */
    mpz_sqrt(g, kN);
    if(cardB == 0 || M == 0){
	int cardB0 = cardB, M0 = M;
	
	ChooseParameters(&cardB, &M, kN);
	if(cardB0 != 0)
	    cardB = cardB0;
	if(M0 != 0)
	    M = M0;
    }
    B = BuildFactorBase(kN, k, cardB, file);
    if(B == NULL)
	return FACTOR_ERROR;
    printf("cardB=%d, M=%d\n", cardB, M);
    status = QS_aux(tabf, nf, N, kN, g, B, cardB, M, phase);
    
    mpz_clears(kN, g, NULL);
    free(B);
    return status;
}
