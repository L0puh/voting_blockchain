#include "blockchain.h"

void miner(int sockfd){

    log("detected miner port");
    json blockchain = recv_blockchain(sockfd); 
    
    addr_t addr;
    std::string block = recv_block(sockfd, &addr); 
    size_t len_sign;
    std::pair<unsigned char*, EVP_PKEY*> sign = recv_sign(sockfd, &len_sign);
    bool res = Vote::verify(block, sign.first, len_sign, sign.second), res2 = true;
    std::string str_bl = blockchain.dump(INDENT);
    if (blockchain.is_null()) {
        res2 = check_block(blockchain, json::parse(block));
    }
    if (res and res2){
        log("signature is valid");
        Block_t bl = proof_of_work(block);
        int res = commit_block(sockfd, bl);
        send_response(sockfd, res, &addr);
    } else 
        log("signature is not valid");
}

void service(int sockfd){
    log("detected service port");
    accept_connection(sockfd);
}

void node(int sockfd, port_t port){
    int res = get_vote();
    

    log("detected client port");
    connect_service(port, sockfd);
    std::string ports = recv_ports(sockfd); 
    convert_ports(ports);
    json blockchain = recv_blockchain(sockfd);
    create_block(sockfd, res);
    std::thread th(recv_request, sockfd);
    th.detach();
    while(true){
        //FIXME
    }
}

std::string pop_end(json str){
    std::string bl = str.dump(INDENT);
    bl.pop_back();
    return bl;
}

int get_length(){
    std::string bl = blockchain.dump(INDENT);
    return bl.size();
}


std::string get_nonce(std::string blockHash, uint8_t DIFFICULTY){
    std::string target = "";

    for (int i=0; i < DIFFICULTY; i++) {
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
        if (hashStr.substr(0, DIFFICULTY) == target) {
            return nonceStr;
        }
        nonce++;
    }
}

std::string create_hash(std::string data) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256((unsigned char*)data.c_str(), data.size(), hash);
    std::ostringstream s;
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        s << std::hex << (int)hash[i];
    }
    std::string hashStr = s.str();
    return hashStr;
}

uint32_t get_timestamp() {
    time_t t = time(0);
    return t;
}

std::string block_to_string(Block_t block) {
    std::string data = 
        block.header.prev_hash +  block.header.nonce +
        block.header.hash + std::to_string(block.header.timestamp) + 
        std::to_string(block.result);
    return data;
}

json block_to_json(Block_t block){
    json bl = {
        {"hash", block.header.hash},
        {"prev_hash", block.header.prev_hash},
        {"nonce", block.header.nonce},
        {"timestamp", block.header.timestamp},
        {"result", block.result}
    };
    return bl;
}

void link_block(Block_t block) {
    std::string bl = "[";
    bl += blockchain.dump(INDENT);
    if (bl[bl.size()-1] == ',')
        bl = pop_end(blockchain);
    bl += ",\n" + block_to_json(block).dump(INDENT) + ']';
    blockchain = json::parse(bl);
}

json get_blockchain(){
        return blockchain; 
}
Block_t init_block(std::string prev_hash, uint8_t res) {
   Block_t block;
   block.result = res;
   
   header_t h{.prev_hash = prev_hash, 
              .timestamp = get_timestamp(), 
              .hash = create_hash(block_to_string(block))};
   block.header = h;
   block.header.nonce = get_nonce(block.header.hash, DIFFICULTY); 
   block.header.hash = create_hash(block.header.hash + block.header.nonce);
   return block;
}
Block_t first_block() {
    Block_t b = init_block("0000000", -1);
    blockchain = block_to_json(b);
    return b;
    
}

Block_t json_to_block(json bl){
    Block_t block;
    bl.at("hash").get_to(block.header.hash);
    bl.at("prev_hash").get_to(block.header.prev_hash);
    bl.at("nonce").get_to(block.header.nonce);
    bl.at("timestamp").get_to(block.header.timestamp);
    bl.at("result").get_to(block.result);
    return block;
}

Block_t get_last(json blockchain){
    json bl;
    if (blockchain.is_array()) {
        bl = blockchain[blockchain.size()-1];
    } else {
        bl = blockchain;
    }
    return json_to_block(bl);
}


bool check_block(json bchain, json block){
    if (block["hash"] != "0" and bchain.is_array()){
        for (int i=0; i < bchain.size(); i++){
            json b = bchain[i];
            if (b["hash"] == block["prev_hash"]){
                return true;
            }
        }
    } else if (bchain["hash"] == block["prev_hash"] ){
        return true;
    }
    return false;
}

