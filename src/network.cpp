#include "blockchain.h"
#include <sys/socket.h>

void log_error(int result) {
    if (result == -1){
        printf("error: %s\n", strerror(errno));
        exit(-1);
    }
}
void log(std::string message) {
    printf("%s\n", message.c_str());
}

Net::Net(port_t port) {
    int sockfd = init_node(port);
    if (port == SERVICE_PORT) {
        init_service(sockfd); 
    } else {
        init_client(sockfd);
    } 

    std::thread recv_th(&Net::handle_recv, this, sockfd);
    recv_th.detach();

    handle_send(sockfd);
}

int Net::init_node(port_t port) {
    struct sockaddr_in servinfo;
    memset(&servinfo, 0, sizeof(servinfo));
    servinfo.sin_family = AF_INET; 
    servinfo.sin_port = htons(port);

    int sockfd = socket(servinfo.sin_family, SOCK_DGRAM, 0);
    log_error(bind(sockfd, (const struct sockaddr *)&servinfo, sizeof(servinfo)));

    return sockfd;
}

void Net::handle_recv(int sockfd){
    int bytes; 
    struct sockaddr_in their_addr;
    memset(&their_addr, 0, sizeof(their_addr));
    socklen_t size_addr = sizeof(their_addr);
    char buffer[32];
    while ((bytes = recvfrom(sockfd, buffer, 32, 0, (struct sockaddr*)&their_addr, &size_addr) != -1)) {
        log("connected\n");
        connections.push_back(their_addr.sin_port);
    }
}

void Net::handle_send(int sockfd) {
    Block block(2);
    std::string message;
    while(true){
        if (connections.size() > 0){
            for (auto itr = connections.begin(); itr != connections.end(); itr++) {
                broadcast(message, sockfd, *itr);
            }
        }     
    }
}

void Net::broadcast(std::string message, int sockfd, port_t port) {
   struct sockaddr_in their_addr;
   memset(&their_addr, 0, sizeof(their_addr));
   their_addr.sin_family = AF_INET;
   their_addr.sin_port = htons(port);
   socklen_t size_addr = sizeof(their_addr);

   if (sizeof(message) <= 32){
        log_error(sendto(sockfd, message.c_str(), 32, 0, (const struct sockaddr*)&their_addr, size_addr));
   }
}


void Net::init_service(int sockfd) {
    port_t port = handle_connection();
    connections.push_back(port);
}

void Net::init_client(int sockfd) {
    send(SERVICE_PORT, std::to_string(1)); //TODO
    std::string ports;
    recv(&ports); 
    std::vector<port_t> connections_table = ports_to_table(ports); 
}

port_t Net::handle_connection() {

}

std::vector<port_t> Net::ports_to_table(std::string ports) {

}

void Net::send(port_t port, std::string message) {

}

void Net::recv(std::string *message){

}
