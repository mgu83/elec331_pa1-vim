/**
 * @file receiver.c
 * @author Maggie Gu (@mgu83), 
 * @author Vi Kankanamge (@vidunikankan)
 * @brief This module implements the receiver side of a custom UDP-based file transfer protocol.
 * 
 * The receiver is designed to listen for incoming data packets over UDP, handle them according to
 * their sequence numbers, and write the data content to a specified output file in the correct order.
 * It utilizes a priority queue to manage out-of-order packets. The receiver also sends acknowledgments (ACKs)
 * back to the sender for each packet received, facilitating reliable data transfer over an unreliable protocol.
 * 
 * @bug No known bugs.
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
 * @brief Priority queue for managing the packets received out of order.
 */
PriorityQueue * pq;

/**
 * @brief Sends an acknowledgment packet to the sender to signal that packet has been received.
 *
 * @param ack_num The sequence number of the next expected packet.
 * @param pkt_type The type of packet, indicating ACK or other control messages.
 */
void send_ack(int ack_num, packet_type pkt_type){
    packet ack;
    ack.ack_num = ack_num;
    ack.pkt_type = pkt_type;
    int buf_size = sizeof(ack);
    if(setsockopt(sockfd, SOL_SOCKET, SO_SNDBUF, &buf_size, sizeof(buf_size)) < 0){
        perror("Error setting send buffer size");
    }
    if (sendto(sockfd, &ack, sizeof(ack), 0, (struct sockaddr*) &other_addr, len) < 0) {
        perror("error in sending acknowledgement\n");
    }
}

/**
 * @brief Main function to control the receiving and processing of packets.
 *
 * Initializes the network socket, binds it to the specified UDP port, and enters a loop to receive data packets.
 * Upon receiving a packet, it checks the packet type and handles it accordingly.
 *
 * @param myUDPport The UDP port on which the receiver listens for incoming packets.
 * @param destinationFile The path to the output file where the received data will be written.
 * @param writeRate Initially intended for controlling the write speed to the file.
 */
void rrecv(unsigned short int myUDPport, 
            char* destinationFile, 
            unsigned long long int writeRate) {
    FILE* file;
    uint64_t ack_num = 0;
    double writeDelay = writeRate > 0 ? (1.0 / writeRate) : 0; // Time in seconds for 1 byte
    struct timespec reqDelay;
    pq = constructPQ();
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }
    memset(&my_addr, 0, sizeof(my_addr));
    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(myUDPport);
    my_addr.sin_addr.s_addr = INADDR_ANY;
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
        // Receiving the packet
        len = sizeof(other_addr);
        if (recvfrom(sockfd, &recv_pkt, sizeof(packet), 0, (struct sockaddr*)&other_addr, (socklen_t*)&len) == -1) {
            send_ack(ack_num, TDACK);
        }
      
        // Check if the packet is last
        if (recv_pkt.pkt_type == FIN){
            printf("FIN packet has been received");
            // If so, send acknowledgement that we have received last packet
            send_ack(ack_num, FINACK);
            break;    
      }
      else if (recv_pkt.pkt_type == DATA){
        reqDelay.tv_nsec = (time_t) (writeDelay * recv_pkt.data_size);
        reqDelay.tv_nsec = (long)((writeDelay * recv_pkt.data_size - reqDelay.tv_sec) * 1e9);
        /* Check if packet is out of order using the packet's sequence number and
          current acknowledgment number  */ 
        if (recv_pkt.seq_num > ack_num) {
              send_ack(ack_num, TDACK);
        }
        else {
            // Write to the destination file 
            pq_push(pq, recv_pkt);
            ack_num += recv_pkt.data_size;
            if(fwrite(&(recv_pkt.data), sizeof(char), recv_pkt.data_size, file) != recv_pkt.data_size){
                perror("error writing to file\n");
            } else {
                nanosleep(&reqDelay, NULL);
            }
            // Use priority queue to select top packets 
            while (!pq_empty(pq) && (pq_top(pq).seq_num == ack_num)){
                packet pkt = pq_top(pq);
                fwrite(pkt.data, sizeof(char), pkt.data_size, file);
                nanosleep(&reqDelay, NULL);
                ack_num += pkt.data_size;
                pq_pop(pq);
            }
            send_ack(ack_num, ACK);
        }
        // Send acknowledgment to the sender that packet has been received 
      }
   }
    fclose(file);
    close(sockfd);
}

/**
 * @brief Main control function in receiver.c
 * 
 * @param argc 
 * @param argv 
 * @return int 
 */
int main(int argc, char** argv) {
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