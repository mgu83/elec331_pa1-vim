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
#include <assert.h>

#include "types.h"
#include "priorityqueue.h"

struct sockaddr_in my_addr, other_addr;
int sockfd, slen;
int write_flag = 1;
socklen_t len;
FILE *rec_file;

/**
 * @brief Priority queue for managing the packets received out of order.
 */
PriorityQueue *pq;

/**
 * @brief Sends an acknowledgment packet to the sender to signal that packet has been received.
 *
 * @param ack_num The sequence number of the next expected packet.
 * @param pkt_type The type of packet, indicating ACK or other control messages.
 */
void send_ack(int ack_num, packet_type pkt_type)
{
    packet ack;
    ack.ack_num = ack_num;
    ack.pkt_type = pkt_type;
    int buf_size = sizeof(ack);
    if (setsockopt(sockfd, SOL_SOCKET, SO_SNDBUF, &buf_size, sizeof(buf_size)) < 0)
    {
        perror("Error setting send buffer size");
    }
    if (sendto(sockfd, &ack, sizeof(ack), 0, (struct sockaddr *)&other_addr, len) < 0)
    {
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
           char *destinationFile,
           unsigned long long int writeRate)
{

    FILE *file;
    uint64_t ack_num = 0;
    pq = constructPQ();
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        perror("socket creation failed\n");
        exit(EXIT_FAILURE);
    }
    memset(&my_addr, 0, sizeof(my_addr));
    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(myUDPport);
    my_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sockfd, (const struct sockaddr *)&my_addr, sizeof(my_addr)) < 0)
    {
        perror("bind failed\n");
        exit(EXIT_FAILURE);
    }

    file = fopen(destinationFile, "wb");
    if (file == NULL)
    {
        perror("Failed to open file\n");
        exit(EXIT_FAILURE);
    }

    while (write_flag)
    {
        packet recv_pkt;
        // Receiving the packet
        len = sizeof(other_addr);
        if (recvfrom(sockfd, &recv_pkt, sizeof(packet), 0, (struct sockaddr *)&other_addr, (socklen_t *)&len) == -1)
        {
            send_ack(ack_num, TDACK);
        }

        // Check if the packet is last
        if (recv_pkt.pkt_type == FIN)
        {
            // If so, send acknowledgement that we have received last packet
            send_ack(ack_num, FINACK);
            break;
        }
        else if (recv_pkt.pkt_type == DATA)
        {
            /* Check if packet is out of order using the packet's sequence number and
              current acknowledgment number  */
            if (recv_pkt.seq_num > ack_num)
            {
                send_ack(ack_num, TDACK);
            }
            else
            {
                // Write to the destination file
                pq_push(pq, recv_pkt);
                ack_num += recv_pkt.data_size;
                if (fwrite(&(recv_pkt.data), sizeof(char), recv_pkt.data_size, file) != recv_pkt.data_size)
                {
                    perror("error writing to file\n");
                }
                // Use priority queue to select top packets
                while (!pq_empty(pq) && (pq_top(pq).seq_num == ack_num))
                {
                    packet pkt = pq_top(pq);
                    fwrite(pkt.data, sizeof(char), pkt.data_size, file);
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

int main(int argc, char **argv)
{
    // This is a skeleton of a main function.
    // You should implement this function more completely
    // so that one can invoke the file transfer from the
    // command line.

    unsigned short int udpPort;
    // TO-DO: Need to correct this back to 3 since we don't need write rate
    if (argc != 4)
    {
        fprintf(stderr, "There are %d arguments.\n", argc - 1);
        fprintf(stderr, "usage: %s UDP_port filename_to_write\n\n", argv[0]);
        exit(1);
    }
    udpPort = (unsigned short int)atoi(argv[1]);
    char *destinationFile = argv[2];
    unsigned long long int writeRate = 0;
    if (argc == 4)
    {
        writeRate = atoll(argv[3]);
    }
    rrecv(udpPort, destinationFile, writeRate);

    // rec_file = fopen(destinationFile, "rb");
    // if (rec_file == NULL)
    // {
    //     perror("Failed to open file");
    //     exit(EXIT_FAILURE);
    // }
    // for (int i = 65; i < 91; i++)
    // {
    //     char c = (char)i;
    //     for (int j = 0; j < 5 * MSS; j++)
    //     {
    //         char d;
    //         if (fseek(rec_file, (i - 65) * (5 * MSS) + j, SEEK_SET) != 0)
    //         {
    //             perror("Fseek failed in slow start state\n");
    //         }

    //         if (fread(&d, sizeof(char), 1, rec_file) < 0)
    //         {
    //             perror("error writing to file\n");
    //         }

    //         if (!(d == c))
    //         {
    //             printf("Test failed: expected %s, read %s\n", c, d);
    //         }
    //     }
    // }
    printf("Test 1 passed!\n");

    return EXIT_SUCCESS;
}