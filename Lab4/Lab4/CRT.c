#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "gmp.h"

#include "xgcd.h"
#include "CRT.h"

/* Given (r0, m0) and (r1, m1), compute n such that
   n mod m0 = r0; n mod m1 = r1.  If no such n exists, then this
   function returns 0. Else returns 1.  The moduli m must all be positive.
*/

// Just to judge if n is the on I need or not
int CRT2_n(mpz_t n, mpz_t r0, mpz_t m0, mpz_t r1, mpz_t m1){
    int status = 0;

    mpz_t tmp, tmp2;
    mpz_init_set_ui(tmp, 0);
    mpz_init(tmp2);

    // Just to make sure one n is ok
    mpz_mod(tmp, n, m0);
    mpz_mod(tmp2, n, m1);
    if((mpz_cmp(tmp, r0) == 0) && (mpz_cmp(tmp2, r1) == 0)){
        status = 1;
    }
    
    mpz_clears(tmp, tmp2, NULL);


/* to be filled in */
    return status;
}

int CRT2(mpz_t n, mpz_t r0, mpz_t m0, mpz_t r1, mpz_t m1){
    int status = 0;

    for(int i=0; i<256; i++){
        mpz_set_ui(n, i);// n += 1
        status = CRT2_n(n, r0, m0, r1, m1);
        if(status == 1){
            break;
        }
    }


/* to be filled in */
    return status;
}

/* to be filled in */

/* Given a list S of pairs (r,m), returns an integer n such that n mod
   m = r for each (r,m) in S.  If no such n exists, then this function
   returns 0. Else returns 1.  The moduli m must all be positive.
*/
int CRT(mpz_t n, mpz_t *r, mpz_t *m, int nb_pairs){
    int status = 0;

    for(int i=0; i<100000; i++){
        mpz_set_ui(n, i);
        int cnt = 0;
        // Put n to all the CRT_n function to compare all the pair
        for(int j=0; j<nb_pairs-1; j++){
            cnt += CRT2_n(n, r[j], m[j], r[j+1], m[j+1]); 
        }

        if((cnt == nb_pairs-1)){// Need all the return status to be 1
            status = 1;
            break;
        }
    }

/* to be filled in */
    return status;
}
