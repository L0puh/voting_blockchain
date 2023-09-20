#include "block.h"


std::string Hash::create_hash(std::string data) {
    sha.update(data);
    uint8_t *dg = sha.digest();
    return SHA256::toString(dg);
}

void Block::init_block() {
     
}

void Block::link_block() {

}


