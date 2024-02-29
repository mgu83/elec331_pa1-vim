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

#define BUFFER_SIZE 1024

void rsend(char* hostname, 
            unsigned short int hostUDPport, 
            char* filename, 
            unsigned long long int bytesToTransfer) 
{
    int sockfd;
    struct sockaddr_in receiver_addr;
    char buffer[1024];
    int bytesRead, totalBytesSent = 0;
    FILE* file;

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&receiver_addr, 0, sizeof(receiver_addr));

    receiver_addr.sin_family = AF_INET;
    receiver_addr.sin_port = htons(hostUDPport);
    receiver_addr.sin_addr.s_addr = inet_addr(hostname);  // maybe user inet_ntoa?

    file = fopen(filename, "rb");
    if (file == NULL) {
        perror("Failed to open file");
        exit(EXIT_FAILURE);
    }

    while ((bytesRead = fread(buffer, 1, BUFFER_SIZE, file)) > 0 && totalBytesSent < bytesToTransfer) {
        sendto(sockfd, buffer, bytesRead, 0, (const struct sockaddr*) &receiver_addr, sizeof(receiver_addr));
         printf("Bytes read: %d\n", bytesRead); 
        totalBytesSent += bytesRead;
    }
    printf("done");

    fclose(file);
    close(sockfd);

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