/**************************************************************/
/* diffusion.c                                                */
/* Author : Alain Couvreur                                    */
/* alain.couvreur@lix.polytechnique.fr                        */
/* Last modification October 6, 2022 by FJM                   */
/* Last modification October 12, 2018                         */
/**************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include "buffer.h"
#include "random.h"
#include "bits.h"
#include "aes.h"
#include "diffusion.h"


double diffusion_test_for_key(buffer_t *key, int nr_tests){
    int length = key->length;
    double result = 0;
    // 1. Intialisation
    buffer_t msg, key2, encrypted, encrypted2;
    buffer_init(&msg, length);
    buffer_init(&key2, length);
    buffer_init(&encrypted, length);
    buffer_init(&encrypted2, length);

    // Complete the function

    for(int i=0; i<nr_tests; i++){
        buffer_random(&msg, length);
        aes_block_encrypt(&encrypted, &msg, key);
        int position = rand() % (length * 8); // [0, L(bits)]
        buffer_flip_bit(&key2, key, position); 
        aes_block_encrypt(&encrypted2, &msg, &key2);
        result += HammingDistance(&encrypted, &encrypted2); 
    }

/* to be filled in */
    // 3. Free memory
    buffer_clear(&msg);
    buffer_clear(&key2);	
    buffer_clear(&encrypted);	
    buffer_clear(&encrypted2);	
    return result / nr_tests;
}


double diffusion_test_for_msg(buffer_t *msg, int nr_tests){
    int length = msg->length;
    double result = 0;
    // 1. Intialisation
    buffer_t key, msg2, encrypted, encrypted2;
    buffer_init(&key, length);
    buffer_init(&msg2, length);
    buffer_init(&encrypted, length);
    buffer_init(&encrypted2, length);
    // Complete the function

    for(int i=0; i<nr_tests; i++){
        buffer_random(&key, length);
        aes_block_encrypt(&encrypted, msg, &key);
        int position = rand() % (length * 8); // [0, L(bits)]
        buffer_flip_bit(&msg2, msg, position); 
        aes_block_encrypt(&encrypted2, &msg2, &key);
        result += HammingDistance(&encrypted, &encrypted2); 
    }

/* to be filled in */
    buffer_clear(&key);
    buffer_clear(&msg2);	
    buffer_clear(&encrypted);	
    buffer_clear(&encrypted2);	
    return result / nr_tests;
}


double diffusion_test_nr_rounds(buffer_t *msg, int Nr, int nr_tests){
    int length = msg->length;
    double result = 0;
    // 1. Intialisation
    buffer_t key, msg2, encrypted, encrypted2;
    buffer_init(&key, length);
    buffer_init(&msg2, length);
    buffer_init(&encrypted, length);
    buffer_init(&encrypted2, length);
    // Complete the function

    for(int i=0; i<nr_tests; i++){
        buffer_random(&key, length);
        aes_block_encrypt_few_rounds (&encrypted, msg, &key, Nr);
        int position = rand() % (length * 8); // [0, L(bits)]
        buffer_flip_bit(&msg2, msg, position); 
        aes_block_encrypt_few_rounds(&encrypted2, &msg2, &key, Nr);
        result += HammingDistance(&encrypted, &encrypted2); 
    }

/* to be filled in */
    buffer_clear(&key);
    buffer_clear(&msg2);	
    buffer_clear(&encrypted);	
    buffer_clear(&encrypted2);	

/* to be filled in */
    return result / nr_tests;
}
