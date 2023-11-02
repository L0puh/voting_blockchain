#include "blockchain.h"
#include <iostream>

port_t init_port(int argc, char* argv[]);
int get_port();
uint8_t get_vote();

int main (int argc, char* argv[]) {
    Block blockch(2);
    Block_t b, b2;

    b = blockch.first_block();
    b2 = blockch.init_block(b.header.hash, 0);
    blockch.link_block(b2);
    /* Net net(init_port(argc, argv), &block); */
    
    Vote v;
    Block_t bl = blockch.get_last(blockch.get_blockchain());
    Block_t block = v.vote(bl, get_vote());
    printf("block done:\n%s\n", blockch.block_to_string(block).c_str());
    std::pair<EVP_PKEY*, EVP_PKEY*> k = v.generate_keys(2000);
    size_t sing_len; 
    unsigned char sign[EVP_PKEY_size(k.first)];
    v.get_signature(k.first, blockch.block_to_string(block), sign, &sing_len);
     std::cout << "Signature HEX: ";
    for (size_t i = 0; i < sing_len; i++)
        std::printf("%02x", sign[i]);
    std::cout << std::endl;

    // cleanup
    EVP_PKEY_free(k.first);
    EVP_PKEY_free(k.second);
    return 0;


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