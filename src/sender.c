#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <pthread.h>
#include <errno.h>
#include <math.h>

#include "param.h"
#include "queue.h"

#define MAX(a,b) ((a) > (b) ? (a) : (b))
#define MIN(a,b) ((a) < (b) ? (a) : (b))


#define BUFFER_SIZE 1024

struct sockaddr_in my_addr, other_addr;
int sockfd, slen;

Queue * backup_queue;
Queue * first_queue;

// Will probably need some other global variables 

void send_pkt(packet* pkt){
    printf("Sending packet");
    sendto(sockfd, pkt, sizeof(packet), 0, (struct sockaddr*)&other_addr, sizeof(other_addr));
}

//TO-DO
void timeout_handler(){
    // will need to change ss threshold 

    Queue * tmp_q = backup_queue; 
    for (int i = 0; i < 32; i++){
        if (!isEmpty(tmp_q)){
            packet p = front(tmp_q);
            send_pkt(&p);
            dequeue(tmp_q);
        }
    }
}

void rsend(char* hostname, 
            unsigned short int hostUDPport, 
            char* filename, 
            unsigned long long int bytesToTransfer) 
{
    int sockfd;
    struct sockaddr_in receiver_addr;
    int bytesRead, totalBytesSent = 0;
    FILE* file;

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset((char *) &other_addr, 0, sizeof (other_addr));
    other_addr.sin_family = AF_INET;
    other_addr.sin_port = htons(hostUDPport);
    other_addr.sin_addr.s_addr = inet_addr(hostname);  // maybe user inet_ntoa?

    file = fopen(filename, "rb");
    if (file == NULL) {
        perror("Failed to open file");
        exit(EXIT_FAILURE);
    }

    // Control flow here 
    
    fclose(file);
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

    if (argc != 5) {
        fprintf(stderr, "usage: %s receiver_hostname receiver_port filename_to_xfer bytes_to_xfer\n\n", argv[0]);
        exit(1);
    }
    hostUDPport = (unsigned short int) atoi(argv[2]);
    hostname = argv[1];
    char* filename = argv[3];
    bytesToTransfer = atoll(argv[4]);
    rsend(hostname, hostUDPport, filename, bytesToTransfer);


    return (EXIT_SUCCESS);
}