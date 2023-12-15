#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <getopt.h>
#include <unistd.h>

#include "gmp.h"

#include "utilities.h"

#include "base64.h"
#include "buffer.h"
#include "bits.h"
#include "random.h"

#include "operating_modes.h"
#include "aes.h"

#include "version.h"

#include "network.h"
#include "dh.h"
#include "channel.h"

#include "certificate.h"
#include "client.h"

/* to be filled in */

#ifdef CORRECTION
#define DEBUG 0
#else
#define DEBUG 2
#endif

static char *client_host;
static int client_port;
static char *user_name;

void set_client_host(const char *ch){
    client_host = malloc(sizeof(char)*strlen(ch)+1);
    strcpy(client_host, ch);
}

void free_client_host(){
    free(client_host);
}

void set_user_name(const char *ch){
    user_name = malloc(sizeof(char)*strlen(ch)+1);
    strcpy(user_name, ch);
}

void free_user_name(){
    free(user_name);
}

void set_client_port(int p){
    client_port = p;
}

int get_client_port(int p){
    return client_port;
}

void handle_reply(char **from, int *portfrom, char **reply, char **packet){
    *packet = network_recv(1);
    if(!parse_packet(from, portfrom, reply, *packet)){
        return;
    };
    printf("Received \"%s\" from %s:%d!\n", *reply, *from, *portfrom);
}

void try_send(const char *host, const int port){
    char *hello = malloc(sizeof(char)*(40+strlen(user_name)+strlen(client_host)+1));
    sprintf(hello, "Hello! My name is %s, calling from %s.", user_name, client_host);
    network_send(host, port, client_host, client_port, hello);
    free(hello);
    char *packet = network_recv(1);
    if (packet == NULL) {
        printf("[ERROR] %s didn't reply to me (%s)\n", host, client_host);
        return;
    }
    free(packet); // discard first reply
    network_send(host, port, client_host, client_port, "I am the client sending to the server.\n");
}

void try_aes(){
    uchar *msg = (uchar*)"It's a long way to Tipperary";
    buffer_t clear, encrypted, key, IV, decrypted;
    mpz_t gab;

    buffer_init(&clear, strlen((char*)msg));
    buffer_init(&encrypted, 1);
    buffer_init(&key, BLOCK_LENGTH);
    buffer_init(&IV, BLOCK_LENGTH);
	
    mpz_init_set_str(gab, "12345612345678907890", 10);
    AES128_key_from_number(&key, gab);
    buffer_random(&IV, BLOCK_LENGTH);
    buffer_from_string(&clear, msg, strlen((char*)msg));

    aes_CBC_encrypt(&encrypted, &clear, &key, &IV, 's');

    buffer_init(&decrypted, 1);
    aes_CBC_decrypt(&decrypted, &encrypted, &key, 's');
    buffer_print(stdout, &decrypted);
    printf("\n");
	
    buffer_clear(&clear);
    buffer_clear(&encrypted);
    buffer_clear(&decrypted);
    buffer_clear(&key);
    buffer_clear(&IV);
    mpz_clear(gab);
}

int send_with_aes(const char *host, const int port, uchar *msg, mpz_t gab){
    int status = 1;
/* to be filled in */

    printf("Sending: AES\n");
    network_send(host, port, client_host, client_port, "AES");

    // Init
    buffer_t clear, encrypted, key, IV, send_buf;
    buffer_init(&clear, strlen((char*)msg));
    buffer_init(&encrypted, 1);
    buffer_init(&key, BLOCK_LENGTH);
    buffer_init(&IV, BLOCK_LENGTH);
    buffer_init(&send_buf, 128);

    // AES
    AES128_key_from_number(&key, gab);
    buffer_random(&IV, BLOCK_LENGTH);
    buffer_from_string(&clear, msg, strlen((char*)msg));

    aes_CBC_encrypt(&encrypted, &clear, &key, &IV, 's');

    // Send to server
    buffer_to_base64(&send_buf, &encrypted);

    char *send_msg = (char *)string_from_buffer(&send_buf);
    printf("Sending: %s\n", send_msg);
    network_send(host, port, client_host, client_port, send_msg);
    free(send_msg);


    // Clear
    buffer_clear(&clear);
    buffer_clear(&encrypted);
    buffer_clear(&key);
    buffer_clear(&IV);
    buffer_clear(&send_buf);
    
    

    return status;
}

