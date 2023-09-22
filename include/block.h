#ifndef BLOCK_H
#define BLOCK_H

#include <stdio.h>
#include <stdlib.h>
#include <sstream>
#include <string>
#include <openssl/sha.h>
#include <cstring>
#include <ctime>
#include <vector>


struct header_t {
    std::string prev_hash;
    std::string timestamp;
    std::string nonce; 
    std::string hash;
};

struct Block_t {
    struct header_t header;
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
        uint32_t get_timestamp();
        Block_t init_block(std::string prev_hash, uint8_t res);
        void link_block(std::vector<Block_t> *blockchain, Block_t block);
};

#endif 
