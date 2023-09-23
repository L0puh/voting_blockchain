#include "blockchain.h"
#include <iostream>

int get_port();
int main () {
    Net net(get_port());
    Block block(2);
    Block_t b, b2;
    b = block.first_block();
    b2 = block.init_block(b.header.hash, 0);
    block.link_block(b2);
    return 0;
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
