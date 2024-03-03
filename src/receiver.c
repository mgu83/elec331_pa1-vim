/**
 * @file receiver.c
 * @author Maggie Gu (@mgu83), Vi Kankanamge (@vidunikankan)
 * @brief Receiver side functions 
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

#include "types.h"
#include "priorityqueue.h"

struct sockaddr_in my_addr, other_addr;
int sockfd, slen;
int write_flag = 1;
socklen_t len;

/**
 * @brief Priority queue to hold packets that prioritizes packets 
 * based on number
 * 
 */
PriorityQueue * pq;

/**
 * @brief Send acknowledgement to sender that packet has been received
 * 
 * @param ack_num 
 * @param pkt_type 
 */
void send_ack(int ack_num, packet_type pkt_type){
    packet ack;
    ack.ack_num = ack_num;
    ack.pkt_type = pkt_type;
    printf("Sending ACK for %d\n", ack_num);
    int buf_size = sizeof(ack);
    if(setsockopt(sockfd, SOL_SOCKET, SO_SNDBUF, &buf_size, sizeof(buf_size)) < 0){
        perror("Error setting send buffer size");
    }
    if (sendto(sockfd, &ack, sizeof(ack), 0, (struct sockaddr*) &other_addr, len) < 0) {
        perror("error in sending acknowledgement\n");
    }
}

/**
 * @brief Controls all the functions on receiver side
 * 
 * @param myUDPport 
 * @param destinationFile 
 * @param writeRate 
 */
void rrecv(unsigned short int myUDPport, 
            char* destinationFile, 
            unsigned long long int writeRate) {

    FILE* file;
    uint64_t ack_num = 0;
    pq = constructPQ();

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }
    memset(&my_addr, 0, sizeof(my_addr));
    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(myUDPport);
    my_addr.sin_addr.s_addr = INADDR_ANY;
    
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
    
    while (write_flag) {
      packet recv_pkt;
        /*struct timeval tv;
        tv.tv_sec = 10;
        tv.tv_usec = 0;
        if(setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv) < 0){
            perror("Error setting recv timeout\n");

        }*/
      // Receiving the packet
        len = sizeof(other_addr);
      if (recvfrom(sockfd, &recv_pkt, sizeof(packet), 0, (struct sockaddr*)&other_addr, (socklen_t*)&len) == -1) {
            printf("Waiting for new pkt, resending ACK for last pkt\n");
            send_ack(ack_num, TDACK);
      }
      
      // Check if the packet is last
      if (recv_pkt.pkt_type == FIN){
          printf("FIN packet has been received");
          // If so, send acknowledgement that we have received last packet
          send_ack(ack_num, FINACK);
        //   int size_count = 0;
        //     while (!pq_empty(pq) &&  size_count < ack_num){
        //         packet pkt = pq_top(pq);
        //         if(fwrite(pkt.data, sizeof(char), pkt.data_size, file) < 0){
        //             perror("Write to file failed\n");
        //         }
        //         size_count += pkt.data_size;
        //         pq_pop(pq);
        //     }
          break;
          
      }
      else if (recv_pkt.pkt_type == DATA){
        /* Check if packet is out of order using the packet's sequence number and
          current acknowledgment number  */ 
        if (recv_pkt.seq_num > ack_num) {
              printf("Received out of order packet\n");
              printf("Packet seq num %d\n", recv_pkt.seq_num);
              printf("ACk num %d\n", ack_num);
              send_ack(ack_num, TDACK);
        }
        else {
            // Write to the destination file 
            pq_push(pq, recv_pkt);
            ack_num += recv_pkt.data_size;
            printf("data received: %s\n", recv_pkt.data);

            if(fwrite(&(recv_pkt.data), sizeof(char), recv_pkt.data_size, file) != recv_pkt.data_size){
                perror("error writing to file\n");
            }
            ack_num += recv_pkt.data_size;
            // Use priority queue to select top packets 
            while (!pq_empty(pq) && (pq_top(pq).seq_num == ack_num)){
                packet pkt = pq_top(pq);
                fwrite(pkt.data, sizeof(char), pkt.data_size, file);
                ack_num += pkt.data_size;
                pq_pop(pq);
            }
            send_ack(ack_num, ACK);
        }
        // Send acknowledgment to the sender that packet has been received 
        //send_ack(ack_num, ACK); //NOTE: not sending anything to sender for now bc sender does not have a bound port
      }
   }
    fclose(file);
    close(sockfd);
    printf("%s received.", destinationFile);
}

int main(int argc, char** argv) {
    // This is a skeleton of a main function.
    // You should implement this function more completely
    // so that one can invoke the file transfer from the
    // command line.

    unsigned short int udpPort;
    // TO-DO: Need to correct this back to 3 since we don't need write rate
    if (argc !=  4) {
        fprintf(stderr, "There are %d arguments.\n", argc - 1); 
        fprintf(stderr, "usage: %s UDP_port filename_to_write\n\n", argv[0]);
        exit(1);
    }
    udpPort = (unsigned short int) atoi(argv[1]);
    char* destinationFile = argv[2];
    unsigned long long int writeRate = 0;
    if (argc == 4) {
      writeRate = atoll(argv[3]);
    }
    rrecv(udpPort, destinationFile, writeRate);

    return EXIT_SUCCESS;
}