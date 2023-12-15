#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "gmp.h"

#include "utilities.h"

#include "buffer.h"
#include "sha3.h"
#include "rsa.h"
#include "sign.h"

#define DEBUG 0


int RSA_generate_key_files(const char *pk_file_name,
			   const char *sk_file_name,
			   size_t nbits, int sec, gmp_randstate_t state){
    int status = 1;
/* to be filled in */
    mpz_t N, p, q, e, d;
    mpz_inits(N, p, q, e, d, NULL);

    RSA_generate_key(N, p, q, e, d, nbits, sec, state);
    // Public key
    FILE *fp_public = fopen(pk_file_name, "w+");
    gmp_fprintf(fp_public, "#RSA Public key (%d bits):\nN = %#Zx\ne = %#Zx\n\t", nbits, N, e);
    fclose(fp_public);

    // Secret key
    FILE *fp_secret = fopen(sk_file_name, "w");
    gmp_fprintf(fp_public, "#RSA Secret key (%d bits):\nN = %#Zx\nd = %#Zx\n\t", nbits, N, d);
    fclose(fp_secret);

    mpz_clears(N, p, q, e, d, NULL);


    return status;
}


void RSA_key_import(mpz_t N, mpz_t ed, const char *key_file_name){
    FILE *key = fopen(key_file_name, "r");
    /* Go to second line, then move from 6 characters to the right */
    while(fgetc(key) != '\n');
    fseek(key, 6, SEEK_CUR);

    /* Scan the modulus N */
    gmp_fscanf(key, "%Zx", N);

    /* Same for e or d*/
    while(fgetc(key) != '\n');
    fseek(key, 6, SEEK_CUR);
    gmp_fscanf(key, "%Zx", ed);

    fclose(key);
}


int hash_length(mpz_t N){
    int bit_size_N = mpz_sizeinbase(N, 2);
    return (bit_size_N % BYTE_SIZE == 0) ?
	bit_size_N / BYTE_SIZE - 1 : (bit_size_N / BYTE_SIZE);
}


int RSA_sign_buffer(mpz_t sgn, buffer_t *msg,
		    mpz_t N, mpz_t d){
    int status = 1;
/* to be filled in */
    buffer_t hash;
    buffer_init(&hash, msg->size);

    mpz_t tmp;
    mpz_init(tmp);

    buffer_hash(&hash, hash_length(N), msg);
    mpz_import(tmp, hash_length(N), 1, 1, 1, 0, hash.tab);
    RSA_encrypt(sgn, tmp, N, d);

    mpz_clear(tmp);
    buffer_clear(&hash);

    return status;
}



// int RSA_verify_signature(mpz_t sgn, buffer_t *msg,
// 			 mpz_t N, mpz_t e){
//     int verify = 0;
    
//     int length = hash_length(N);
//     buffer_t hash, hash2;
//     mpz_t mpz_hash;
//     mpz_init(mpz_hash);
//     buffer_init(&hash, length);
//     buffer_init(&hash2, length);
	
//     RSA_encrypt(mpz_hash, sgn, N, e);
// #if DEBUG > 0
//     gmp_printf("GMP-ized hash in verification :\n%#Zx\n\n", mpz_hash);
// #endif
	
//     mpz_export(hash.tab, NULL, 1, 1, 1, 0, mpz_hash);
//     hash.length = length;
//     buffer_hash(&hash2, length, msg);
//     verify = buffer_equality(&hash, &hash2);
	
//     buffer_clear(&hash);
//     buffer_clear(&hash2);
//     mpz_clear(mpz_hash);
//     return verify;
// }


int RSA_verify_signature(mpz_t sgn, buffer_t *msg,
			 mpz_t N, mpz_t e){
    int verify = 0;
/* to be filled in */

    buffer_t hash;
    buffer_init(&hash, msg->size);

    mpz_t msg_tmp, msg_decrypt;
    mpz_inits(msg_tmp, msg_decrypt, NULL);

    buffer_hash(&hash, hash_length(N), msg);
    mpz_import(msg_tmp, hash_length(N), 1, 1, 1, 0, hash.tab);

    RSA_decrypt(msg_decrypt, sgn, N, e);

    if(mpz_cmp(msg_decrypt, msg_tmp) == 0)
        verify = 1;

    return verify;
}


void RSA_signature_import(mpz_t S, const char* signature_file_name){
    FILE *sgn = fopen(signature_file_name, "r");
    while(fgetc(sgn) != '\n');
    fseek(sgn, 6, SEEK_CUR);
    gmp_fscanf(sgn, "%Zx", S);
    fclose(sgn);
}


void RSA_sign(const char* file_name, const char* key_file_name,
	      const char* signature_file_name){
    // 1. Initialisation
    buffer_t msg;
    mpz_t N, d, signature;
    mpz_inits(N, d, signature, NULL);
    buffer_init(&msg, 100);

    // 2. Import the message in a buffer
    buffer_from_file(&msg, file_name);
	
    // 3. Parse the secret key
    RSA_key_import(N, d, key_file_name);

    // 4. Sign the buffer
    int status = RSA_sign_buffer(signature, &msg, N, d);
    implementation_check("RSA_sign_buffer", status);

    // 5. Exports the signature in a file
    FILE* sgn = fopen(signature_file_name, "w");
    gmp_fprintf(sgn, "#RSA signature\nS = %#Zx\n", signature);
	
    // 6. Close and free
    fclose(sgn);
    mpz_clears(N, d, signature, NULL);
    buffer_clear(&msg);
}


int RSA_verify(const char* file_name, const char* key_file_name,
	       const char* signature_file_name){
    // 1. Initialisation
    buffer_t msg;
    mpz_t N, e, S;	
    buffer_init(&msg, 100);
    mpz_inits(N, e, S, NULL);	

	
    // 2. Import the message into a buffer
    buffer_from_file(&msg, file_name);

    // 3. Import the public key
    RSA_key_import(N, e, key_file_name);

    // 4. Parse the signature
    RSA_signature_import(S, signature_file_name);
	
    // 5. Verify
    int verify = RSA_verify_signature(S, &msg, N, e);
    implementation_check("RSA_verify_signature", verify);
	
    // 6. Close, free and return
    mpz_clears(S, N, e, NULL);
    buffer_clear(&msg);
    return verify;
}
