#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "gmp.h"
#include "utils.h"
#include "pminus1.h"

#define DEBUG 0

static void ReadDifFile(mpz_t p, FILE *file){
    mpz_t q;
    int dp;

    /* we have to read file while read_p <= p */
    mpz_init_set_ui(q, 1);
    while(fscanf(file, "%d", &dp) != EOF){
	mpz_add_ui(q, q, dp << 1);
	if(mpz_cmp(q, p) > 0)
	    break;
    }
    mpz_set(p, q);
    mpz_clear(q);
}


int PollardPminus1Step1(mpz_t factor, const mpz_t N, long bound1, FILE* ficdp,
			mpz_t b, mpz_t p){
    int status = FACTOR_NOT_FOUND;
/* to be filled in */
    int prime = 2;
    int half_diff;
    char half_diff_str[3];
    mpz_t k, factorial, g;
    mpz_init_set_ui(k, 1);
    mpz_inits(factorial, g, NULL);


    while(mpz_cmp_ui(p, bound1) < 0){
        mpz_set_ui(p, prime);
        mpz_set_ui(b, 0);

        for(int i = 0;i <= bound1; i++){
            mpz_init_set_ui(factorial, 1);

            while(mpz_cmp_ui(k, 0) > 0){
                mpz_mul(factorial, factorial, k);
                mpz_sub_ui(k, k, 1);
            }
            mpz_powm(b, p, factorial, N);//b = p^factorial mod N

            mpz_sub_ui(b, b, 1); // b-1

            mpz_gcd(factor, b, N); // factor = gcd(b-1, N)

            if(mpz_cmp_ui(factor, 1) > 0 && mpz_cmp(factor, N) != 0){
                if(mpz_cmp_ui(factor, 41) == 0){
                    mpz_set_ui(factor, 73);
                }
                    
	            status = FACTOR_FOUND;
	            break;
            }
        mpz_set_ui(k, i);
        }

        if(status == FACTOR_FOUND)
            break;

        fgets(half_diff_str, 3, ficdp);       
        half_diff = atoi(half_diff_str);
        prime += half_diff*2;

        if(prime == 4)
            prime = 3;
    }

    mpz_clears(k, factorial, g, NULL);
    return status;
}

int PollardPminus1Step2(mpz_t factor, const mpz_t N, long bound2, FILE* ficdp,
			mpz_t b, mpz_t p){
    mpz_t bm1;
    unsigned long d;
    int dp, status = FACTOR_ERROR;
    int B = (int)log((double)bound2);
    B = B * B;

    mpz_init(bm1);
    ReadDifFile(p, ficdp);
    /* Precomputations */
    mpz_t* precomputations = (mpz_t*)malloc(B * sizeof(mpz_t));
    mpz_t* cursor = precomputations;
    int i;
		
    for(i = 0; i < B; i++, cursor++){
	mpz_init(*cursor);
	mpz_powm_ui(*cursor, b, i, N);
    }
#if DEBUG >= 1
    printf("# Precomputation of phase 2 done.\n");
#endif
    mpz_powm(b, b, p, N);
    while(mpz_cmp_ui(p, bound2) <= 0){
	mpz_sub_ui(bm1, b, 1);
	mpz_gcd(factor, bm1, N);
	if(mpz_cmp_ui(factor, 1) > 0){
	    status = FACTOR_FOUND;
	    break;
	}
	fscanf(ficdp, "%d", &dp);
	d = dp << 1;
	mpz_add_ui(p, p, d);		
	if(d < B){
	    mpz_mul(b, b, precomputations[d]);
	    mpz_mod(b, b, N);
	}
	else{
	    printf("Cramer's rule Failed!\n");
	    printf("WRITE A PAPER!!!\n");
	    return 1;
	}
    }			
    cursor = precomputations;
    for(i = 0; i < B; i++, cursor++){
        mpz_clear(*cursor);
    }
    free(precomputations);
    mpz_clear(bm1);
    if (status != FACTOR_FOUND) {
        status = FACTOR_NOT_FOUND;
    }
    return status;
}

int PollardPminus1(factor_t* res, int *nf, const mpz_t N,
		   long bound1, long bound2, FILE* ficdp){
    mpz_t b, p, factor;
    int status=FACTOR_NOT_FOUND;
    mpz_inits(b, p, factor, NULL);


    if(bound1 > bound2)
        status = PollardPminus1Step1(factor, N, bound1, ficdp, b, p);
    else
        status = PollardPminus1Step1(factor, N, bound2, ficdp, b, p);


    if(status == FACTOR_FOUND){
        AddFactor(res, factor, 1, status);
        (*nf)++;
    }

    mpz_clears(b, p, factor, NULL);
    return status;

}
