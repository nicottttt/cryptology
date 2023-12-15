#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "gmp.h"

#include "utilities.h"

#include "buffer.h"
#include "sha3.h"

#include "rsa.h"
#include "sign.h"
#include "dsa.h"

#define DEBUG 0

void generate_probable_prime(mpz_t p, int psize,
			     gmp_randstate_t state){
    do{
	mpz_rrandomb(p, state, psize);
	mpz_nextprime(p, p);
    }while(mpz_sizeinbase(p, 2) < psize);
}

int generate_pq(mpz_t p, mpz_t q, size_t psize, size_t qsize,
		gmp_randstate_t state){
    int status = 1;
/* to be filled in */

    mpz_t tmp;
    mpz_init_set_ui(tmp, 0);

    /* 
    Need to generate psize p by using qsize q, the value that multiply
    should approximate to 2^psize / 2^qsize. If traverse from 1, it will
    take a lot of time
    */
    mpz_ui_pow_ui(tmp, 2, (psize - qsize));

    // mpz_rrandomb(q, state, qsize);
	// mpz_nextprime(q, q);
    generate_probable_prime(q, qsize, state);

    while(1){
        mpz_add_ui(tmp, tmp, 1);
        mpz_mul(p, q, tmp);
        mpz_add_ui(p, p, 1);
        if(mpz_sizeinbase(p, 2) == psize && mpz_probab_prime_p(p, 10) != 0)
            break;
    }

    mpz_clear(tmp);

    return status;
}
	
int dsa_generate_keys(mpz_t p, mpz_t q, mpz_t a, mpz_t x, mpz_t y,
		      size_t psize, size_t qsize, gmp_randstate_t state){
    int status = 1;
/* to be filled in */
    mpz_t h, tmp;
    mpz_inits(h, tmp, NULL);

    generate_pq(p, q, psize, qsize, state);
    mpz_sub_ui(tmp, p, 1);
    mpz_div(tmp, tmp, q);

    while (1){
        mpz_urandomb(h, state, psize);
        mpz_powm(a, h, tmp, p); // a = h^((p-1)/q) mod p
        if (mpz_cmp_ui(a, 1) != 0) // a != 1
            break;
    }

    mpz_urandomm(x, state, q);
    mpz_powm(y, a, x, p);

    mpz_clears(h, tmp, NULL);
    return status;
}


void dsa_generate_key_files(const char* pk_file_name, const char* sk_file_name,
			    size_t psize, size_t qsize,
			    gmp_randstate_t state){
    // 1. INITS
    mpz_t p, q, a, x, y;
    mpz_inits(p, q, a, x, y, NULL);
    FILE* pk = fopen(pk_file_name, "w");
    FILE* sk = fopen(sk_file_name, "w");
	
    // 2. Key generation
    dsa_generate_keys(p, q, a, x, y, psize, qsize, state);

    // 3. Printing files
    fprintf(pk, "#DSA public key (%lu bits, %lu bits):\n", psize, qsize);
    gmp_fprintf(pk, "p = %#Zx\nq = %#Zx\na = %#Zx\ny = %#Zx\n", p, q, a, y);
    fprintf(sk, "#DSA Private Key (%lu bits, %lu bits):\n", psize, qsize);
    gmp_fprintf(sk, "p = %#Zx\nq = %#Zx\na = %#Zx\nx = %#Zx\n", p, q, a, x);
	
    // 4. Cleaning
    mpz_clears(p, q, a, x, y, NULL);
    fclose(pk);
    fclose(sk);
}


void dsa_key_import(const char* key_file_name, mpz_t p, mpz_t q, mpz_t a,
		    mpz_t xy){
    FILE* key = fopen(key_file_name, "r");
	
    // Go to second line, then move from 6 characters to the right
    while(fgetc(key) != '\n');
    fseek(key, 6, SEEK_CUR);

    // Scan the modulus p
    gmp_fscanf(key, "%Zx", p);

    // Same for q
    while(fgetc(key) != '\n');
    fseek(key, 6, SEEK_CUR);
    gmp_fscanf(key, "%Zx", q);

    // Same for a
    while(fgetc(key) != '\n');
    fseek(key, 6, SEEK_CUR);
    gmp_fscanf(key, "%Zx", a);

    // Same for x or y
    while(fgetc(key) != '\n');
    fseek(key, 6, SEEK_CUR);
    gmp_fscanf(key, "%Zx", xy);

    fclose(key);
}


