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
            unsigned long long int writeRate) {
                int sockfd;
    struct sockaddr_in my_addr;
    char buffer[1024];
    int n;
    FILE* file;

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }
  printf("here");
    memset(&my_addr, 0, sizeof(my_addr));

    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(myUDPport);
    my_addr.sin_addr.s_addr = INADDR_ANY;
    socklen_t len = sizeof(my_addr);

    if (bind(sockfd, (const struct sockaddr *)&my_addr, sizeof(my_addr)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
  printf("here");
    file = fopen(destinationFile, "wb");
    if (file == NULL) {
        perror("Failed to open file");
        exit(EXIT_FAILURE);
    }
    printf("here");

    while ((n = recvfrom(sockfd, buffer, 1024, 0, (struct sockaddr *) &my_addr, &len)) > 0) {
    fwrite(buffer, 1, n, file);

    // Calculate the time to sleep to match the writeRate.
    if (writeRate > 0) {
        // Calculate the time to sleep in microseconds.
        // (1 second = 1,000,000 microseconds)
        unsigned long long timeToSleep = ((unsigned long long)n * 1000000) / writeRate;

        usleep(timeToSleep);
    }

    for (int i = 0; i < n; ++i) {
        printf("%02x ", (unsigned char)buffer[i]); // Print each byte as a two-digit hexadecimal number
    }
    printf("\n");
}


    fclose(file);
    close(sockfd);

}

int main(int argc, char** argv) {
    // This is a skeleton of a main function.
    // You should implement this function more completely
    // so that one can invoke the file transfer from the
    // command line.

    unsigned short int udpPort;
  printf("hereeee");
    if (argc != 4) {
        fprintf(stderr, "There are %d arguments.\n", argc - 1); 
        fprintf(stderr, "usage: %s UDP_port filename_to_write\n\n", argv[0]);
        exit(1);
    }
  printf("hereeee");
    udpPort = (unsigned short int) atoi(argv[1]);
    char* destinationFile = argv[2];
    unsigned long long int writeRate = atoll(argv[3]);
  printf("hereeee");
   rrecv(udpPort, destinationFile, writeRate);
   printf("this is running");

    return EXIT_SUCCESS;
}
