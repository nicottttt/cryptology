/**************************************************************/
/* Geffe.c                                                    */
/* Author : Alain Couvreur                                    */
/* alain.couvreur@lix.polytechnique.fr                        */
/* Last modification September 24, 2018                       */
/**************************************************************/

#include <stdio.h>
#include "buffer.h"
#include "bits.h"
#include "LFSR.h"
#include "Geffe.h"
#define DEBUG 1


void Geffe(buffer_t *output, buffer_t *s1, buffer_t *s2, buffer_t *s3){
/* to be filled in */
	uchar *pt_s1 = s1->tab;
	uchar *pt_s2 = s2->tab;
	uchar *pt_s3 = s3->tab;

	uchar tmp;
	// Do the operation in bytes
	for (int i=0; i<s1->length; i++){
        tmp = ((pt_s1[i] & pt_s2[i]) ^ (pt_s2[i] & pt_s3[i])) ^ pt_s3[i];
        buffer_append_uchar(output, tmp);
    }

}


double correlation(buffer_t *s1, buffer_t *s2){
/* to be filled in */
	uchar *pt_s1 = s1->tab;
	uchar *pt_s2 = s2->tab;

	double equal = 0;
	double cnt = 0;

	// Calculate bit by bit
	for (int i=0; i<s1->length; i++){
		for (int j=0; j<8; j++){
			cnt += 1;
			if(getBit(pt_s1[i],j) ==  getBit(pt_s2[i],j))
				equal += 1;
		}
		
	}

	double correlation = equal / cnt;

	return correlation;
}


void searchIV(buffer_t *IV_candidate, buffer_t *stream, buffer_t *trans, double threshold){
	buffer_t stream_candidate;
	int i;
	
	buffer_init(&stream_candidate, trans->length);
	buffer_reset(IV_candidate);
	for(i = 0; i < trans->length; i++)
		buffer_append_uchar(IV_candidate, 0);

/* to be filled in */
	while(1){
		LFSR(&stream_candidate, trans, IV_candidate, stream->length);
		if (correlation(&stream_candidate, stream) > threshold)
			break;
		increment_buffer(IV_candidate);
	}
	
	buffer_clear(&stream_candidate);
}


void positions(buffer_t *output, buffer_t *s1, buffer_t *s3){
	if(s1->length != s3->length){
		perror("Input streams should have the same length.\n");
		return;
	}
	buffer_reset(output);
/* to be filled in */
	uchar *pt_s1 = s1->tab;
	uchar *pt_s3 = s3->tab;
	//uchar *pt_output = output->tab;
	int shift;
	uchar t;

	for (int i=0; i<s1->length; i++){
		t = 0;
		for (int j=0; j<8; j++){
			if((getBit(pt_s1[i],j) == 1) && (getBit(pt_s3[i],j) == 0)){
				// Use the logic of setBit
				shift = !(1) << j;
				t = t | (1 << j);
				t = t & (~shift); 	
			}
		}	
		buffer_append_uchar(output, t);
	}
	output->length = s1->length;
}

int match_at(buffer_t *s, buffer_t *s1, buffer_t *pos){
	if(s->length != s1->length || s->length != pos->length){
		perror("Input buffers should have the same lengths\n");
		return 0;
	}
	
/* to be filled in */
	int result = 1;
	uchar *pt_s = s->tab;
	uchar *pt_s1 = s1->tab;
	uchar *pt_pos = pos->tab;
	for (int i=0; i<s1->length; i++){
		for (int j=0; j<8; j++){
			if(getBit(pt_pos[i],j) == 1){
				if(getBit(pt_s[i],j) != getBit(pt_s1[i],j))
					return 0;// If not, immediately return 0
			}
		}	
	}
	return result;
}


void search_with_match(buffer_t *IV_candidate, buffer_t *stream,
					   buffer_t *trans, buffer_t *pos){
	buffer_t stream_candidate;
	int i;
	
	buffer_init(&stream_candidate, stream->length);
	buffer_reset(IV_candidate);
	for(i = 0; i < trans->length; i++)
		buffer_append_uchar(IV_candidate, 0);
	
		
/* to be filled in */
	while(1){
		LFSR(&stream_candidate, trans, IV_candidate, stream->length);
		if (match_at(&stream_candidate, stream, pos))
			break;
		increment_buffer(IV_candidate);
	}

	buffer_clear(&stream_candidate);
}


void attack(buffer_t *IV_candidate1, buffer_t *IV_candidate2,
			   buffer_t *IV_candidate3, buffer_t *stream,
			   buffer_t *trans1, buffer_t *trans2, buffer_t *trans3,
			   double threshold){
/* to be filled in */

	buffer_t stream_candidate1, stream_position2, stream_candidate3;
	buffer_init(&stream_candidate1, stream->length);
	buffer_init(&stream_position2, stream->length);
	buffer_init(&stream_candidate3, stream->length);

	stream_candidate1.length = stream->length;
	stream_position2.length = stream->length;
	stream_candidate3.length = stream->length;

	searchIV(IV_candidate1, stream, trans1, threshold);
	searchIV(IV_candidate3, stream, trans3, threshold);

	LFSR(&stream_candidate1, trans1, IV_candidate1, stream->length);
	LFSR(&stream_candidate3, trans3, IV_candidate3, stream->length);

	positions(&stream_position2, &stream_candidate1, &stream_candidate3);

	search_with_match(IV_candidate2, stream, trans2, &stream_position2);
}
