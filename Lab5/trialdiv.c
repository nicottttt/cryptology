#include <stdlib.h>
#include <stdio.h>

#include "gmp.h"
#include "utils.h"
#include "trialdiv.h"

/* OUTPUT: 1 if factorization finished. */
int trialDivision(factor_t* factors, int *nf, mpz_t cof, const mpz_t N,
		  const long bound, uint length, FILE* ficdp){
    int status = FACTOR_NOT_FOUND;
/* to be filled in */

    int prime = 2; // The smallest prime
    int pos = 0;// position
    int exponent = 0;
    int half_diff;
    char half_diff_str[3]; // The number in file shorter than 3

    mpz_t mpz_prime, rest, tmp;
    mpz_inits(mpz_prime, tmp, rest, NULL);
    mpz_set(rest, N);
    mpz_set_ui(mpz_prime, prime);

    while(prime <= bound){
        mpz_mod(tmp, N, mpz_prime); //tmp = N mod mpz_prime
        if(mpz_get_ui(tmp) == 0){// if find the prime
            exponent = 1;
            mpz_set(tmp, mpz_prime);
            status = FACTOR_FOUND;
            while(mpz_cmp_ui(tmp, bound)<0){ // to find the exponent
                exponent ++;
                mpz_pow_ui(tmp, mpz_prime, exponent);//tmp = mpz_prime^exponent
                mpz_mod(tmp, N, tmp);
                if(mpz_get_ui(tmp)!=0){ //if not, --
                    exponent --;
                    break;
                }
            }

            mpz_pow_ui(tmp, mpz_prime, exponent );//tmp = mpz_prime^exponent
            mpz_cdiv_q(rest, rest, tmp);// when it divide the tmp, the rest of the N
            AddFactor(factors + pos, mpz_prime, exponent, status);
            pos++;// next position
        }

        // get the next prime
        fgets(half_diff_str, 3, ficdp);// read the half difference from the file     
        half_diff = atoi(half_diff_str);
        prime += half_diff*2;
        if(prime == 4) // Because the first line represent the difference between 2 and 3 but not the half difference
            prime = 3;
        mpz_set_ui(mpz_prime, prime);
    }

    // Finished
    status = FACTOR_FINISHED;
    mpz_set(cof, rest);
    *nf = pos;
    
    
    mpz_clears(mpz_prime, rest, tmp,  NULL);
    return status;
}
