#include <stdio.h>
#include <stdlib.h>
#include "gmp.h"

void fermat(int a0, int pmin, int pmax, int composites){
/* to be filled in */
    mpz_t a, m, power_p, mod_p;
    int flag = 0; // No composites, default is 0
    mpz_init(a);
    mpz_init(m);
    mpz_init(power_p);
    mpz_init(mod_p);
    mpz_set_si(a, a0);

    for(int i=pmin; i<=pmax; i++){
        mpz_set_si(power_p, i-1);
        mpz_set_si(mod_p, i);
        mpz_powm(m, a, power_p, mod_p);

        // Only when composites == 1, I start judge if it is prime
        if(composites == 1){
            flag = mpz_probab_prime_p(mod_p, 10);
        }
            
        if (mpz_cmp_ui(m, 1) == 0 && flag == 0){
            printf("2 %d\n",i);
        }
    }
}

void Usage(char *cmd){
    fprintf(stderr, "Usage: %s a pmin pmax [0|1]\n", cmd);
}


int main (int argc, char *argv[]){
    if(argc < 4){
	Usage(argv[0]);
	return 0;
    }
    int composites = 0;
    if(argc == 5)
	composites = atoi(argv[4]);
    fermat(atoi(argv[1]), atoi(argv[2]), atoi(argv[3]), composites);
    return 0;
}
