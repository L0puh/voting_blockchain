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


// const 
#define PORTS_SIZE 300
#define SERVICE_PORT 9000
#define ADDR_SIZE 11 
#define PORT_SIZE 4
const std::string node_addr =  "198.16.0.0.18";
const char separator= '_';

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
    std::vector<conn_t> connections;
    public:
        Net(port_t port);
        int init_socket(port_t port);
        addr_t init_addr(port_t port);
        conn_t convert_addr(std::string addr_str);
    private:
        void log_error(int result);
        void log(std::string message);
        void print_connections();
    private: 
        //  node
        void connect_service(port_t port, int sockfd);
        void get_ports(char* ports[PORTS_SIZE], int sockfd);
        void convert_ports(std::string ports);

    private:
        // service 
        void accept_connection(int sockfd);
        int save_port(std::string addr_str);

    private:
        // miner

};
#endif 
