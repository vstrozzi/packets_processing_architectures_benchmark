#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>  // Get time functions
#include <string.h>
#include <linux/if_link.h>
#include <signal.h>
#include <net/if.h>
#include <assert.h>
#include <arpa/inet.h>
#include <sys/socket.h> // Socket library

#include "../../../helpers/helpers.h"

// Size of payload chunks (BYTE) --> Needs to divide size of file
#define SIZE TOT_SIZE_B
#define BUFLEN 2*sizeof(clock_t)
#define NS_PER_SEC 1000000000

// Global vars
const char F_READ[] = "../../../helpers/To_test.txt";
FILE *rfptr;
const char F_WRITE[] = "fw_back_ov_ben.txt";
FILE *wfptr;

int pid;
// Size of file 
int size = MiB;
// Number of chunks
int chunks = MiB/SIZE;


/* This function will remove the socket */
static void int_exit(int sig){
    printf("Closing socket");
    close(pid);
    fclose(rfptr);
    fclose(wfptr);
    exit(0);
}

/* This function gather data*/
static void poll_stats(){
    // Netw data
    struct sockaddr_in sock_to, sock_from;
    int sock_len = sizeof(sock_to);
    int pay_len;
	char buf[BUFLEN];
    char fdata[SIZE];
	
	// Create a UDP socket
	if ((pid=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1){
        printf("Error creating socket\n");
		exit(1);
	}
	
	// Zero out the structure
	memset((char *) &sock_to, 0, sizeof(sock_to));
	
    // Set our server socket parameters (listen to any msg)
	sock_from.sin_family = AF_INET;
	sock_from.sin_port = htons(PORT);
	sock_from.sin_addr.s_addr = htonl(INADDR_ANY);

    // Set our socket to send to
	sock_to.sin_family = AF_INET;
	sock_to.sin_port = htons(PORT);
	sock_to.sin_addr.s_addr = inet_addr(IP);

	// Bind socket to port
	if(bind(pid, (struct sockaddr*)&sock_from, sizeof(sock_from)) == -1){
		printf("Error binding to port\n");
		exit(1);
	}

    // Repeat same tes NR_REP time
	for (int j = 0; j < NR_REP; j++){
        // Open file and read it as byte
        if((rfptr = fopen(F_READ, "rb")) == NULL){
            printf("Error readingfile \n");
            exit(1);
        }

        // Keep sending data to test
        unsigned long long time_diff[chunks];
        struct timespec send, rec;
        for(int i = 0; i < chunks; i++){
            //printf("Operating on chunk %d\n", i);
            fflush(stdout);
            // Send data
            fread(fdata, sizeof(fdata), 1, rfptr);
            // Send time
            clock_gettime(CLOCK_MONOTONIC, &send);
            sendto(pid, fdata, sizeof(fdata), 0, (struct sockaddr *) &sock_to, sock_len);
            // Try to receive some data (blocking)
            if((pay_len = recvfrom(pid, buf, BUFLEN, 0, (struct sockaddr *) &sock_from, &sock_len)) == -1){
                printf("Error from receiving\n");
                exit(1);
            }
            // Rec time
            clock_gettime(CLOCK_MONOTONIC, &rec);
            unsigned long long usr_time_send = (unsigned long long)send.tv_nsec + ((unsigned long long)send.tv_sec)*NS_PER_SEC;
            unsigned long long usr_time_rec = (unsigned long long)rec.tv_nsec + ((unsigned long long)rec.tv_sec)*NS_PER_SEC;
            unsigned long long usr_time_diff = usr_time_rec - usr_time_send;
            time_diff[i] = usr_time_diff;
        }
        fclose(rfptr);

        // Write data for benchmark
        if ((wfptr = fopen(F_WRITE, "a")) == NULL){
            printf("FILE not found\n");
            exit(1);
        }
        // Write previously gathered data to file
        for(int i = 0; i < chunks; i++){
            fprintf(wfptr, "%llu\n", time_diff[i]);
        }

        fclose(wfptr);
    }
	close(pid);
	return;
    
}

// Simple polling
int main(int argc, char *argv[]){

    // Check correct size
    if (size % SIZE != 0){
        printf("Size and size file not divisor\n");
        exit(1);
    }

     // Set signals to remove from interface when killed 
    signal(SIGINT, int_exit);
    signal(SIGTERM, int_exit);

    // Poll stats
    poll_stats();

    return 0;
}