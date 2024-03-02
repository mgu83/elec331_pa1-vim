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
#include <netdb.h>
#include <sys/ioctl.h>

#define MSG_CONFIRM 0

void rsend(char* hostname, 
            unsigned short int hostUDPport, 
            char* filename, 
            unsigned long long int bytesToTransfer) 
{   
    int sock;
    struct sockaddr_in addrrec; 
    char *hello = "ABC";
    socklen_t cliaddrlen;

    if((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0){
        perror("Failed to open receiver socket.. Exiting..");
        exit(EXIT_FAILURE);
    }

    memset((char *) &addrrec, 0, sizeof(addrrec));
    addrrec.sin_family = AF_INET;
	addrrec.sin_port = htons(hostUDPport);
    addrrec.sin_addr.s_addr = inet_addr(hostname);
	
    cliaddrlen = sizeof(addrrec);
    //for(int i = 0; i < 4; i++){
        if(sendto(sock, hello, 3, 0, (struct sockaddr *) &addrrec, sizeof(addrrec)) < 0){
            perror("Error sending bytes\n");
            //break;
        }
        printf("message sent \n");
    //}

    printf("Bytes sent: %s", hello);
    close(sock);
	return;


}

int main(int argc, char** argv) {
    // This is a skeleton of a main function.
    // You should implement this function more completely
    // so that one can invoke the file transfer from the
    // command line.
    int hostUDPport;
    unsigned long long int bytesToTransfer;
    char* hostname = NULL;
    int socksen_fd;
    char* filename = NULL;
    

    if (argc != 5) {
        fprintf(stderr, "usage: %s receiver_hostname receiver_port filename_to_xfer bytes_to_xfer\n\n", argv[0]);
        exit(1);
    }
    hostUDPport = (unsigned short int) atoi(argv[2]);
    hostname = argv[1];
    bytesToTransfer = atoll(argv[4]);

    rsend(hostname, hostUDPport, filename, bytesToTransfer); 
    //rsend("hostname", 0, "filename", 0); 
    return (EXIT_SUCCESS);
}