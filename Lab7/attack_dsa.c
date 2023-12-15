#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "utilities.h"

#include "gmp.h"
#include "buffer.h"
#include "sha3.h"

#include "rsa.h"
#include "sign.h"
#include "dsa.h"
#include "attack_dsa.h"

#define DEBUG 0

int dsa_sign_dummy(buffer_t *msg, mpz_t p,
		   mpz_t q, mpz_t a, mpz_t x, mpz_t r, mpz_t s,
		   mpz_t k){
    int status = 1;
    
/* to be filled in */
    mpz_t hash_msg, tmp;
    mpz_inits(hash_msg, tmp, NULL);
    buffer_t hash;

    // Generate hash of msg
    buffer_init(&hash, hash_length(q));
    buffer_hash(&hash, hash_length(q), msg);
    mpz_import(hash_msg, hash_length(q), 1, 1, 0, 0, hash.tab);

    // no need to ramdom, use precise k
    mpz_powm(r, a, k, p);
    mpz_mod(r, r, q);

    mpz_mul(s, x, r);
    mpz_add(s, s, hash_msg);
    mpz_invert(tmp, k, q);
    mpz_mul(s, s, tmp);
    mpz_mod(s, s, q);

    mpz_clears(hash_msg, tmp, NULL);

    return status;
}

/* Solves the system with unkowns k, x:
   s1.k - r1.x = h1
   s2.k - r1.x = h2
   and fills in x
*/
int solve_system_modq(mpz_t x, mpz_t r1, mpz_t s1,
		      mpz_t r2, mpz_t s2, mpz_t h1, mpz_t h2,
		      mpz_t q){
    int status = 1;

    /* to be filled in */

    gmp_printf("Linear system :\n%Zd k  - %Zd x = %Zd\n%Zd k  - %Zd x = %Zd\n\n", s1, r1, h1, s2, r2, h2);

    /*
        k*s1 = r1*x + h1
        k*s2 = r2*x + h2
        .....
        result:
        x = (s2h1 - s1h2)/(s1r2 - s2r1)
    */

    mpz_t s2h1, s1h2, s1r2, s2r1, dividend, divisor, inv;
    mpz_inits(s2h1, s1h2, s1r2, s2r1, dividend, divisor, inv, NULL);

    mpz_mul(s2h1, s2, h1);
    mpz_mul(s1h2, s1, h2);
    mpz_mul(s1r2, s1, r2);
    mpz_mul(s2r1, s2, r1);

    mpz_sub(dividend, s2h1, s1h2);
    mpz_sub(divisor, s1r2, s2r1);

    // mpz_cdiv_q(x, dividend, divisor);
    // mpz_mod(x, x, q);

    // Use multiply to replace divide by
    mpz_invert(inv, divisor, q);
    mpz_mul(x, dividend, inv);
    mpz_mod(x, x, q);


    gmp_printf("Candidate for secret key obtained from the attack:\nx = %Zd\n\n", x);

    mpz_clears(s2h1, s1h2, s1r2, s2r1, dividend, divisor, inv, NULL);


    return status;
}


int dsa_attack(mpz_t x, buffer_t *msg1, buffer_t *msg2,
	       mpz_t p, mpz_t q, mpz_t a, mpz_t r1,
	       mpz_t s1, mpz_t r2, mpz_t s2){
    int status = 1;
    
/* to be filled in */
    mpz_t hash_msg1, hash_msg2;
	buffer_t hash;

	mpz_inits(hash_msg1, hash_msg2, NULL);

    buffer_init(&hash, hash_length(q));
    buffer_hash(&hash, hash_length(q), msg1);
    mpz_import(hash_msg1, hash_length(q), 1, 1, 0, 0, hash.tab);

    buffer_init(&hash, hash_length(q));
    buffer_hash(&hash, hash_length(q), msg2);
    mpz_import(hash_msg2, hash_length(q), 1, 1, 0, 0, hash.tab);

    // Return x
	solve_system_modq(x, r1, s1, r2, s2, hash_msg1, hash_msg2, q);

	mpz_clears(hash_msg1, hash_msg2, NULL);

    return status;
}
