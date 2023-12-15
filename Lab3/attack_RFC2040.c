/*************************************************************
attack_RFC_2040.c
author Alain Couvreur : alain.couvreur@lix.polytechnique.fr
Last modification : October 8, 2018

**************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include "buffer.h"
#include "sha3.h"
#include "aes.h"
#include "operating_modes.h"
#include "attack_RFC2040.h"

#define DEBUG 1

// Returns 1 if the cipher text is valid and had been padded with
//   RFC2040 
//   Else return 0.
//   If the cipher text has not the good length, returns -1. 
int oracle(buffer_t *encrypted, buffer_t *key){
    if(encrypted->length % BLOCK_LENGTH != 0){
	perror("[oracle] ERROR : cipher text has not a valid length.\n");
	return -1;
    }

    buffer_t decrypted;
    buffer_init(&decrypted, encrypted->length - BLOCK_LENGTH);	
    aes_raw_CBC_decrypt(&decrypted, encrypted, key);
	
    uchar *cursor = decrypted.tab + decrypted.length - 1;
    uchar a = *cursor;
    if(a == 0 || a > BLOCK_LENGTH){
	buffer_clear(&decrypted);
	return 0;
    }
    cursor--;
    int result = 1;

    for(int i = 1; i < a; i++, cursor--){
	if(*cursor != a){
	    result = 0;
	    break;
	}
    }
	
    buffer_clear(&decrypted);
    return result;
}


/* Returns the position of the first byte of padding */
/* in the last block of the encrypted_text           */
int get_padding_position(buffer_t *encrypted, buffer_t *key){
    if(encrypted->length % BLOCK_LENGTH != 0 ||
       encrypted->length / BLOCK_LENGTH < 2){
	perror("[get_padding_position] ERROR : cipher text has not a valid length.\n");
	return -1;
    }
    if(!oracle(encrypted, key)){
	perror("[get_padding_position] ERROR : input is not a valid ciphertext.\n");
	return -1;
    }

    // Complete the function
    int first_padding = 0;
    buffer_t tmp_encrypted; // create a tmp buffer to store the value of encrypted
    buffer_init(&tmp_encrypted, encrypted->length);

    for(int j = 0; j < encrypted->length; j++)
		buffer_append_uchar(&tmp_encrypted, encrypted->tab[j]);//  copy the encrypted to the tmp buffer

    for (int i=0; i<encrypted->length; i++){
        tmp_encrypted.tab[i] = encrypted->tab[i] ^ 1;
        if(!oracle(&tmp_encrypted, key)){
            first_padding = i;
            break;
        }
    }

/* to be filled in */
    return first_padding%16; // each element has 16 bytes
}


int prepare(buffer_t *corrupted, buffer_t *encrypted, buffer_t *decrypted,
	    int known_positions){
    /* encrypted has length a * BLOCK_LENGTH + 1
       decrypted has length a * BLOCK_LENGTH.
       CAUTION : encrypted has one block more than decrypted
       because of the IV that is its leftmost block

       decrypted contains the exact entries of the padded plaintext
       from position known_positions - BLOCK_LENGTH to the last one.
       Equivalently, on encrypted, bytes after position known_positions
       are known.
    */

   // First initialize the corrupted buffer
   buffer_reset(corrupted);
   buffer_clone(corrupted, encrypted);

   int block_length = encrypted->length - decrypted->length;
   int start_position = known_positions - block_length; // known_positions - BLOCK_LENGTH
   int a = block_length - known_positions%block_length;
   
   int corrupted_length =  known_positions + block_length - known_positions%block_length; // Part of the block

   if((decrypted->length - known_positions) == 0 || a == block_length){ // Detect the number
		a = 0;
		corrupted_length = known_positions + known_positions%block_length;
		start_position = known_positions - block_length - block_length;
	}
    // printf("\n--------------------------------------encrypted->length:%d--------------------------------------\n", (int)encrypted->length);
    // printf("\n--------------------------------------decrypted->length:%d--------------------------------------\n", (int)decrypted->length);
    // printf("\n--------------------------------------known_positions:%d--------------------------------------\n", known_positions);
    // printf("\n--------------------------------------corrupted_length:%d--------------------------------------\n", corrupted_length);
    // printf("\n--------------------------------------a:%d--------------------------------------\n", a);
   for(int i = start_position; i < corrupted_length - block_length; i++){
		corrupted->tab[i] = corrupted->tab[i] ^ (a+1) ^ a;
	}
	corrupted->tab[start_position - 1] = corrupted->tab[start_position - 1] ^ (a + 1);
	corrupted->length = corrupted_length;
	
    
    // fill in
/* to be filled in */
    return 0;
}



int find_last_byte(uchar *hack, buffer_t *corrupted, int position, buffer_t *key){
    if(corrupted->length < 2 * BLOCK_LENGTH){
	perror("[find_last_byte] ERROR : cipher_text is too short.\n");
	*hack = 0;
	return 0;
    }
    if(corrupted->length % BLOCK_LENGTH != 0){
	perror("[find_last_byte] ERROR : not a valid cipher text.\n");
	*hack = 0;
	return 0;
    }	

    // Complete the function	
    int tmp = corrupted->tab[position - BLOCK_LENGTH];

	for (int i=0; i<256; i++){ // Max is 256
		corrupted->tab[position - BLOCK_LENGTH] = tmp ^ (*hack);
		if(oracle(corrupted, key)){
            break;
        }
		(*hack)++;
	}

/* to be filled in */
    return *hack;
}


int full_attack(buffer_t *decrypted, buffer_t *encrypted, buffer_t *key) {

	 int pad_position = get_padding_position(encrypted, key);
	
	 uchar pad_value = BLOCK_LENGTH - pad_position;
	 decrypted->length = encrypted->length - BLOCK_LENGTH;
	 int pad_block = encrypted->length - BLOCK_LENGTH + pad_position;
	 buffer_t corrupted;
	 uchar temp_last_v;
     printf("\n--------------------------------------decrypted->length:%d--------------------------------------\n", (int)decrypted->length);
     printf("\n--------------------------------------pad_block:%d--------------------------------------\n", pad_block);

	buffer_init(&corrupted, encrypted->length);
	for(int i = decrypted->length; i >= (decrypted->length - pad_value); i--){
        printf("--------------pad_value:%d\n",pad_value);
		decrypted->tab[i] = pad_value;
	}

	for(int i = pad_block; i >= pad_block - 54; i--){
		prepare(&corrupted, encrypted, decrypted, i);
		find_last_byte(&temp_last_v, &corrupted, i-1, key);
        printf("--------------temp_last_v:%d\n",temp_last_v);
		decrypted->tab[i-16] = temp_last_v ;
	}

    return 1;
}

