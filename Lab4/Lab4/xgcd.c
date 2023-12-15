#include <stdio.h>
#include <assert.h>

#include "gmp.h"

#include "xgcd.h"

#define DEBUG 0


/* to be filled in */

long exgcd(long *u, long *v, long a, long b){

    if(b==0)
    {            
        *u = (long)1;
        *v = (long)0;
        return a;
    }
    long gcd = exgcd(u, v, b, a%b);
    long t = *v;
    *v = *u - (a/b)*t;   
    *u = t;
    return gcd;
}

/* compute g, u and v s.t. a*u+b*v = g = gcd(a, b) */
int XGCD_long(long *g, long *u, long *v, long a, long b){
    int status = 1;

    *g = exgcd(u, v, a, b);

/* to be filled in */
    return status;
}


/* compute g, u and v s.t. a*u+b*v = g = gcd(a, b) */
int XGCD(mpz_t g, mpz_t u, mpz_t v, mpz_t a, mpz_t b){
    int status = 1;

    // mpz_t tmp, tmp2, tmp3, tmp4;
    // mpz_init(tmp);
    // mpz_init(tmp2);
    // mpz_init(tmp3);
    // mpz_init(tmp4);

    // if(mpz_cmp_ui(b, 0) == 0){
    //     // mpz_set_ui(u, 1);
    //     // mpz_set_ui(v, 0);
    //     gmp_printf("----------a: %Zd\n", a);
    //     mpz_set(g, a);
    //     return status;
    // }


    // mpz_cdiv_r(tmp, a, b);// a%b
    // gmp_printf("----------tmp: %Zd\n", tmp);

    // XGCD(g, u, v, b, tmp);
    
    
    // // mpz_set(tmp3, v); // t = *v;
    // // mpz_cdiv_q(tmp4, a, b);// a/b
    // // mpz_mul(tmp, tmp4, v);// (a/b)*(*v)
    // // mpz_sub(tmp2, u, tmp);// *u - (a/b)*(*v)

    // // mpz_set(v, tmp2);
    // // mpz_set(u, tmp3);

    // mpz_clears(tmp, tmp2, tmp3, tmp4, NULL);

    //----------------------------------------
    // Try to use the above method, but the return value's sign flag sometimes reverse??
    // Like the first one: g should be 1 but it shows to be -1. the same as the forth one, so I change to the following one
    //----------------------------------------
    mpz_t tmp, tmp2, tmp3, tmp4, tmp5;
    mpz_init(tmp);
    mpz_init(tmp2);
    mpz_init(tmp3);
    mpz_init(tmp4);
    mpz_init(tmp5);

    if(mpz_cmp_ui(a, 0) == 0){
        mpz_set_ui(u, 0);
        mpz_set_ui(v, 1);
        mpz_set(g, b);
        return status;
    }

    mpz_cdiv_r(tmp4, b, a);
    //gmp_printf("----------tmp: %Zd\n", tmp4);

    XGCD(g, u, v, tmp4, a);

    mpz_set(tmp5, u);
    mpz_cdiv_q(tmp3, b, a);
    mpz_mul(tmp2, u, tmp3);
    mpz_sub(tmp, v, tmp2);
    mpz_set(u, tmp);
    mpz_set(v, tmp5);

    mpz_clears(tmp, tmp2, tmp3, tmp4, tmp5, NULL);

/* to be filled in */
    return status;
}


/* Solve a*x=b mod m if possible. 
   
 */
int linear_equation_mod(mpz_t x, mpz_t a, mpz_t b, mpz_t m){
    int status = 0;
    mpz_t tmp, tmp2, tmp3, tmp4;
    mpz_init(tmp);
    mpz_init(tmp2);
    mpz_init(tmp3);
    mpz_init(tmp4);

    mpz_init_set_ui(x, 0);

    for (int i=0; i<500; i++){
        mpz_add_ui(tmp, x, i);
        mpz_mul(tmp2, a, tmp);
        mpz_mod(tmp3, tmp2, m);
        mpz_mod(tmp4, b, m);
        if(mpz_cmp(tmp3, tmp4) == 0){
            mpz_set(x, tmp);
            status = 1;
            break;
        }
    }

    mpz_clears(tmp, tmp2, tmp3, tmp4, NULL);

/* to be filled in */
    return status;
}
 