int dsa_sign_buffer(buffer_t *msg, mpz_t p,
		    mpz_t q, mpz_t a, mpz_t x, mpz_t r, mpz_t s,
		    gmp_randstate_t state){
    int status = 0;
/* to be filled in */

    buffer_t hash;
    mpz_t hash_msg, k, tmp, inv_k;
    mpz_inits(hash_msg, k, tmp, inv_k, NULL);

    // Generate hash of msg
    buffer_init(&hash, hash_length(q));
    buffer_hash(&hash, hash_length(q), msg);
    mpz_import(hash_msg, hash_length(q), 1, 1, 1, 0, hash.tab);

    // Generate sign

    mpz_urandomb(k, state, mpz_sizeinbase(q, 2));
    mpz_powm(tmp, a, k, p); // tmp = a^k mod p
    mpz_mod(r, tmp, q); // r = tmp mod q

    mpz_invert(inv_k, k, q); 
    mpz_mul(tmp, x, r);  // tmp = x*r
    mpz_add(tmp, tmp, hash_msg); // tmp = H(m) + x*r
    mpz_mul(tmp, tmp, inv_k); // tmp = tmp * k^(-1)
    mpz_mod(s, tmp, q);


    mpz_clears(hash_msg, k, tmp, inv_k, NULL);
    buffer_clear(&hash);

    return status;
}


void dsa_sign(const char* file_name, const char* key_file_name,
	      const char* signature_file_name,
	      gmp_randstate_t state){
    // 1. Initialisation
    mpz_t p, q, a, x, r, s;
    buffer_t msg;
    mpz_inits(r, s, p, q, a, x, NULL);
    buffer_init(&msg, 100);
	
    // 2. Import the message
    buffer_from_file(&msg, file_name);
#if DEBUG
    printf("Length of the message = %lu.\n", msg.length);
#endif

    /* 3. Parse the secret key */
    dsa_key_import(key_file_name, p, q, a, x);
#if DEBUG > 0
    gmp_printf("p = %#Zx\nq = %#Zx\n", p, q);
#endif
	
    /* 4. Sign */
    dsa_sign_buffer(&msg, p, q, a, x, r, s, state);

    /* 5. Write signature in a file */
    FILE* sgn = fopen(signature_file_name, "w");
    gmp_fprintf(sgn, "#DSA signature:\nr = %#Zx\ns = %#Zx\n", r, s);
	
    /* . Cleaning */
    mpz_clears(p, q, a, x, r, s, NULL);
    fclose(sgn);
    buffer_clear(&msg);
}


int dsa_verify_buffer(buffer_t *msg, mpz_t p, mpz_t q,
		      mpz_t a, mpz_t r, mpz_t s, mpz_t y){
    int verify = 0;
/* to be filled in */

    // Verify if r, s legal
    if (mpz_cmp_ui(r, 0) <= 0 || mpz_cmp(r, q) >= 0 || 
        mpz_cmp_ui(s, 0) <= 0 || mpz_cmp(s, q) >= 0) {
        return 0;
    }

    buffer_t hash;
    mpz_t hash_msg, w, u1, u2, tmp, g_u1, y_u2, v;
    mpz_inits(hash_msg, w, u1, u2, tmp, g_u1, y_u2, v, NULL);

    // Generate hash of msg
    buffer_init(&hash, hash_length(q));
    buffer_hash(&hash, hash_length(q), msg);
    mpz_import(hash_msg, hash_length(q), 1, 1, 1, 0, hash.tab);

    mpz_invert(w, s, q);

    //u1 = H(M)*w mod q and u2 = r*w mod q
    mpz_mul(u1, hash_msg, w);
    mpz_mod(u1, u1, q);
    mpz_mul(u2, r, w);
    mpz_mod(u2, u2, q);

    // v = (a^u1 * y^u2 mod p) mod q
    mpz_powm(g_u1, a, u1, p);
    mpz_powm(y_u2, y, u2, p);
    mpz_mul(tmp, g_u1, y_u2);
    mpz_mod(tmp, tmp, p);
    mpz_mod(v, tmp, q);

    verify = !mpz_cmp(v, r);

    mpz_clears(hash_msg, w, u1, u2, tmp, g_u1, y_u2, v, NULL);
    buffer_clear(&hash);

    return verify;
}


void dsa_import_signature(mpz_t r, mpz_t s, const char* signature_file_name){
    FILE* sgn = fopen(signature_file_name, "r");
    while(fgetc(sgn) != '\n');
    fseek(sgn, 6, SEEK_CUR);
    gmp_fscanf(sgn, "%Zx", r);
	
    while(fgetc(sgn) != '\n');
    fseek(sgn, 6, SEEK_CUR);
    gmp_fscanf(sgn, "%Zx", s);
	
    fclose(sgn);
}


int dsa_verify(const char* file_name, const char* key_file_name,
	       const char* signature_file_name){
    // 1. INIT
    mpz_t p, q, a, y, r, s;
    buffer_t msg;
    mpz_inits(p, q, a, y, r, s, NULL);
    buffer_init(&msg, 100);
	
    // 2. Imports the message
    buffer_from_file(&msg, file_name);
	
    // 3. Parse the public key 
    dsa_key_import(key_file_name, p, q, a, y);

#if DEBUG > 0
    gmp_printf("\n\np = %#Zx\nq = %#Zx\n\n", p, q);
#endif
	
    // 4. Parse the signature 
    dsa_import_signature(r, s, signature_file_name);
    int verify = dsa_verify_buffer(&msg, p, q, a, r, s, y);
	
    // 5. Cleaning and return
    mpz_clears(p, q, a, y, r, s, NULL);
    buffer_clear(&msg);
    return verify;
}
