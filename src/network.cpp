#include "blockchain.h"
#include <sys/socket.h>
#include <thread>

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
    Conn_t addr = init_addr();
    
    char buffer[32];
    while ((bytes = recvfrom(sockfd, buffer, 32, 0, (struct sockaddr*)&addr.their_addr, &addr.size_addr) != -1)) {
        log("connected\n");
        connections.push_back(addr.their_addr.sin_port);
    }
}

void Net::handle_send(int sockfd) {
    Block block(2);
    std::string message;
    while(true){
        if (connections.size() > 0){
            for (auto itr = connections.begin(); itr != connections.end(); itr++) {
                send(sockfd,*itr, message);
            }
        }     
    }
}

void Net::send(int sockfd, port_t port, std::string message) {
   Conn_t addr = init_addr();
   struct sockaddr_in their_addr = addr.their_addr;
   their_addr.sin_port = htons(port);
   socklen_t addr_size = sizeof(their_addr);
   if (sizeof(message) <= 32){
        log_error(sendto(sockfd, message.c_str(), 32, 0, (const struct sockaddr*)&their_addr, addr_size));
   }
}


void Net::init_service(int sockfd) {
    handle_connection(sockfd);
}

void Net::init_client(int sockfd) {
    send(sockfd, SERVICE_PORT, std::to_string(1)); //TODO
    std::string ports;
    recv_ports(&ports); 
    std::vector<port_t> connections_table = ports_to_table(ports); 
}

Conn_t Net::init_addr(){
   struct sockaddr_in their_addr;
   memset(&their_addr, 0, sizeof(their_addr));
   their_addr.sin_family = AF_INET;
   socklen_t size_addr = sizeof(their_addr);
   return Conn_t{.size_addr = size_addr, .their_addr = their_addr};
}

void Net::handle_connection(int sockfd) {
    Conn_t addr = init_addr();
    std::thread th(&Net::handle_recv, this, sockfd);
    th.detach();
    // handle_send()
    // send the table of ports to new user. 
    // and send new port to all users as well.
}

std::string Net::table_to_ports(std::vector<port_t> table) {
    std::string ports = "";
    //TODO serealization from string ports to vector table
    return ports;
}
std::vector<port_t> Net::ports_to_table(std::string ports) {
    std::vector<port_t> table;
    //TODO serealization from string ports to vector table
    return table; 
}


void Net::recv_ports(std::string *message){
    // recv ports from service node
}
