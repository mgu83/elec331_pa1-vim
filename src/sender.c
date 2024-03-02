/**
 * @file sender.c
 * @author Maggie Gu (@mgu83) Vi Kankanamge (@vidunikankan)
 * @brief 
 * 
 * 
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
 * @brief MAX & MIN macros since C does not have built in
 * 
 */
#define MAX(a,b) ((a) > (b) ? (a) : (b))
#define MIN(a,b) ((a) < (b) ? (a) : (b))

// BUFFER SIZE can be changed
#define BUFFER_SIZE 1024

struct sockaddr_in my_addr, other_addr;
int sockfd;
socklen_t slen;
FILE* file;
FILE* send_file;
uint64_t seq_num = 0;
state_type status = SLOW_START;
float cong_win_size = MSS;
uint64_t total_num_pkt;
uint64_t bytes_to_send;
uint64_t total_sent = 0;
uint64_t total_received = 0;
uint64_t highest_received = -1;
uint64_t total_duplicated = 0;
float ss_threshold;

/**
 * @brief This queue acts as a record of all packets that have been sent but not yet acknowledged
 * It allows the sender to keep track of which packets might need to be retransmitted in case an 
 * ACK is not received within a certain timeout period or in response to receiving duplicate ACKs.
 * 
 */
Queue * sent_not_ackd;
/**
 * @brief This queue is used to manage the current set of packets that are prepared and ready to 
 * be sent. It is temporary storage for packets that are about to be sent. Once a packet is sent, 
 * it is removed from this queue but remains in backup_queue until it is acknowledged.
 * 
 */
Queue * ready_to_send;

/**
 * @brief Sends packet across connection
 * 
 * @param pkt 
 */
void send_pkt(packet* pkt){
    if(sendto(sockfd, pkt, sizeof(packet), 0, (struct sockaddr*)&other_addr, sizeof(other_addr)) <  0){
        perror("Error sending bytes..\n");
    }
}

/**
 * @brief Add packets to queues and then calls send_pkt
 * 
 */
void queue_send(){
    // Check if there is anything to send
    if (bytes_to_send == 0){
        return;
    }
    // Initialize bugger to store file data
    char buffer[MSS];
    packet pkt;
    int packets_to_send = floor((cong_win_size - size(sent_not_ackd) * MSS) / MSS); // available space / max space per packet
    
    for (int i = 0; i < packets_to_send; i++){
        size_t read_size = fread(buffer, sizeof(char), MIN(bytes_to_send, MSS), file);
        if (read_size > 0){
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
    while (!isEmpty(ready_to_send)){
        packet p = front(ready_to_send);
        total_sent++;
        send_pkt(&p);
        dequeue(ready_to_send);
    }
}

/**
 * @brief Sends final acknowledgment package before connection terminates
 * 
 */
void send_final_ack(){
    char temp[sizeof(packet)];
    packet pkt;
    packet ack;
    pkt.pkt_type = FIN;
    pkt.data_size = 0;
    memset(pkt.data, 0, MSS);
    send_pkt(&pkt);
    while (1) {
        slen = sizeof(other_addr);
        if (recvfrom(sockfd, temp, sizeof(packet), 0, (struct sockaddr *)&other_addr, (socklen_t*)&slen) == -1){
                pkt.pkt_type = FIN;
                pkt.data_size = 0;
                memset(pkt.data, 0, MSS);
                send_pkt(&pkt);
        }
        else{
            memcpy(&ack, temp, sizeof(packet));
            if (ack.pkt_type == FINACK){
                pkt.pkt_type = FINACK;
                pkt.data_size = 0;
                send_pkt(&pkt);
                break;
            }
        }
    }
}

/**
 * @brief Controls all the functions on receiver side
 * 
 * @param hostname 
 * @param hostUDPport 
 * @param filename 
 * @param bytesToTransfer 
 */
void rsend(char* hostname, 
            unsigned short int hostUDPport, 
            char* filename, 
            unsigned long long int bytesToTransfer) {

    total_num_pkt = ceil(1.0 * bytesToTransfer / MSS);
    bytes_to_send = bytesToTransfer;
    packet pkt;
    sent_not_ackd = constructQueue();
    ready_to_send = constructQueue();
    
    printf("Inside rsend\n");
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset((char *) &other_addr, 0, sizeof (other_addr));
    memset((char *) &my_addr, 0, sizeof (my_addr));
    other_addr.sin_family = AF_INET;
    other_addr.sin_port = htons(hostUDPport);
    other_addr.sin_addr.s_addr = inet_addr(hostname);  // maybe user inet_ntoa?
    printf("After memset\n");
    file = fopen(filename, "rb");
    if (file == NULL) {
        perror("Failed to open file");
        exit(EXIT_FAILURE);
    }

    printf("File opened\n");
    queue_send();
    printf("Queue sent\n");
    while (total_sent < total_num_pkt || total_received < total_num_pkt){
        printf("Waiting for ACKs\n");
        slen = sizeof(other_addr);
        if (recvfrom(sockfd, &pkt, sizeof(packet), 0, (struct sockaddr*)&other_addr, (socklen_t*)&slen) == -1) {
            printf("error in recvfrom");
            // Restransmit everything in sent_not_ackd queue BUT DO NOT ERASE ANYTHING FROM IT
            if (!isEmpty(sent_not_ackd)) {
                Queue* tosend = sent_not_ackd;
                while (!isEmpty(tosend)) {
                    packet pk = front(tosend);
                    send_pkt(&pk);
                    dequeue(tosend);
                }
                ss_threshold = MAX(2 * MSS, cong_win_size / 2);
                cong_win_size = MSS;
                status = SLOW_START;
            }
        }
        if (pkt.pkt_type == ACK){
            // New ACK
            if (pkt.ack_num > front(sent_not_ackd).seq_num){
                highest_received = pkt.ack_num;
                total_duplicated = 0;
                int num_pkt = ceil((pkt.ack_num - front(sent_not_ackd).seq_num) / (1.0 * MSS));
                int count = 0;
                total_received += num_pkt;
                while(!isEmpty(sent_not_ackd) && count < num_pkt){
                    printf("Received ACK for packet %d \n", (front(sent_not_ackd).seq_num)/MSS);
                    dequeue(sent_not_ackd);
                    count++;
                }
                queue_send();
            } else if (pkt.ack_num == front(sent_not_ackd).seq_num){ // Is this a ACK we have received? Duplicated
                total_duplicated++;
                cong_win_size = MAX(cong_win_size / 2, 1 * MSS); // Ensure window size doesn't fall below 1 MSS
                ss_threshold = cong_win_size;
                status = FAST_RETRANSMIT; // TO-DO: we need to deal with being in FAST_RETRANSMIT
            }
        }
    }
    printf("All ACKs received\n");
    send_final_ack();
    fclose(file);
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
    
    printf("%s\n", filename);
    send_file = fopen(filename, "wb");
    if (send_file == NULL) {
        perror("Failed to open file at beginning");
        fclose(send_file);
        exit(EXIT_FAILURE);
    }
    for(int i = 65; i < 91; i++){
        char c = (char)i;
        for(int j = 0; j < MSS; j++){
            if(fwrite(&c, sizeof(char), 1, send_file) != 1){
                perror("error writing to file\n");
            }
        }

    }
    fclose(send_file);

    rsend(hostname, hostUDPport, filename, bytesToTransfer);

    return (EXIT_SUCCESS);
}