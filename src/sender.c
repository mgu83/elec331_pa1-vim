/**
 * @file sender.c
 * @author Maggie Gu (@mgu83)
 * @author Vi Kankanamge (@vidunikankan)
 * @brief This module implements the sender side of a UDP-based file transfer protocol.
 *
 * The sender is responsible for breaking down a large file into smaller data packets, sequentially
 * transmitting these packets over UDP to a specified receiver, and handling acknowledgments (ACKs) from
 * the receiver to ensure reliable data transfer.
 *
 * @bug No known bugs.
 */
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
#include "types.h"
#include "queue.h"

/**
 * @brief Utility macros for calculating maximum and minimum of two values.
 */
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))

// BUFFER SIZE can be changed
#define BUFFER_SIZE 1024

struct sockaddr_in my_addr, other_addr;
int sockfd;
socklen_t slen;
FILE *file;
FILE *send_file;
uint64_t seq_num = 0;
state_type status = SLOW_START;
float cong_win_size = MSS;
uint64_t total_num_pkt;
uint64_t bytes_to_send;
uint64_t bytes_to_send_total;
uint64_t last_bytes_read;
uint64_t total_sent = 0;
uint64_t total_received = 0;
uint64_t highest_received = -1;
uint64_t total_duplicated = 0;
float ss_threshold;
uint64_t prev_mss, cur_mss = MSS;
uint64_t cwnd = MSS;
uint64_t ssthresh = 4 * MSS;
uint64_t bytes_sent_and_ackd = 0;
uint64_t packet_seq_num = 0;
state_type state;
int retransmit_flag = 0;

/**
 * @brief Queue for tracking packets sent but not yet acknowledged.
 */
Queue *sent_not_ackd;

/**
 * @brief Queue for managing packets ready to be sent.
 */
Queue *ready_to_send;

/**
 * @brief Sends a packet to the receiver over connection.
 *
 * @param pkt Pointer to the packet structure to be sent.
 */
void send_pkt(packet *pkt)
{
    if (sendto(sockfd, pkt, sizeof(packet), 0, (struct sockaddr *)&other_addr, sizeof(other_addr)) < 0)
    {
        perror("Error sending bytes..\n");
    }
}

/**
 * @brief Prepares and sends packets based on available congestion window size.
 *
 * Reads data from the input file, segments it into packets conforming to the current
 * congestion window size, enqueues packets for sending, and transmits them to the receiver.
 */
void queue_send()
{
    // Check if there is anything to send
    if (bytes_to_send == 0)
    {
        return;
    }
    // Initialize bugger to store file data
    char buffer[MSS];
    packet pkt;
    int packets_to_send = floor((cong_win_size - size(sent_not_ackd) * MSS) / MSS); // available space / max space per packet

    for (int i = 0; i < packets_to_send; i++)
    {
        size_t read_size = fread(buffer, sizeof(char), MIN(bytes_to_send, MSS), file);
        if (read_size > 0)
        {
            pkt.pkt_type = DATA;
            pkt.data_size = read_size;
            pkt.seq_num = seq_num;
            // Copy read data into packet
            memcpy(pkt.data, &buffer, read_size);
            // Enqueue packet into both queues
            enqueue(sent_not_ackd, pkt);
            enqueue(ready_to_send, pkt);
            // Update sequence number and remaining bytes to send
            seq_num += read_size;
            bytes_to_send -= read_size;
        }
    }
    // Send all packets that are ready to be sent
    while (!isEmpty(ready_to_send))
    {
        packet p = front(ready_to_send);
        total_sent++;
        send_pkt(&p);
        dequeue(ready_to_send);
    }
}

/**
 * @brief Creates a packet from file data and sends it, considering retransmission needs.
 *
 * Reads a portion of data from the file, encapsulates it into a packet, and sends it.
 * Adjusts the sequence number accordingly unless in retransmission mode.
 */
void create_send_pkt()
{
    int pkts_to_send = floor((bytes_to_send - bytes_sent_and_ackd) / cwnd);
    char buffer[cwnd];
    int bytes_read;
    packet ss_pkt;

    bytes_read = fread(buffer, sizeof(char), MIN(bytes_to_send, cwnd), file);
    if (bytes_read != MIN(bytes_to_send, cwnd))
    {
        perror("Fread failed in slow start\n");
    }
    ss_pkt.pkt_type = DATA;
    ss_pkt.data_size = bytes_read;
    ss_pkt.seq_num = packet_seq_num;
    memcpy(ss_pkt.data, &buffer, bytes_read);
    if (!retransmit_flag)
    {
        packet_seq_num += bytes_read;
    }
    last_bytes_read = bytes_read;
    send_pkt(&ss_pkt);
}

/**
 * @brief Checks for acknowledgments from the receiver to manage sent packets.
 *
 * Listens for ACK packets from the receiver, adjusting the congestion window and
 * retransmission needs based on received acknowledgments.
 */
