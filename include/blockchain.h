#ifndef BLOCKCHAIN_H
#define BLOCKCHAIN_H


//block
#include <stdio.h>
#include <stdlib.h>
#include <sstream>
#include <string>
#include <openssl/sha.h>
#include <cstring>
#include <ctime>
#include <vector>

//net
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>
#include <thread>


#define SERVICE_PORT 9000

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

struct Conn_t {
    socklen_t size_addr;
    struct sockaddr_in their_addr;
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
        std::vector<Block_t> blockchain;
    public:
        Block(uint8_t difficulty);
        Block();
    public:
        std::string block_to_string(Block_t block);
    public:
        uint32_t get_timestamp();
        Block_t init_block(std::string prev_hash, uint8_t res);
        void link_block(Block_t block);
        Block_t first_block();
        std::vector<Block_t> get_blockchain();

};

class Net : public Hash { 
    std::vector<port_t> connections;
    public:
        Net(port_t port);
    private: 
        int init_node(port_t port);
        void handle_recv(int sockfd);
        void handle_send(int sockfd);
        void recv_ports(std::string *message);
        void send(int sockfd, port_t port, std::string message); 
        std::vector<port_t> ports_to_table(std::string ports);
        void handle_connection(int sockfd);
        void init_client(int sockfd);
        Conn_t init_addr();
        void init_service(int sockfd);
        std::string table_to_ports(std::vector<port_t> table); 

};
#endif 
