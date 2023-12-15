#include <stdio.h>
#include <assert.h>

#include "gmp.h"

#include "xgcd.h"
#include "CRT.h"
#include "rsa.h"

void generate_probable_prime(mpz_t p, size_t nbits, gmp_randstate_t state){
/* to be filled in */
    // Generate a random n-bit number
    mpz_urandomb(p, state, nbits);

    // Find the next prime >= p
    mpz_nextprime(p, p);

    // Check to ensure the prime is still n-bits. If not, repeat the process.
    while (mpz_sizeinbase(p, 2) > nbits) {
        mpz_urandomb(p, state, nbits);
        mpz_nextprime(p, p);
    }
}

/* Generate p and q with nbits. */
void RSA_generate_key(mpz_t N, mpz_t p, mpz_t q, mpz_t e, mpz_t d, size_t nbits, gmp_randstate_t state){
/* to be filled in */
    mpz_t phi; // phi(N) value
    mpz_t gcd_value; // temporary variable to hold gcd result
    
    mpz_inits(phi, gcd_value, NULL);

    // 1. Generate two random prime numbers p and q, each of size nbits
    generate_probable_prime(p, nbits, state);
    do {
        generate_probable_prime(q, nbits, state);
    } while (mpz_cmp(p, q) == 0);  // Ensure p != q

    // 2. Compute the modulus N = p * q
    mpz_mul(N, p, q);

    // 3. Calculate the totient phi(N) = (p-1)(q-1)
    mpz_sub_ui(p, p, 1);
    mpz_sub_ui(q, q, 1);
    mpz_mul(phi, p, q);

    // 4. Choose an encryption key e
    mpz_set_ui(e, 3);  // Commonly chosen starting value for e
    while (1) {
        mpz_gcd(gcd_value, e, phi);
        if (mpz_cmp_ui(gcd_value, 1) == 0) break;
        mpz_add_ui(e, e, 2);
    }

    // 5. Compute the decryption key d
    if (!mpz_invert(d, e, phi)) {
        // This should not happen if p and q are primes and e is chosen correctly.
        mpz_set_ui(d, 0);  // Indicate error by setting d=0
    }

    // Clean up temporary variables
    mpz_clears(phi, gcd_value, NULL);
}

void RSA_encrypt(mpz_t cipher, mpz_t msg, mpz_t N, mpz_t e){
/* to be filled in */
    mpz_powm(cipher, msg, e, N);
}

void RSA_decrypt(mpz_t msg, mpz_t cipher, mpz_t N, mpz_t d){
/* to be filled in */
    mpz_powm(msg, cipher, d, N);
}

/* Use CRT. */
void RSA_decrypt_with_p_q(mpz_t msg, mpz_t cipher, mpz_t N, mpz_t d, mpz_t p, mpz_t q){
/* to be filled in */
    mpz_t dp, dq, qinv, mp, mq, h;

    // Initialize temporary variables
    mpz_inits(dp, dq, qinv, mp, mq, h, NULL);

    // Calculate dp = d mod (p-1) and dq = d mod (q-1)
    mpz_sub_ui(dp, p, 1);
    mpz_mod(dp, d, dp);

    mpz_sub_ui(dq, q, 1);
    mpz_mod(dq, d, dq);

    // Compute mp = cipher^dp mod p and mq = cipher^dq mod q
    mpz_powm(mp, cipher, dp, p);
    mpz_powm(mq, cipher, dq, q);

    // Compute qinv = q^(-1) mod p
    mpz_invert(qinv, q, p);

    // Compute h = qinv * (mp - mq) mod p
    mpz_sub(h, mp, mq);
    mpz_mul(h, h, qinv);
    mpz_mod(h, h, p);

    // Compute msg = mq + hq
    mpz_mul(msg, h, q);
    mpz_add(msg, msg, mq);

    // Clear temporary variables
    mpz_clears(dp, dq, qinv, mp, mq, h, NULL);
}
