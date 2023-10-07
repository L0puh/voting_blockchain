#include "blockchain.h"
#include <cstring>
#include <sys/socket.h>

Net::Net(port_t port){
    int sockfd = init_socket(port);
    if (port == SERVICE_PORT) {
        //...
    } else {
        connect_service(port, sockfd);
    }
}

// NODE
void Net::connect_service(port_t port, int sockfd){
    int res;
    std::string buff  = node_addr + ":" + std::to_string(port);
    addr_t addr = init_addr(port);
    log_error(sendto(sockfd, buff.c_str(), sizeof(buff), 0, (const struct sockaddr*)&addr.their_addr, addr.size_addr));
}

int Net::init_socket(port_t port) {
    struct sockaddr_in servinfo;
    memset(&servinfo, 0, sizeof(servinfo));
    servinfo.sin_family = AF_INET; 
    servinfo.sin_port = htons(port);

    int sockfd = socket(servinfo.sin_family, SOCK_DGRAM, 0);
    log_error(bind(sockfd, (const struct sockaddr *)&servinfo, sizeof(servinfo)));
    return sockfd;
}

addr_t Net::init_addr(port_t port){
   struct sockaddr_in their_addr;
   memset(&their_addr, 0, sizeof(their_addr));
   their_addr.sin_family = AF_INET;
   their_addr.sin_port = htons(port);
   socklen_t size_addr = sizeof(their_addr);
   return addr_t{.size_addr = size_addr, .their_addr = their_addr};
}

// LOG

void Net::log_error(int result) {
    if (result == -1){
        printf("error: %s\n", strerror(errno));
        exit(-1);
    }
}
void Net::log(std::string message) {
    printf("%s\n", message.c_str());
}

