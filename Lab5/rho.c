/****************************************************************/
/* rho.c                                                        */
/* Authors: Alain Couvreur, Maxime Bombar                       */
/* alain.couvreur@lix.polytechnique.fr                          */
/* maxime.bombar@inria.fr                                       */
/* Last modification October 24, 2022                           */
/****************************************************************/

#include <stdio.h>
#include <assert.h>

#include "gmp.h"
#include "utils.h"

#include "rho.h"


/* to be filled in */
long random_function(long var, const long N){
    return (var*var + 7) % N;
}

int PollardRho_with_long(long *factor, const long N,
                         long nbOfIterations) {
    int status = FACTOR_ERROR;
/* to be filled in */
    long x = 1;
    long y = 1;
    mpz_t g, tmp, tmp2;
    mpz_init_set_ui(g, 1);
    mpz_inits(tmp, tmp2, NULL);


    for(long i=0; i<nbOfIterations; i++){
        if(mpz_cmp_ui(g, 1) != 0){
            if(i != nbOfIterations){
                status = FACTOR_FOUND;
                *factor = mpz_get_ui(g); 
                break;
            }
        }
        x = random_function(x, N);     
        y = random_function(random_function(y, N), N); //two iteration
        mpz_set_ui(tmp, x);
        mpz_set_ui(tmp2, y);
        mpz_sub(tmp, tmp, tmp2);// y-x
        mpz_abs(tmp, tmp);// |y-x|
        mpz_gcd_ui(g, tmp, N);
    }
    
    mpz_clears(tmp, tmp2, g, NULL);
    return status;
}

int PollardRhoSteps(mpz_t factor, const mpz_t N,
                    void (*f)(mpz_t, mpz_t, const mpz_t),
                    long nbOfIterations) {
    int status = FACTOR_ERROR;
/* to be filled in */

    return status;
}

// int PollardRho(factor_t *result, int *nf, const mpz_t N,
//                void (*f)(mpz_t, mpz_t, const mpz_t), long nbOfIterations) {
//     int status = FACTOR_ERROR;
// /* to be filled in */

//     return status;
// }
int PollardRho(factor_t *result, int *nf, const mpz_t N,
               void (*f)(mpz_t, mpz_t, const mpz_t), long nbOfIterations) {
    int status = FACTOR_ERROR;
/* to be filled in */
    long i;
    mpz_t x, y, g, tmp;
    mpz_init_set_ui(x, 1);
    mpz_init_set_ui(y, 1);
    mpz_inits(g, tmp, NULL);

    for(i=0; i < nbOfIterations+1; i++){

        // normal steps
        f(x, x, N);
        f(y, y, N);
        f(y, y, N);
        mpz_sub(tmp, x, y);
        mpz_abs(tmp, tmp);
        mpz_gcd(g, tmp, N);

        if(i == nbOfIterations)    // loop exit with the max value
            status = FACTOR_NOT_FOUND;
        
        if(mpz_cmp_ui(g, 1) != 0 && mpz_cmp(g, N) != 0){
            status = FACTOR_FOUND;
            result->status = FACTOR_FOUND;

            AddFactor(result, g, 1, status);
            
            (*nf)++;
            break;
        }
    }

    mpz_clears(x, y, g, tmp, NULL);
    return status;
}