void try_send_aes(const char *host, const int port){
    uchar *msg = (uchar*)"It's a long way to Tipperary";
    mpz_t gab;

    mpz_init_set_str(gab, "12345612345678907890", 10);
    int status = send_with_aes(host, port, msg, gab);
    implementation_check("send_with_aes", status);
    mpz_clear(gab);
}

void prepare_cipher(buffer_t *encrypted, buffer_t *clear, buffer_t *key){
    buffer_t IV;
    buffer_init(&IV, BLOCK_LENGTH);
    buffer_random(&IV, BLOCK_LENGTH);
    aes_CBC_encrypt(encrypted, clear, key, &IV, 's');
    buffer_clear(&IV);
}


void CaseDH(const char *server_host, const int server_port, gmp_randstate_t state){
/* to be filled in */

    // Init
    mpz_t g, a, p, ga, gb, k;
    mpz_inits(g, a, p, ga, gb, k, NULL);
    channel_init(p, g);
    int nbits = 4;
    DH_init(a, state, nbits);

    
    char buf[1024];
    char *rcv_pkt, *rcv_msg;

    uchar *msg = (uchar*)"Hello from Alice";
    
    // step 1: Alice sends ga to Bob
    mpz_powm(ga, g, a, p);// ga = g^a mod p;
    msg_export_mpz(buf, "DH: ALICE/BOB CONNECT1 ", ga, 0);
    network_send(server_host, server_port, client_host, client_port, buf);

#if DEBUG
    printf("Step1 finish\n");
#endif

    // step 2: Bob sends gb to Alice
    rcv_pkt = network_recv(10);

    if(rcv_pkt != NULL)
        parse_packet(NULL, NULL, &rcv_msg, rcv_pkt);
    
    msg_import_mpz(gb, rcv_msg, "DH: BOB/ALICE CONNECT2 ", 0);// Get gb from Bob

#if DEBUG
    printf("Step2 finish\n");
#endif

    // Step 3: compute secret key, send encrypted msg
    mpz_powm(k, gb, a, p);// k = (g^b)^a mod p;
        
    buffer_t clear, encrypted, key, IV, send_buf;
    buffer_init(&clear, strlen((char*)msg));
    buffer_init(&encrypted, 1);
    buffer_init(&key, BLOCK_LENGTH);
    buffer_init(&IV, BLOCK_LENGTH);
    buffer_init(&send_buf, 128);

    // AES
    AES128_key_from_number(&key, k);
    buffer_random(&IV, BLOCK_LENGTH);
    buffer_from_string(&clear, msg, strlen((char*)msg));

    aes_CBC_encrypt(&encrypted, &clear, &key, &IV, 's');

    buffer_to_base64(&send_buf, &encrypted);

    char *send_msg = (char *)string_from_buffer(&send_buf);
    msg_export_string(buf, "DH: ALICE/BOB CONNECT3 ", send_msg);
    network_send(server_host, server_port, client_host, client_port, buf);
    free(send_msg);

#if DEBUG
    printf("Step3 finish\n");
#endif


    // Free
    buffer_clear(&clear);
    buffer_clear(&encrypted);
    buffer_clear(&key);
    buffer_clear(&IV);
    buffer_clear(&send_buf);
    free(rcv_msg);
    free(rcv_pkt);

    mpz_clears(g, a, p, ga, gb, k, NULL);
    return;
}

