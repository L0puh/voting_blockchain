#include "blockchain.h"
#include <iostream>

port_t init_port(int argc, char* argv[]);
int get_port();

int main (int argc, char* argv[]) {
    Net net(init_port(argc, argv));

    Block block(2);
    Block_t b, b2;

    b = block.first_block();
    b2 = block.init_block(b.header.hash, 0);
    block.link_block(b2);

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
