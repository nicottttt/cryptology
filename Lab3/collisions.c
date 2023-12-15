/**************************************************************/
/* collisions.c                                               */
/* Author : Matthieu Lequesne                                 */
/* Last modification October 5, 2023                          */
/**************************************************************/

#include <stdio.h>
#include <stdlib.h>

#include "random.h"
#include "hashtable.h"
#include "easyhash.h"
#include "collisions.h"

int find_collisions(int imax){
    hash_table H;
    buffer_t buf; // The current buffer you work on
    buffer_t *tab; // A table to store buffers you treated
    int status = 1;
/* to be filled in */
    hash_key k;
    hash_pair kv;

    H = hash_init(imax);
    buffer_init(&buf, 4);
    
    tab = (buffer_t *) malloc(imax * sizeof(buffer_t));

    // Complete the function here...
    for (long i=0; i<imax; i++){
        buffer_random(&buf, 4);
        //*(tab + i) = buf;
        buffer_clone(tab+i, &buf);
        k = easy_hash(&buf);
        if(hash_get(&kv, H, k) == HASH_FOUND){
            print_collision(&buf, tab + kv.v);
            //printf("collision happen\n");
        } else {
            hash_put(H, k, i);
        }
    }
/* to be filled in */
    buffer_clear(&buf);
    hash_clear(H);
    free (tab);
    return status;
}

void print_collision(buffer_t *value1, buffer_t *value2){
    unsigned long h1 = (unsigned long) easy_hash(value1);
    unsigned long h2 = (unsigned long) easy_hash(value2);

    if (h1!=h2){
        perror("[print_collision] Hash values are not equal!");
        return;
    }

    if (buffer_equality(value1, value2) != 0){
        perror("[print_collision] Buffer values are equal!");
        return;
    }

    printf("collision found: \n");
    printf("h(");
    buffer_print_int(stdout, value1);
    printf(") = %ld\n", h1);
    printf("h(");
    buffer_print_int(stdout, value2);
    printf(") = %ld\n\n", h2);
}
