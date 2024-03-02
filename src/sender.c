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
uint64_t total_duplicated = 0;
float ss_threshold;

Queue * backup_queue;
Queue * pending_queue;

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
    int packets_to_send = floor((cong_win_size - size(backup_queue) * MSS) / MSS); // available space / max space per packet
    
    for (int i = 0; i < packets_to_send; i++){
        size_t read_size = fread(buffer, sizeof(char), MIN(bytes_to_send, MSS), file);
        if (read_size > 0){
            pkt.pkt_type = DATA;
            pkt.data_size = read_size;
            pkt.seq_num = seq_num;
            // Copy read data into packet
            memcpy(pkt.data, &buffer, read_size); 
            // Enqueue packet into both queues
            enqueue(backup_queue, pkt);
            enqueue(pending_queue, pkt);
            // Update sequence number and remaining bytes to send
            seq_num += read_size;
            bytes_to_send -= read_size;
        }
    }
    
    // Send all packets from the first queue
    while (!isEmpty(pending_queue)){
        packet p = front(pending_queue);
        total_sent++;
        send_pkt(&p);
        dequeue(pending_queue);
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
    while (1){
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

void rsend(char* hostname, 
            unsigned short int hostUDPport, 
            char* filename, 
            unsigned long long int bytesToTransfer) {

    total_num_pkt = ceil(1.0 * bytesToTransfer / MSS);
    bytes_to_send = bytesToTransfer;
    packet pkt;
    backup_queue = constructQueue();
    pending_queue = constructQueue();
    
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
            if (!isEmpty(backup_queue)) {
                // Reset everything 
                ss_threshold = MAX(2 * MSS, cong_win_size / 2);
                cong_win_size = MSS;
                status = SLOW_START;
            }
        }
        if (pkt.pkt_type == ACK){
            // Stale ACK
            if (pkt.ack_num < front(backup_queue).seq_num){
                return;
            }
            // Is this a ACK we have received? Duplicated
            else if (pkt.ack_num == front(backup_queue).seq_num){
                total_duplicated++;
            }
            // New ACK
            else {
                total_duplicated = 0;
                int num_pkt = ceil((pkt.ack_num - front(backup_queue).seq_num) / (1.0 * MSS));
                int count = 0;
                total_received += num_pkt;
                while(!isEmpty(backup_queue) && count < num_pkt){
                    printf("Received ACK for packet %d \n", (front(backup_queue).seq_num)/MSS);
                    dequeue(backup_queue);
                    count++;
                }
                queue_send();
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