int CaseSTS(const char *server_host, const int server_port,
	    certificate_t *CA, mpz_t NA, mpz_t dA, mpz_t N_aut, mpz_t e_aut,
	    gmp_randstate_t state){
    int retno=0;
/* to be filled in */

    // Init
    mpz_t g, a, p, n, gb, k, sigmaB, tmp, sigmaA;
    mpz_inits(g, a, p, n, gb, k, sigmaB, tmp, sigmaA, NULL);
    char buf[1024];
    char *rcv_pkt, *rcv_msg, *ctmp;
    size_t nbits;

    buffer_t in, y, key, decrypted, IV, encrypted, clear, out;
    buffer_init(&in, 1);
    buffer_init(&y, 1);
    buffer_init(&key, BLOCK_LENGTH);
    buffer_init(&decrypted, 1);
    buffer_init(&IV, BLOCK_LENGTH);
    buffer_random(&IV, BLOCK_LENGTH);
    buffer_init(&encrypted, 1);
    buffer_init(&clear, BLOCK_LENGTH);
    buffer_init(&out, BLOCK_LENGTH);

    certificate_t CB;


    // step 1: Alice sends n to Bob
    channel_init(p, g);
    nbits = mpz_sizeinbase(p, 2) - 1;
    DH_init(a, state, nbits);
    

    mpz_powm(n, g, a, p);// n = g^a mod p;
    msg_export_mpz(buf, "STS: ALICE/BOB CONNECT1 ", n, 0);
    network_send(server_host, server_port, client_host, client_port, buf);

#if DEBUG
    printf("Step 1 finish\n");
#endif

    // step 2: Bob sends gb to Alice, receive (y, n, CB) from Bob

    //receive y:
    rcv_pkt = network_recv(10);

    if(rcv_pkt != NULL)
        parse_packet(NULL, NULL, &rcv_msg, rcv_pkt);
    
    if(msg_import_string(buf, rcv_msg, "STS: BOB/ALICE CONNECT2 ")<=0){
        free(rcv_msg);
        free(rcv_pkt);
        goto free;
    }
        
    
    free(rcv_msg);
    free(rcv_pkt);


    buffer_from_string(&in, (uchar*)buf, strlen(buf));
    buffer_from_base64(&y, &in);

#if DEBUG
    printf("Receive y finish\n");
#endif

    //receive n(gb):
    rcv_pkt = network_recv(10);

    if(rcv_pkt != NULL)
        parse_packet(NULL, NULL, &rcv_msg, rcv_pkt);

    if(msg_import_mpz(gb, rcv_msg, "STS: BOB/ALICE CONNECT2 ", 0) <= 0){
        free(rcv_msg);
        free(rcv_pkt);
        goto free;
    }
    
    free(rcv_msg);
    free(rcv_pkt);

#if DEBUG
    printf("Receive n finish\n");
#endif

    //receive CB:
    rcv_pkt = network_recv(10);

    if(rcv_pkt != NULL)
        parse_packet(NULL, NULL, &rcv_msg, rcv_pkt);

    if(msg_import_string(buf, rcv_msg, "STS: BOB/ALICE CONNECT2 ") <= 0){
        free(rcv_msg);
        free(rcv_pkt);
        goto free;
    }
    
    free(rcv_msg);
    free(rcv_pkt);

#if DEBUG
    printf("Receive CB finish\n");
#endif

    init_certificate(&CB);
    certificate_from_string(&CB, buf);

#if DEBUG
    printf("Step 2 finish\n");
#endif

    // Step 3: compute secret key, send encrypted msg
    if (!valid_certificate(&CB, N_aut, e_aut))
        goto free;

    mpz_powm(k, gb, a, p);// k = (g^b)^a mod p;
    AES128_key_from_number(&key, k);

    aes_CBC_decrypt(&decrypted, &y, &key, 's'); // Decrypte

    buffer_to_mpz(sigmaB, &decrypted); //sigmaB = SIGN_{SK_B}(ga || gb)

    concatenate_gb_ga(tmp, gb, n, p);
    mpz_powm(sigmaB, sigmaB, CB.e, CB.N);

    if (mpz_cmp(tmp, sigmaB) != 0) 
	    goto free;

    SIGNSK(sigmaA, n, gb, p, NA, dA); //sigma_A <- SIGN_{SK_A}(gb, ga)
    buffer_from_mpz(&clear, sigmaA);
    aes_CBC_encrypt(&encrypted, &clear, &key, &IV, 's');

    // Send (Z,Ca):
    buffer_to_base64(&out, &encrypted);
    ctmp = (char*)string_from_buffer(&out);
    msg_export_string(buf, "STS: ALICE/BOB CONNECT3 ", ctmp);
    network_send(server_host, server_port, client_host, client_port, buf);
    free(ctmp);

    ctmp = (char*)string_from_certificate(CA);
    msg_export_string(buf, "STS: ALICE/BOB CONNECT3 ", ctmp);
    network_send(server_host, server_port, client_host, client_port, buf);
    free(ctmp);

#if DEBUG
    printf("Step 3 finish\n");
#endif

    // Step 4: receive OK:
    rcv_pkt = network_recv(10);
    parse_packet(NULL, NULL, &rcv_msg, rcv_pkt);

    if(msg_import_string(buf, rcv_msg, "STS: BOB/ALICE CONNECT4 ") <= 0){
        free(rcv_msg);
        free(rcv_pkt);
        goto free;
    }
    
    free(rcv_msg);
    free(rcv_pkt);

    buffer_from_string(&in, (uchar*)buf, strlen(buf));
    buffer_from_base64(&y, &in);

    aes_CBC_decrypt(&decrypted, &y, &key, 's');
    if(strcmp((const char*)decrypted.tab,"OK") == 0){

#if DEBUG
        printf("Receive OK\n");
#endif
        
        retno = 1;
    }

    printf("last message: %s\n", decrypted.tab);

#if DEBUG
    printf("Step 4 finish\n");
#endif

free:
    buffer_clear(&in);
    buffer_clear(&y);
    buffer_clear(&key);
    buffer_clear(&decrypted);
    buffer_clear(&IV);
    buffer_clear(&encrypted);
    buffer_clear(&clear);
    buffer_clear(&out);

    clear_certificate(&CB);

    // free(rcv_msg);
    // free(rcv_pkt);
    // free(ctmp);

    mpz_clears(g, a, p, n, gb, k, sigmaB, tmp, sigmaA, NULL);


    return retno;
}

