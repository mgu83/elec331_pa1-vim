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


void rrecv(unsigned short int myUDPport, 
            char* destinationFile, 
            unsigned long long int writeRate, int socket) {
    
    int len = 0;


    while(len == 0){
        ioctl(sock, FIONREAD, &len);
    }
    printf("%d bytes received", len);
    len = 


    

}

int main(int argc, char** argv) {
    // This is a skeleton of a main function.
    // You should implement this function more completely
    // so that one can invoke the file transfer from the
    // command line.

    unsigned short int udpPort;
    int sockrec_fd;
    struct sockaddr_in addrrec; 


    if (argc != 3) {
        fprintf(stderr, "usage: %s UDP_port filename_to_write\n\n", argv[0]);
        exit(1);
    }

    udpPort = (unsigned short int) atoi(argv[1]);

    if(sockrec_fd = socket(AF_INET, SOCK_DGRAM, 0) < 0){
        perror("Failed to open receiver socket.. Exiting..");
        exit(EXIT_FAILURE);
    }

    addrrec.sin_family = AF_INET;
    addrrec.sin_addr.s_addr = INADDR_ANY;
    addrrec.sin_port = htons(udpPort);

    if ( bind(sockrec_fd, (const struct sockaddr *)&addrrec,
			sizeof(servaddr)) < 0 )
	{
		perror("Failed to bind to socket...");
		exit(EXIT_FAILURE);
	}



}