void check_ack()
{
    packet ackpkt;
    while (bytes_sent_and_ackd < packet_seq_num)
    {
        int buf_size = sizeof(packet);
        if (setsockopt(sockfd, SOL_SOCKET, SO_SNDBUF, &buf_size, sizeof(buf_size)) < 0)
        {
            perror("Error setting send buffer size\n");
        }
        if (recvfrom(sockfd, &ackpkt, sizeof(packet), 0, (struct sockaddr *)&other_addr, (socklen_t *)&slen) == -1)
        {
            perror("Error receiving ack\n");
        }
        if (ackpkt.pkt_type == ACK && ackpkt.ack_num == packet_seq_num)
        { //&& ackpkt.ack_num == packet_seq_num
            bytes_sent_and_ackd += last_bytes_read;
            bytes_to_send -= last_bytes_read;
            switch (state)
            {
            case SLOW_START:
                if ((cwnd * 2) > ssthresh)
                {
                    state = CONG_AVOID;
                }
                else
                {
                    state = SLOW_START;
                }
                break;
            case SLOW_RETRANS:
                state = SLOW_START;
                retransmit_flag = 0;
                break;
            default:
                break;
            }
        }
        else if (ackpkt.pkt_type == TDACK)
        {
            switch (state)
            {
            case SLOW_START:
                state = SLOW_RETRANS;
                break;
            case CONG_AVOID:
                ssthresh = floor(cwnd / 2);
                cwnd = MSS;
                state = SLOW_START;
                break;
            default:
                break;
            }
            break;
        }
    }
}

/**
 * @brief Manages packet transmission through different phases of the transfer protocol.
 *
 * Controls the sending of packets by dynamically adjusting to network conditions
 * and transitioning through slow start, congestion avoidance, and retransmission states.
 */
void dyn_pkts()
{
    while (bytes_sent_and_ackd < bytes_to_send_total)
    {
        switch (state)
        {
        case SLOW_START:
            cwnd *= 2;
            create_send_pkt();
            check_ack();
            break;
        case CONG_AVOID:
            cwnd += MSS;
            create_send_pkt();
            check_ack();
            break;

        case SLOW_RETRANS:
            retransmit_flag = 1;
            create_send_pkt();
            check_ack();
            break;
        }
    }
}

/**
 * @brief Sends a final acknowledgment packet indicating the completion of data transmission.
 */
void send_final_ack()
{
    char temp[sizeof(packet)];
    packet pkt;
    packet ack;
    pkt.pkt_type = FIN;
    pkt.data_size = 0;
    memset(pkt.data, 0, MSS);
    send_pkt(&pkt);
    while (1)
    {
        slen = sizeof(other_addr);
        if (recvfrom(sockfd, temp, sizeof(packet), 0, (struct sockaddr *)&other_addr, (socklen_t *)&slen) == -1)
        {
            pkt.pkt_type = FIN;
            pkt.data_size = 0;
            memset(pkt.data, 0, MSS);
            send_pkt(&pkt);
        }
        else
        {
            memcpy(&ack, temp, sizeof(packet));
            if (ack.pkt_type == FINACK)
            {
                pkt.pkt_type = FINACK;
                pkt.data_size = 0;
                send_pkt(&pkt);
                break;
            }
        }
    }
}

/**
 * @brief Initiates the file transfer process to the specified receiver.
 *
 * Sets up the UDP connection, prepares the file for reading, and manages the sending process.
 *
 * @param hostname The IP address or hostname of the receiver.
 * @param hostUDPport The UDP port number on which the receiver is listening.
 * @param filename Path to the file to be sent.
 * @param bytesToTransfer The size of the file or the amount of data to be transferred.
 */
void rsend(char *hostname,
           unsigned short int hostUDPport,
           char *filename,
           unsigned long long int bytesToTransfer)
{
    total_num_pkt = ceil(1.0 * bytesToTransfer / MSS);
    bytes_to_send = bytesToTransfer;
    packet pkt;
    sent_not_ackd = constructQueue();
    ready_to_send = constructQueue();

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }
    memset((char *)&other_addr, 0, sizeof(other_addr));
    memset((char *)&my_addr, 0, sizeof(my_addr));
    other_addr.sin_family = AF_INET;
    other_addr.sin_port = htons(hostUDPport);
    other_addr.sin_addr.s_addr = inet_addr(hostname);
    file = fopen(filename, "rb");
    if (file == NULL)
    {
        perror("Failed to open file");
        exit(EXIT_FAILURE);
    }
    int characters = 0;
    for (char c = getc(file); c != EOF; c = getc(file)){
        characters++;
    }
    if(bytesToTransfer > characters){
        bytesToTransfer = characters;
    }
    if(fseek(file, 0, SEEK_SET) != 0){
            perror("Fseek failed\n");
    }

    bytes_sent_and_ackd = 0;
    bytes_to_send = bytesToTransfer;
    bytes_to_send_total = bytesToTransfer;
    state = SLOW_START;
    cur_mss = MSS;
    dyn_pkts();
    send_final_ack();
    fclose(file);
}

int main(int argc, char **argv)
{
    // This is a skeleton of a main function.
    // You should implement this function more completely
    // so that one can invoke the file transfer from the
    // command line.
    int hostUDPport;
    unsigned long long int bytesToTransfer;
    char *hostname = NULL;

    if (argc != 5)
    {
        fprintf(stderr, "usage: %s receiver_hostname receiver_port filename_to_xfer bytes_to_xfer\n\n", argv[0]);
        exit(1);
    }
    hostUDPport = (unsigned short int)atoi(argv[2]);
    hostname = argv[1];
    char *filename = argv[3];
    bytesToTransfer = atoll(argv[4]);

    /*send_file = fopen(filename, "wb");
    if (send_file == NULL)
    {
        perror("Failed to open file at beginning");
        fclose(send_file);
        exit(EXIT_FAILURE);
    }
    // for (int i = 65; i < 91; i++)
    // {
    //     char c = (char)i;
    //     for (int j = 0; j < 5 * MSS; j++)
    //     {
    //         if (fwrite(&c, sizeof(char), 1, send_file) != 1)
    //         {
    //             perror("error writing to file\n");
    //         }
    //     }
    // }
    fclose(send_file);*/

    rsend(hostname, hostUDPport, filename, bytesToTransfer);

    return (EXIT_SUCCESS);
}