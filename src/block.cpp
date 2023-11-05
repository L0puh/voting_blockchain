#include "blockchain.h"

std::string pop_end(json str){
    std::string bl = str.dump(INDENT);
    bl.pop_back();
    return bl;
}


int Block::get_length(){
    std::string bl = blockchain.dump(INDENT);
    return bl.size();
}


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
    std::string data = 
        block.header.prev_hash +  block.header.nonce +
        block.header.hash + std::to_string(block.header.timestamp) + 
        std::to_string(block.result);
    return data;
}

json Block::block_to_json(Block_t block){
    json bl = {
        {"hash", block.header.hash},
        {"prev_hash", block.header.prev_hash},
        {"nonce", block.header.nonce},
        {"timestamp", block.header.timestamp},
        {"result", block.result}
    };
    return bl;
}

void Block::link_block(Block_t block) {
    std::string bl = "[";
    bl += blockchain.dump(INDENT);
    if (bl[bl.size()-1] == ',')
        bl = pop_end(blockchain);
    bl += ",\n" + block_to_json(block).dump(INDENT) + ']';
    blockchain = json::parse(bl);
}

json Block::get_blockchain(){
        return blockchain; 
}
Block_t Block::init_block(std::string prev_hash, uint8_t res) {
   block.result = res;
   
   header_t h{.prev_hash = prev_hash, 
              .timestamp = get_timestamp(), 
              .hash = create_hash(block_to_string(block))};
   block.header = h;
   block.header.nonce = get_nonce(block.header.hash, difficulty); 
   block.header.hash = create_hash(block.header.hash + block.header.nonce);
   return block;
}
Block_t Block::first_block() {
    Block_t b = init_block("0000000", -1);
    blockchain = block_to_json(b);
    return b;
    
}

Block::Block(){
    difficulty = 2;
}

Block::Block(uint8_t dif){
    difficulty = dif;
}

Block_t Block::json_to_block(json bl){
    Block_t block;
    bl.at("hash").get_to(block.header.hash);
    bl.at("prev_hash").get_to(block.header.prev_hash);
    bl.at("nonce").get_to(block.header.nonce);
    bl.at("timestamp").get_to(block.header.timestamp);
    bl.at("result").get_to(block.result);
    return block;
}

Block_t Block::get_last(json blockchain){
    json bl;
    if (blockchain.is_array()) {
        bl = blockchain[blockchain.size()-1];
    } else {
        bl = blockchain;
    }
    return json_to_block(bl);
}
