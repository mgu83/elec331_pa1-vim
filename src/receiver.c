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

#define MAXBYTES 1024

void rrecv(unsigned short int myUDPport, 
            char* destinationFile, 
            unsigned long long int writeRate) {
    
    int len = 0;
    int sock;
    struct sockaddr_in addrrec, cliaddr; 
    char buffer[MAXBYTES];
    int ready;
    int listen_flag = 1;
    //struct sockaddr_storage addr;
    socklen_t slen = sizeof(cliaddr);


    if((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0){
        perror("Failed to open receiver socket.. Exiting..");
        exit(EXIT_FAILURE);
    }
    memset((char *) &addrrec, 0, sizeof(addrrec));
    addrrec.sin_family = AF_INET;
    addrrec.sin_addr.s_addr = INADDR_ANY;
    addrrec.sin_port = htons(myUDPport);

    if ( bind(sock, (const struct sockaddr *)&addrrec,
			sizeof(addrrec)) < 0 )
	{
		perror("Failed to bind to socket...");
		exit(EXIT_FAILURE);
	}
    
    len = recvfrom(sock, buffer, len, 0, &cliaddr, &slen);
    /*while(listen_flag){
        struct timeval timeout;
        timeout.tv_sec = 1;       //timeout (secs.)
        timeout.tv_usec = 0;
        fd_set socketset;
        FD_SET(sock+1, &socketset);//tcp socket
        FD_SET(STDIN_FILENO, &socketset);

        ready = select(2, &socketset, NULL, NULL, &timeout);
                
        if(ready > 0){
            ioctl(sock, FIONREAD, &len);
            len = read(sock, &buffer, MAXBYTES);
            printf("bytesRead %i : %s", len, buffer);
            //len = recvfrom(sock, buffer, len, 0, &cliaddr, &slen);
            listen_flag = 0;
        }
    }*/

    /*while(len == 0){
        ioctl(sock, FIONREAD, &len);
    
        if(len > 1){
            printf("# bytes received: %d", len);
            len = read(sock, buffer, len);
            //break;
        }
    }*/
    printf("Bytes received: %s", buffer);
    //close(sock);
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

    printf("inside main");
    if (argc != 3) {
        fprintf(stderr, "usage: %s UDP_port filename_to_write\n\n", argv[0]);
        exit(1);
    }

    udpPort = (unsigned short int) atoi(argv[1]);
    
    rrecv(udpPort, dest, rate);
    

}
