#include <stdio.h>
#include <stdlib.h>

#include "utilities.h"

#include "gmp.h"
#include "crt.h"
#include "rsa.h"

#define DEBUG 0


int is_valid_key(mpz_t p, mpz_t q, mpz_t e, mpz_t d, int nlen, int sec){
    mpz_t lambda, pm1, qm1, pmq, ed, g, bound1, bound2;
    mpz_inits(lambda, pm1, qm1, pmq, ed, g, bound1, bound2, NULL);

    mpz_sub_ui(pm1, p, 1);
    mpz_sub_ui(qm1, q, 1);
    mpz_lcm(lambda, pm1, qm1);

    int status = 1;
    // p, q should be prime
    if(!mpz_probab_prime_p(p, 25) || ! mpz_probab_prime_p(q, 25)){
#if DEBUG
      printf("[is_valid_key] : p, q should be prime.  ");
#endif
      status = 0;
      goto end_valid;
    }

    // e should be odd
    if(mpz_divisible_ui_p(e, 2)){
#if DEBUG
	  printf("[is_valid_key] : e should be odd.  ");
#endif
      status = 0;
      goto end_valid;
    }

    // e should be prime to lambda
    mpz_gcd(g, e, lambda);
    if(mpz_cmp_ui(g, 1) != 0){
#if DEBUG
      printf("[is_valid_key] : e should be prime to lambda. ");
#if DEBUG >= 2
	  gmp_printf("gcd(e, lambda) = %Zd.  ", g);
#endif
#endif
	  status = 0;
      goto end_valid;
    }
    // Bounds on e.
    size_t size_e = mpz_sizeinbase(e, 2);
    if(size_e < 16){
#if DEBUG
      printf("[is_valid_key] : e is too small.   ");
#endif
      status = 0;
      goto end_valid;
    }
    if(size_e > 256){
#if DEBUG
      printf("[is_valid_key] : e is too large.   ");
#endif
      status=0;
      goto end_valid;
    }

    // p, q should be large enough
    mpf_t b_f, tmp_f;
    mpf_inits(b_f, tmp_f, NULL);
    mpf_sqrt_ui(b_f, 2);
    mpf_set_ui(tmp_f, 2);
    mpf_pow_ui(tmp_f, tmp_f, nlen/2 - 1);
    mpf_mul(b_f, b_f, tmp_f);
    mpz_set_f(bound1, b_f);	
    if(mpz_cmp(p, bound1) <= 0 || mpz_cmp(q, bound1) <= 0){
#if DEBUG
	printf("[is_valid_key] : p or q is too small.  ");
#endif
	status = 0;
    }
    mpz_clear(bound1);
    mpf_clears(b_f, tmp_f, NULL);
    if(status == 0)
	  goto end_valid;

    // p, q should not be too close to each other
    mpz_inits(bound2, pmq, NULL);
    mpz_ui_pow_ui(bound2, 2, nlen/2 - sec);
    mpz_sub(pmq, p, q);
    if(mpz_cmpabs(pmq, bound2) <= 0){
#if DEBUG
	  printf("[is_valid_key] : p and q are too close.  ");
#endif
      status = 0;
    }
    mpz_clears(bound2, pmq, NULL);
    if(status == 0)
	  goto end_valid;

    // ed = 1 mod lambda
    mpz_mul(ed, e, d);
    mpz_mod(ed, ed, lambda);
    if(mpz_cmp_ui(ed, 1) != 0){
#if DEBUG
	  printf("[is_valid_key] : ed should be");
	  printf(" congruent to 1 modulo lambda.   ");
#endif
	  status = 0;
      goto end_valid;
    }
 end_valid:
    mpz_clears(lambda, pm1, qm1, ed, g, NULL);

    return status;
}


