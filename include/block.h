#ifndef BLOCK_H
#define BLOCK_H

#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <sstream>
#include <openssl/sha.h>
#include <cstring>
#include <ctime>


struct header_t {
    std::string prev_hash;
    std::string timestamp;
    std::string nonce; 
    std::string hash;
};

struct Block_t {
    header_t header;
    uint8_t result;
};

class Hash {
    public:
        std::string create_hash(std::string data);
        std::string get_nonce(std::string blockHash, uint8_t difficulty);
};

class Block : public Hash {  
    private:
        Block_t block; 
        uint8_t difficulty;
    public:
        Block(uint8_t difficulty);
        Block();
    public:
        std::string block_to_string(Block_t block);
    public:
        char* get_timestamp();
        void init_block(std::string prev_hash, uint8_t res);
        void link_block();
};



#endif 
