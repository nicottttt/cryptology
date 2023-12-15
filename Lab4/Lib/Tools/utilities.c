#include <stdio.h>
#include <stdlib.h>

#include "utilities.h"

/* What a trick! */
void implementation_check(const char *fctname, int n){
    if(n == -42){
	printf("\nWARNING: presumably, the function \"%s\" is"
	       " not programmed yet; exiting.\n", fctname);
	exit(-1);
    }
}

