#include "block.h"

std::string Hash::get_nonce(std::string blockHash, uint8_t difficulty){
    std::string target = "";

    for (int i=0; i < difficulty; i++) {
        target += '0'; 
    }

    std::string nonceStr = "";
    unsigned char hash[SHA256_DIGEST_LENGTH];
    int nonce = 0;
    while (true) {
        nonceStr = std::to_string(nonce);
        std::string d = blockHash + nonceStr;
        SHA256((unsigned char*)d.c_str(), d.size(), hash);

        std::ostringstream s;
        for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
            s << std::hex << (int)hash[i];
        }
        std::string hashStr = s.str();
        if (hashStr.substr(0, difficulty) == target) {
            return nonceStr;
        }
        nonce++;
    }
}

std::string Hash::create_hash(std::string data) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256((unsigned char*)data.c_str(), data.size(), hash);
    std::ostringstream s;
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        s << std::hex << (int)hash[i];
    }
    std::string hashStr = s.str();
    return hashStr;
}

uint32_t Block::get_timestamp() {
    time_t t = time(0);
    return t;
}

std::string Block::block_to_string(Block_t block) {
    std::string data = block.header.prev_hash + 
        block.header.timestamp + (char)block.result;
    return data;
}

void Block::link_block(std::vector<Block_t> *blockchain, Block_t block) {
    blockchain->push_back(block);
}

Block_t Block::init_block(std::string prev_hash, uint8_t res) {
   block.header.prev_hash = prev_hash;
   block.result = res;
   block.header.timestamp = get_timestamp();
   block.header.hash = create_hash(block_to_string(block));
   block.header.nonce = get_nonce(block.header.hash, difficulty); 
   block.header.hash = create_hash(block.header.hash + block.header.nonce);
   return block;
}

Block::Block(){
    difficulty = 4;
}

Block::Block(uint8_t dif){
    difficulty = dif;
}