int RSA_weak_generate_key(mpz_t p, mpz_t q, mpz_t e, mpz_t d, int nlen,
			   gmp_randstate_t state){
    int status = 1;
    // complete the function
/* to be filled in */
    int length = nlen/2;
    mpz_t pq_threshold, tmp, tmp2, p_1, q_1;
    mpz_inits(pq_threshold, p_1, q_1, NULL);
    mpz_init_set_ui(tmp, 2);
    mpz_init_set_ui(tmp2, 2);

    mpz_pow_ui(tmp, tmp, length - 1); //2^(nlen/2 - 1)
    mpz_sqrt(tmp2, tmp2); // √2
    mpz_mul(pq_threshold, tmp, tmp2); // √2 * 2^(nlen/2 - 1)


    // Generate p,q
    while(1){
        mpz_urandomb(p, state, length); // mpz_urandomb generate (s^length - 1) random value
        mpz_nextprime(p, p); // p,q must be prime
        //gmp_printf("The number p is: %Zd\n", p);

        if(mpz_cmp(p, pq_threshold) >= 0 && mpz_sizeinbase(p, 2) == length)
            break;
    }

    while(1){
        mpz_urandomb(q, state, length);
        mpz_nextprime(q, q); // p,q must be prime

        if(mpz_cmp(q, pq_threshold) >= 0 && mpz_sizeinbase(p, 2) == length)
            break;
    }


    // Generate λ(pq) = lcm(p-1, q-1)
    mpz_sub_ui(p_1, p, 1);
    mpz_sub_ui(q_1, q, 1);
    mpz_lcm(tmp, p_1, q_1); // tmp = λ(pq)

    // Generate e
    while(1){

        while(1){
            mpz_urandomb(e, state, 256);
            if(mpz_cmp_ui(e, 1 << 16) >= 0)
                break;
        }
        mpz_gcd(tmp2, e, tmp);// e≡1modλ(pq)
        if(mpz_cmp_ui(tmp2, 1) == 0)
            break;
    }

    mpz_invert(d, e, tmp);// ed≡1modλ(pq)
    
    mpz_clears(pq_threshold, tmp, tmp2, p_1, q_1, NULL);
    return status;
}


int RSA_generate_key(mpz_t N, mpz_t p, mpz_t q, mpz_t e, mpz_t d,
		      int nlen, int sec, gmp_randstate_t state){
    int status = 1;
/* to be filled in */

    while(1){
        RSA_weak_generate_key(p, q, e, d, nlen, state);
        if(is_valid_key(p, q, e, d, nlen, sec) == 1){
            break;
        }
    }

    mpz_mul(N, p, q);
    return status;
}



int RSA_encrypt(mpz_t cipher, mpz_t msg, mpz_t N, mpz_t e){
    int status = 1;
    // complete the function
/* to be filled in */
    mpz_powm(cipher, msg, e, N);
    return status;
}

int RSA_decrypt(mpz_t msg, mpz_t cipher, mpz_t N, mpz_t d){
    int status = 1;
    // complete the function
/* to be filled in */
    mpz_powm(msg, cipher, d, N);
    return status;
}


/* Use CRT. */
int RSA_decrypt_with_p_q(mpz_t msg, mpz_t cipher, mpz_t N, mpz_t d,
			 mpz_t p, mpz_t q){
    int status = 1;
    // complete the function
/* to be filled in */

    mpz_t y_d, r0, r1, m0, m1, fi_p, fi_q;
    mpz_inits(y_d, r0, r1, m0, m1, fi_p, fi_q, NULL);  
    mpz_sub_ui(fi_p, p, 1);
    mpz_sub_ui(fi_q, q, 1);
    mpz_mod(r0, cipher, p); // cipher mod p
    mpz_mod(r1, cipher, q); // cipher mod q
    mpz_mod(m0, d, fi_p); // m0 = d mod (p - 1)
    mpz_mod(m1, d, fi_q); // m0 = d mod (q - 1)
    mpz_powm(r0, r0, m0, N);// r0 = (cipher mod p) ^ (d mod (p-1))
    mpz_powm(r1, r1, m1, N);// r1 = (cipher mod q) ^ (d mod (q-1))

    CRT2(msg, r0, p, r1, q);
    mpz_clears(y_d, NULL);

    return status;
}



void RSA_dummy_generate_key(mpz_t N, mpz_t e, int nlen,
			    gmp_randstate_t state){
	
    mpz_t g, p, q, lambda, pm1, qm1;
    mpz_inits(g, p, q, lambda, pm1, qm1, NULL);

    do{
	mpz_urandomb(p, state, nlen/2);
	mpz_nextprime(p, p);		
	do{
	    mpz_urandomb(q, state, nlen/2);
	    mpz_nextprime(q, q);
	}while(mpz_cmp(p, q) == 0);
		
	mpz_sub_ui(pm1, p, 1);
	mpz_sub_ui(qm1, q, 1);
	mpz_lcm(lambda, pm1, qm1);
	mpz_gcd(g, e, lambda);
	mpz_mul(N, p, q);
	
    }while(mpz_cmp_ui(g, 1) != 0 || mpz_sizeinbase(N, 2) < nlen);
#if DEBUG
    gmp_printf("[RSA_dummy_generate_key] N = %Zd, size : %ld\n",
	       N, mpz_sizeinbase(N, 2));
#endif
	
    mpz_clears(g, p, q, lambda, pm1, qm1, NULL);
}