void CaptureTheFlag(const char *server_host, const int server_port,
                    certificate_t *CA, mpz_t NA, mpz_t dA, mpz_t N_aut,
		    mpz_t e_aut, gmp_randstate_t state){
    // Ask to capture the flag
    char ctf[] = "CTF: CONNECT";
    network_send(server_host, server_port, client_host, client_port, ctf);
    network_send(server_host, server_port, client_host, client_port, user_name);
    int status = CaseSTS(server_host, server_port, CA, NA, dA, 
			 N_aut, e_aut, state);
    implementation_check("CaseSTS", status);
    if (status == 0) {
        printf("[CTF] ERROR, try again.\n");
        return;
    }
    char *packet = network_recv(5);
    char *from, *msg;
    parse_packet(&from, NULL, &msg, packet);
    if (strcmp(from, server_host) != 0) {
        printf("[CTF] You've been hacked by %s!\n", from);
        free(packet);
        free(from);
        free(msg);
        return;
    }
    mpz_t secret;
    mpz_init(secret);
    printf("Message = %s\n", msg);
    if (mpz_set_str(secret, msg, 16) == 0) {
        gmp_printf("[CTF] Congratulations ! You captured your flag!\nSecret=%#Zx\n", secret);
    }
    else {
        printf("[CTF] ERROR, try again.\n");
    }
    free(packet);
    free(from);
    free(msg);
    mpz_clear(secret);
}
