
## UDP-based TCP Implementation

##### Overview
This project implements a custom UDP-based file transfer protocol consisting of two main components: a sender and a receiver. The sender module is responsible for breaking down a large file into smaller data packets and sequentially transmitting these packets over UDP to the specified receiver. The receiver module listens for incoming data packets, handles them according to their sequence numbers to ensure data is written in the correct order, and sends acknowledgments back to the sender.

##### Compileation and Execution
To compile the code, use 'make clean' and 'make'.
To execute receiver, use './receiver [unsigned short int myUDPport] [char *destinationFile] [unsigned long long int writeRate]'. 
To execute sender, use './sender [char *hostname] [unsigned short int hostUDPport] [char *filename] [unsigned long long int bytesToTransfer (optional)]'.

##### Authors
- Maggie Gu (mgu83)
- Vi Kankanamge (vidunikankan)

##### Implementation Highlights
Priority Queue: Manages out-of-order packets on the receiver side, ensuring data integrity.
Dynamic Congestion Window: Optimizes data flow based on network conditions, implemented in the sender module. Uses slow start and AIMD 
protocols to optimize throughput while maintaining fairness.
State Machine: Governs the sender's behavior, transitioning between slow start, congestion avoidance, and fast recovery.
Queues: Used two queues to keep track of packets on sender side. See below.
- **Sent Not Acknowledged Queue:** Tracks packets that have been sent but not yet acknowledged by the receiver.
- **Ready To Send Queue:** Holds packets that are prepared and ready to be sent, considering the available congestion window size.

##### Challenges and Learning Outcomes
A major challenge we faced was implementing the fairness/congestion avoidance protocol. While implementing the states, our communication
between the sender and the receiver broke down completely, and we were not able to receive any packets for a while. In the end, we
figured out that this was because our packet size got too large during the slow-start state, and so we had to change the baseline
MSS to account for the exponential increase. In the end, we were able to implement the correct protocols.


##### No known bugs.
