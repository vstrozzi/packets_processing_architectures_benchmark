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

#include "../helpers/helpers.h"

// Size of payload chunks (BYTE) --> Needs to divide size of file
#define SIZE TOT_SIZE_B
#define BUFLEN 2*sizeof(clock_t)
#define NS_PER_SEC 1000000000

// Global vars
const char F_READ[] = "../helpers/To_test.txt";
FILE *rfptr;
const char F_WRITE[] = "zuc_ben.txt";
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

/* This function will print the time at which a packet is received at the XDP interface
 */
static void poll_stats(){
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
	
     // Open file and read it as byte
    if((rfptr = fopen(F_READ, "rb")) == NULL){
        printf("Error readingfile \n");
		exit(1);
    }

	// Keep sending data to test
    struct timespec send, rec;
    for(int i = 0; i < chunks; i++){
		//printf("Cleaning");
		fflush(stdout);
        // Send data
        fread(fdata, sizeof(fdata), 1, rfptr);
        sendto(pid, fdata, sizeof(fdata), 0, (struct sockaddr *) &sock_to, sock_len);
	}
    fclose(rfptr);

    // Write data for benchmark
    if ((wfptr = fopen(F_WRITE, "a")) == NULL){
        printf("FILE not found\n");
        exit(1);
    }

    // Keep listening for data
    int count = 0;
	while(1){
		//printf("Waiting for data...\n");
		fflush(stdout);
		// Try to receive some data (blocking)
		if((pay_len = recvfrom(pid, buf, BUFLEN, 0, (struct sockaddr *) &sock_from, &sock_len)) == -1){
			printf("Error from receiving\n");
		    exit(1);
		}
        count++;
        // Save time of receival (important: use same clock_id as xdp kern)
        struct timespec rec;
        clock_gettime(CLOCK_MONOTONIC, &rec);
        unsigned long long usr_time_ns = (unsigned long long)rec.tv_nsec + ((unsigned long long)rec.tv_sec)*NS_PER_SEC;
		//printf("Received packet from %s:%d\n", inet_ntoa(sock_from.sin_addr), ntohs(sock_from.sin_port));
        // Read time from boot if possible (long long )
        if(pay_len >= BUFLEN){
            // Print and write time of receival in different formats
            //printf("Time of XDP_kern receival: %llu ns\n" , *((unsigned long long*)buf));
            //printf("Time of XDP_kern processing: %llu ns\n" , *((unsigned long long*)buf + 1));
            //printf("Tot time of XDP_kern processing: %llu ns \n", *((unsigned long long*)buf + 1) - *((unsigned long long*)buf));
            //printf("Time of user_space receival: %llu ns\n" , usr_time_ns);
            //printf("Difference of time in ns from receival: %llu ns \n", usr_time_ns - *((unsigned long long*)buf));
            fprintf(wfptr, "%llu\n", *((unsigned long long*)buf + 1) - *((unsigned long long*)buf));
        }
	}

    fclose(wfptr);
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