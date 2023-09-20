#ifndef BLOCK_H
#define BLOCK_H

#include <stdlib.h>
#include <stdio.h>
#include <string>
#include "libs/SHA256.h"

struct header_t {
    std::string prev_hash;
    std::string timestamp;
    int nonce; 
    std::string hash;
};

struct Block_t {
    header_t header;
    uint8_t result;
};

class Hash {
    SHA256 sha;
    public:
        std::string create_hash(std::string data);

};

class Block : public Hash {  
        Block_t block;
    public:
        void init_block();
        void link_block();
        // validate_block();
};



#endif 
