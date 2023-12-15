#include <stdio.h>
#include <stdlib.h>

#include "utilities.h"

#include "gmp.h"
#include "buffer.h"
#include "rsa.h"
#include "text_rsa.h"

#define DEBUG 0


int lengths(int *block_length, int *cipher_length, int *last_block_size,
	    buffer_t *msg, mpz_t N){
    int status = 1;
/* to be filled in */

    *cipher_length = msg->length;
    *block_length = mpz_sizeinbase(N, 2) / 8 - 1;
    *last_block_size = *cipher_length % *block_length;
    *cipher_length = ((float)*cipher_length / (*block_length) + 0.5);

    if(*last_block_size == 0)
        *last_block_size = *block_length;

    return status;
}


int RSA_text_encrypt(mpz_t *cipher, int block_length,
		     int cipher_length, int last_block_size,
		     buffer_t *msg, mpz_t N, mpz_t e){
    // cipher is a table of mpz_t of length cipher_length.
    // Memory allocation and initialisation of the cells is
    // already done.

    // block_length denotes the size of blocks of uchar's
    // which will partition the message.
    // last_block_size denotes the size of the last block. It may
    // be 0.
    int status = 1;

/* to be filled in */
    mpz_t msg_en;
    mpz_init(msg_en);

    // only loop cipher_length - 1 round, using block_length
    for (int i = 0; i < cipher_length - 1; i++)
    {
        mpz_import(msg_en, block_length, 1, 1, 0, 0, msg->tab + i * block_length);
        RSA_encrypt(cipher[i], msg_en, N, e);
    }

    // the final use last_block_size
    mpz_import(msg_en, last_block_size, 1, 1, 0, 0, msg->tab + (cipher_length - 1) * block_length);
    RSA_encrypt(cipher[cipher_length - 1], msg_en, N, e);
    mpz_clear(msg_en);

    return status;
}

int RSA_text_decrypt(buffer_t *decrypted, mpz_t *cipher,
		     int cipher_length, int block_length,
		     int last_block_size,
		     mpz_t N, mpz_t d){

    int status = 1;
    // buffer decrypted is supposed to be initialised.
    buffer_reset(decrypted);
	
/* to be filled in */
    mpz_t decrypt_mpz;
    
    mpz_inits(decrypt_mpz, NULL);

    // for buffer_append_uchar
    uchar *decrypt_append = (uchar *)malloc(block_length);

    // mpz_export need the data structure size_t
    size_t cnt;

    for (int i = 0; i < cipher_length; i++)
    {
        RSA_decrypt(decrypt_mpz, cipher[i], N, d);
        mpz_export(decrypt_append, &cnt, 1, 1, 0, 0, decrypt_mpz);
        
        for (int j = 0; j < cnt; j++)
            buffer_append_uchar(decrypted, decrypt_append[j]);
    }

    return status;
}
