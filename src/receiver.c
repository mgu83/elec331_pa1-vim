#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

#include <pthread.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/select.h>

#define MAXBYTES 16

void rrecv(unsigned short int myUDPport, 
            char* destinationFile, 
            unsigned long long int writeRate) {
    
    int len = 0;
    int sock;
    struct sockaddr_in addrrec, cliaddr; 
    char buffer[MAXBYTES];
    int ready;
    int listen_flag = 1;
    socklen_t slen = sizeof(cliaddr);
    int n;



    if((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0){
        perror("Failed to open receiver socket.. Exiting..");
        exit(EXIT_FAILURE);
    }
    memset((char *) &addrrec, 0, sizeof(addrrec));
    addrrec.sin_family = AF_INET;
    addrrec.sin_addr.s_addr = INADDR_ANY;
    addrrec.sin_port = htons(8080);

    if ( bind(sock, (struct sockaddr *)&addrrec,
			sizeof(addrrec)) < 0 )
	{
		perror("Failed to bind to socket...");
		exit(EXIT_FAILURE);
	}

    for(int i = 0; i< 4; i++){
        len = recvfrom(sock, buffer, sizeof(buffer)- 1, 0, NULL, 0);
        if(len < 0){
            perror("recvfrom failed");
            break;
        }
        buffer[len] = '\0';
        printf( "%d bytes: '%s'\n", len, buffer );
    }
    
    close(sock);
    return;
    

}

int main(int argc, char** argv) {
    // This is a skeleton of a main function.
    // You should implement this function more completely
    // so that one can invoke the file transfer from the
    // command line.

    unsigned short int udpPort;
    int sockrec_fd;
    char * dest = NULL;
    unsigned long long int rate;

    
    /*if (argc != 3) {
        fprintf(stderr, "usage: %s UDP_port filename_to_write\n\n", argv[0]);
        exit(1);
    }

    udpPort = (unsigned short int) atoi(argv[1]);*/
    
    //rrecv(udpPort, dest, rate);
    rrecv(0, 0, 0);
    

}
