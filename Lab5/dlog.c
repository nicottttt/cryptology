/****************************************************************/
/* dlog.c                                                       */
/* Authors: Alain Couvreur, FMorain                             */
/* alain.couvreur@lix.polytechnique.fr                          */
/* Last modification October 16, 2023                           */
/****************************************************************/

#include <stdio.h>
#include <assert.h>

#include "utilities.h"

#include "hash.h"
#include "dlog.h"
#include "gmp.h"

/* 2 is very verbose, 1 is mild */
#define DEBUG 0

/* INPUT: u <= sqrt(ord(g))
   OUTPUT: DLOG_ERROR in case of problem
           DLOG_SMALL_ORDER if order of g found
           DL_OK otherwise 
   SIDE-EFFECT: result <- ord(g) if small and DLOG_SMALL_ORDER is returned.
*/

#if !DEBUG
int babySteps(mpz_t result, hash_table H, mpz_t g, mpz_t u, mpz_t p){
    int res = DLOG_OK;
/* to be filled in */

    int addr = 0;
    mpz_t tmp1, tmp2;
    mpz_init(tmp1);
    mpz_init_set_ui(tmp2, 0);
    for(int i=0; mpz_cmp_ui(u, i) > 0; i++){ // u here is the sqrt(g)
        mpz_set_ui(tmp1, i);
        mpz_powm(tmp2, g, tmp1, p);
        hash_put_mpz(H, &addr, tmp2, tmp1, g, p);
    }
    mpz_set_ui(result, res);

    mpz_clears(tmp1, tmp2, NULL);
    return res;
}



/* OUTPUT: 0 in case of problem, 1 otherwise */
int giantSteps(mpz_t result, hash_table H, mpz_t g, mpz_t ordg, mpz_t u, mpz_t p, mpz_t a)
{
    int res = DLOG_ERROR;
/* to be filled in */

    mpz_t tmp1, tmp2, tmp3, tmp4, tmp5, tmp6;
    mpz_inits(tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, NULL);

    mpz_set(tmp1, u);
    mpz_pow_ui(tmp2, tmp1, 2); // n=u^2
    mpz_sub_ui(tmp1, tmp1, 1);// u-1
    mpz_cdiv_q(tmp3, tmp2, tmp1); // c=n/(u-1)

    mpz_pow_ui(tmp4, g, mpz_get_ui(u));// g^u
    mpz_invert(tmp5, tmp4, p);


    for(int c=0; mpz_cmp_ui(tmp3, c) > 0; c++){
        mpz_pow_ui(tmp6, tmp5, c);
        mpz_mul(tmp1, a, tmp6);
        mpz_mod(tmp2, tmp1, p);
        if(hash_get_mpz(result, H, tmp2, g, p) == HASH_FOUND){
            mpz_mul_ui(tmp4, u, c);//tmp4 = u*c
            mpz_add(result, result, tmp4);//d+c*u
            res = DLOG_OK;
            break;
        }
    }

    mpz_clears(tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, NULL);

    return res;
}



/* INPUT: ordg is an upper bound on ord(g).
   OUTPUT: DLOG_ERROR in case of pb
   SIDE-EFFECT: result = ord(g) if small and DLOG_SMALL_ORDER is returned.
 */
int BSGS_aux(mpz_t result, mpz_t a, mpz_t g, mpz_t ordg, mpz_t p)
{
    int res = DLOG_OK;
/* to be filled in */

    mpz_t u;
    mpz_init(u);
    mpz_sqrt(u, ordg);

    hash_table H = hash_init(mpz_get_ui(u) + 1);

    if (babySteps(result, H, g, u, p) != DLOG_OK)
        res =  DLOG_ERROR;

    if (giantSteps(result, H, g, ordg, u, p, a) == DLOG_ERROR){
        mpz_set(result, ordg);
        res = DLOG_SMALL_ORDER;
    }   
        
    mpz_clear(u);
    hash_clear(H);

    return res;
}

