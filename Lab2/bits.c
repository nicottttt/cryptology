/**************************************************************/
/* bits.c                                                     */
/* Author : Alain Couvreur                                    */
/* alain.couvreur@lix.polytechnique.fr                        */
/* Modifications: Maxime Bombar (maxime.bombar@inria.fr)      */
/* Last modification September 29, 2022 (Maxime Bombar)       */
/**************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include "buffer.h"
#include "bits.h"

void printDec(uchar* u, int length){
    int i;
    
    printf("[");
    if(length == 0){
	printf("]");
	return;
    }
    for(i  = 0; i < length - 1; i++){
	printf("%d, ", *(u++));
    }
    printf("%d]", *u);
}


void printHexa(uchar* u, int length){
    int i;
    
    printf("[");
    if(length == 0){
	printf("]");
	return;
    }
    for(i  = 0; i < length - 1; i++){
	if(*u < 0x10)
	    printf("0x0%x, ", *(u++));
	else
	    printf("0x%x, ", *(u++));
    }
    if(*u < 0x10)
	printf("0x0%x]", *(u++));
    else
	printf("0x%x]", *(u++));
}


void printBin(uchar* u, int length){
    int i, j;

    printf("[ ");
    if(length == 0){
	printf("]");
	return;
    }
    for(i  = 0; i < length - 1; i++){
	for(j = BYTE_SIZE - 1; j >= 0; j--){
	    printf("%d ", (*u >> j) & 1);
	}
	printf("| ");
	u++;
    }
    for(j = BYTE_SIZE - 1; j >= 0 ; j--){
	printf("%d ", (*u >> j) & 1);
    }
    printf("]");
}



uchar getBit(uchar t, int position){
/* to be filled in */
    int shift = 1 << position;
    return !!(t & shift);
}



uchar setBit(uchar t, int position, uchar value){
/* to be filled in */
    int shift = !(value) << position; // Not !! is because later I will use ~
    t = t | (1 << position); // Set the position of t be 1, later can &
    
    return t & (~shift);
}


int HammingWeightByte(uchar c){
/* to be filled in */
    int cnt = 0;
    for (int i=0; i<8; i++){
        cnt += getBit(c,i);
    }
    return cnt;
}


int HammingWeight(buffer_t *buf){
/* to be filled in */
    int cnt = 0;
    for (int i=0; i< buf->length; i++){
        for (int j=0; j<8; j++){
            cnt += getBit(buf->tab[i],j);
        }
    }
    return cnt;
}



void oneTimePad(buffer_t *encrypted, buffer_t *msg, buffer_t *key) {
/* to be filled in */
    buffer_reset(encrypted);

    uchar *pt_msg = msg->tab;
    uchar *pt_key = key->tab;

    uchar tmp;

    for (int i=0; i<msg->length; i++){
    // Operation by bits
        // tmp = 0;
        // for (int j=0; j<8; j++){
        //     //setBit(tmp, j, getBit(pt_msg[i], j) ^ getBit(pt_key[i], j)); 
        //     int value = getBit(pt_msg[i], j) ^ getBit(pt_key[i], j);
        //     int shift = !(value) << j; // Not !! is because later I will use ~
        //     tmp = tmp | (1 << j); // Set the position of t be 1, later can &
        
        //     tmp = tmp & (~shift);
        // }
        // buffer_append_uchar(encrypted, tmp);
        
    // Operation by bytes
        tmp = pt_msg[i] ^ pt_key[i];
        buffer_append_uchar(encrypted, tmp);
    }
}
