#include "blockchain.h"
#include <iostream>


int main (int argc, char* argv[]) {
    Block_t b, b2;
    b = first_block();
    b2 = init_block(b.header.hash, 1);
    link_block(b2);
    json blockchain = get_blockchain();
    printf("%s\n", blockchain.dump(INDENT).c_str());
    port_t port;
    int sockfd = init_socket(port);

    if (port >= SERVICE_PORT) service(sockfd);
    else if(port == MINER_PORT) miner(sockfd);
    else node(sockfd, port);

    return 0;
}
port_t init_port(int argc, char* argv[]){
    port_t port;
    if (argc == 1)
        return get_port();
    return std::atoi(argv[1]);
}
int get_port(){
    int port;
    while (true) {
        printf("enter port\n>");
        std::cin >> port;
        if (port > 1024 && port < 9999){
            return port; 
        }
    }
}

uint8_t get_vote(){
    int vote;
    while (true) {
        printf("enter your vote[0,1]:\n>");
        std::cin >> vote;
        if (vote == 0 or vote == 1){
            return vote; 
        } else {
            printf("try again\n");
        }
    }
}