int BSGS(mpz_t result, mpz_t a, mpz_t g, mpz_t p)
{
    if (mpz_cmp_ui(a, 1) == 0) {
        mpz_set_ui(result, 0);
        return DLOG_OK;
    }

    mpz_t ordg;
    mpz_init(ordg);
    mpz_sub_ui(ordg, p, 1);

    int res = BSGS_aux(result, a, g, ordg, p);
    printf("-------OK-----\n");

    if (res == DLOG_SMALL_ORDER) {
        res = DLOG_ERROR;
    }else if(res == DLOG_OK){
        res = DLOG_FOUND;
    }

    mpz_clear(ordg);
    return res;
}
#endif


#if DEBUG
int babySteps(mpz_t result, hash_table H, mpz_t u, mpz_t g, mpz_t p){
    int res = 1;
    int addr = 0;
    //int count = 0;
/* to be filled in */
    mpz_t temp1, temp2;
    mpz_init_set_ui(temp1, 0);
    mpz_init_set_ui(temp2, 0);
    for(; mpz_cmp(temp1, u)<0; mpz_add_ui(temp1, temp1, 1)){
        // mpz_pow_ui(temp2, g, mpz_get_ui(temp1)); //temp2 = g^temp1 mod p
        mpz_powm(temp2, g, temp1, p); //temp2 = g^temp1 mod p
        hash_put_mpz(H, &addr, temp2, temp1, g, p);
    }
    // for(count; count<mpz_get_ui(u); count++){
    //     mpz_set_ui(temp1, count);
    //     mpz_powm(temp2, g, temp1, p); //temp2 = g^temp1 mod p
    //     hash_put_mpz(H, &addr, temp2, temp1, g, p);
    // }
    return res;
}


int giantSteps(mpz_t result, hash_table H, mpz_t u, mpz_t g,
	       mpz_t p, mpz_t a){
    int res = 0;
/* to be filled in */
    mpz_t temp, temp1, temp2, temp3, temp4, g_u1, g_u2, g_uc, c;
    mpz_inits(temp, temp1, temp2, temp3, temp4, g_u1, g_u2, g_uc, NULL);
    mpz_init_set_ui(c, 0);
    mpz_set(temp, u);//temp = u
    mpz_pow_ui(temp1, temp, 2);//temp1 = u^2 = n
    mpz_sub_ui(temp2, temp1, 1);//temp2 = n-1
    mpz_cdiv_q(temp3, temp2, temp);//temp3 = (n-1)/u
    mpz_pow_ui(g_u1, g, mpz_get_ui(u));
    mpz_invert(g_u2, g_u1, p);
    for(; mpz_cmp(c, temp3); mpz_add_ui(c, c, 1)){
        if(mpz_cmp_ui(c,0) == 0){
            mpz_set_ui(g_uc, 1);
        }else{
            mpz_mul(g_uc, g_uc, g_u2);
        }
        mpz_mul(temp2, a, g_uc);//temp1 = a*temp
        mpz_mod(temp4, temp2, p); //times a and then mod p again
        if(hash_get_mpz(result, H, temp4, g, p) == HASH_FOUND){
            mpz_mul(temp, u, c);//temp = u*c
            mpz_add(result, result, temp);//d+c*u
            res = 1;
            break;
        }
    }
    mpz_clears(temp, temp1, temp2, temp3, temp4, c, NULL);
    return res;
}

int BSGS(mpz_t result, mpz_t a, mpz_t g, mpz_t p)
{
    int res = 0;
/* to be filled in */
    //gmp_printf("a: %Zd g: %Zd p: %Zd", a, g, p);
    mpz_t u;
    mpz_inits(u, NULL);
    mpz_sqrt(u, p);//n = (p)^0.5
    unsigned int size = mpz_get_ui(u);
    hash_table H = hash_init(size);	
    if(babySteps(result, H, u, g, p))
        if(giantSteps(result, H, u, g, p, a))
            res = 1;
    mpz_clears(u, NULL);
    return res;
}

#endif

