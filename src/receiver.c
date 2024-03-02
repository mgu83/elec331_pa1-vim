/**
 * @file receiver.c
 * @author Maggie Gu (@mgu83), [INSERT VI'S INFO]
 * @brief 
 * 
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/stat.h>
#include <signal.h>
#include <string.h>
#include <sys/time.h>
#include <errno.h>
#include <math.h>
#include <unistd.h>
#include <fcntl.h>
#include <netdb.h>

#include "param.h"
#include "priorityqueue.h"

struct sockaddr_in my_addr, si_other;
int sockfd, slen;

PriorityQueue * pq;


void send_ack(int ack_num, packet_type pkt_type){
    packet ack;
    ack.ack_num = ack_num;
    ack.pkt_type = pkt_type;
    if (sendto(sockfd, &ack, sizeof(packet), 0, (struct sockaddr*) &si_other, slen) == -1) {
        printf("error in sending acknowledgement");
    }
}

void rrecv(unsigned short int myUDPport, 
            char* destinationFile, 
            unsigned long long int writeRate) {

    FILE* file;
    int ack_num = 0;

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }
    memset(&my_addr, 0, sizeof(my_addr));
    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(myUDPport);
    my_addr.sin_addr.s_addr = INADDR_ANY;
    socklen_t len = sizeof(my_addr);
    printf("Now binding\n");

    if (bind(sockfd, (const struct sockaddr *)&my_addr, sizeof(my_addr)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    
    file = fopen(destinationFile, "wb");
    if (file == NULL) {
        perror("Failed to open file");
        exit(EXIT_FAILURE);
    }
    
    while (1) {
      packet recv_pkt;
      // Receiving the packet
      if (recvfrom(sockfd, &recv_pkt, sizeof(packet), 0, (const struct sockaddr *)&si_other, (socklen_t*)&slen) == -1) {
        printf("error in recvfrom");
      }
      // Check if the packet is last
      if (recv_pkt.pkt_type == FIN){
          printf("FIN packet has been received");
          // If so, send acknowledgement that we have received last packet
          send_ack(ack_num, FINACK);
          break;
      }
      else if (recv_pkt.pkt_type == DATA){
        /* Check if packet is out of order using the packet's sequence number and
          current acknowledgment number  */ 
        if (recv_pkt.seq_num > ack_num) {
              printf("Received out of order packet");
              if (pq_size(pq) < MAX_QUEUE_SIZE){
                  pq_push(pq, recv_pkt);
              }
        }
        else {
            // Write to the destination file 
            fwrite(recv_pkt.data, sizeof(char), recv_pkt.data_size, file);
            ack_num += recv_pkt.data_size;
            // Use priority queue to select top packets 
            while (!pq_empty(pq) && pq_top(pq).seq_num == ack_num){
                packet pkt = pq_top(pq);
                fwrite(pkt.data, sizeof(char), pkt.data_size, file);
                ack_num += pkt.data_size;
                pq_pop(pq);
            }
        }
        // Send acknowledgment to the sender that packet has been received 
        send_ack(ack_num, ACK);
      }
   }
    fclose(file);
    close(sockfd);
    printf("%sockfd received.", destinationFile);
}

int main(int argc, char** argv) {
    // This is a skeleton of a main function.
    // You should implement this function more completely
    // so that one can invoke the file transfer from the
    // command line.

    unsigned short int udpPort;
    // TO-DO: Need to correct this back to 3 since we don't need write rate
    if (argc != 4) {
        fprintf(stderr, "There are %d arguments.\n", argc - 1); 
        fprintf(stderr, "usage: %s UDP_port filename_to_write\n\n", argv[0]);
        exit(1);
    }
    udpPort = (unsigned short int) atoi(argv[1]);
    char* destinationFile = argv[2];
    unsigned long long int writeRate = atoll(argv[3]);
    rrecv(udpPort, destinationFile, writeRate);

    return EXIT_SUCCESS;
}
