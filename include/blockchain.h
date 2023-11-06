#ifndef BLOCKCHAIN_H
#define BLOCKCHAIN_H

//block
#include <stdio.h>
#include <stdlib.h>
#include <sstream>
#include <string>
#include <openssl/evp.h>
#include <openssl/sha.h>
#include <openssl/pem.h>
#include <openssl/bio.h>
#include <openssl/rsa.h>
#include <cstring>
#include <ctime>
#include <utility>
#include <vector>
#include <nlohmann/json.hpp>

//net
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>
#include <thread>


using json = nlohmann::json;

// const 
#define SERVICE_PORT 9000
#define MINER_PORT 8000
#define ADDR_SIZE 11 
#define PORT_SIZE 4
#define INDENT 4
static const std::string node_addr =  "198.16.0.0.18";
static const char separator= '_';

enum req {
    LENGTH = 0,
    GET,  
    VERIFY,
    SIGN,
    BLOCK,
    SEND,
    OK,
    VALID,
    NOT_VALID
};

typedef uint16_t port_t;

struct header_t {
    std::string prev_hash;
    uint32_t timestamp;
    std::string nonce; 
    std::string hash;
};

struct Block_t {
    header_t header;
    uint8_t result;
};

struct addr_t {
    socklen_t size_addr;
    struct sockaddr_in their_addr;
};

struct conn_t {
    std::string addr;
    port_t port; 
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
        json blockchain;

    public:
        Block(uint8_t difficulty);
        Block();
    public:
        std::string block_to_string(Block_t block);
        json block_to_json(Block_t block);
        Block_t json_to_block(json bl);
    public:
        static uint32_t get_timestamp();
        Block_t init_block(std::string prev_hash, uint8_t res);
        void link_block(Block_t block);
        Block_t first_block();
    public:
        json get_blockchain();
        int get_length();
        Block_t get_last(json blockchain);

};
class Vote : public Block {
    public:
        Vote(json blockchain, uint8_t res);
        Vote();
    public: 
        static std::pair<EVP_MD_CTX*, EVP_PKEY_CTX*> init_ctx(EVP_PKEY* key, int type);
        static Block_t vote(Block_t last_block, uint8_t res);
        static std::pair<EVP_PKEY*, EVP_PKEY*> generate_keys(int length);
        static void get_signature(EVP_PKEY* sKey, std::string block, unsigned char* sign, size_t* len);
        static bool verify(std::string block, unsigned char* sign, size_t len, EVP_PKEY* pKey);

};

class Net : public Block { 
    std::vector<conn_t> connections;
    public:
        Net(port_t port, Block *block, int res);
        int init_socket(port_t port);

        template<typename T>
        int send_to(int sockfd, addr_t addr, T *data, int data_size);
        template<typename T>
        int recv_from(int sockfd, addr_t addr, T *data, int data_size);

        addr_t init_addr(port_t port);
        conn_t convert_addr(std::string addr_str);
    public:
        static void log_error(int result);
        static void log(std::string message);
        void print_connections();
    private: 
        //  node
        void connect_service(port_t port, int sockfd);
        std::string recv_ports(int sockfd);
        void convert_ports(std::string ports);
        json recv_blockchain(int sockfd);
        void recv_request(int sockfd, Block *block);
        std::string get_ports();
        int recv_length(int sockfd, addr_t *tr_addr);
        void create_block(int sockfd, Block *block, int vote);
        void send_miner(int sockfd, Block_t block, unsigned char* sign, size_t sign_len, char* key, long key_len);
    private:
        // service 
        void accept_connection(int sockfd);
        int save_port(std::string addr_str);
    private:
        // miner
        std::string recv_block(int sockfd, addr_t *addr);
        std::pair<unsigned char*, EVP_PKEY*> recv_sign(int sockfd, size_t *len_sign);
        Block_t proof_of_work(std::string block);
        int commit_block(int sockfd, Block_t block);
        void send_response(int sockfd, int res, addr_t *addr);

};

#endif 
