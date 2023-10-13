#include "blockchain.h"
#include <cstring>
#include <sys/socket.h>
#include <vector>

Net::Net(port_t port){
    int sockfd = init_socket(port);
    if (port == SERVICE_PORT) {
        std::vector<conn_t> ports;
        log("detected service port");
        accept_connection(ports, sockfd);
    } else {
        log("detected client port");
        connect_service(port, sockfd);
        get_ports(sockfd);
    }
}

// SERVICE
void Net::accept_connection(std::vector<conn_t> ports, int sockfd){
    int bytes;

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    socklen_t addr_size = sizeof(addr);

    char addr_str[300]; //FIXME
    log("waiting to recieve..");
    while((bytes = recvfrom(sockfd, &addr_str, PORTS_SIZE, 0, (struct sockaddr*)&addr, &addr_size)) != -1) {
        printf("new connection from %s\n", addr_str);
        save_port(ports, addr_str);
        if (bytes > 0) {
           //TODO
           log_error(sendto(sockfd, "2312412412", 300, 0, (struct sockaddr*)&addr, addr_size));
        }
     }
}

conn_t Net::convert_addr(std::string addr_str) {
    std::string addr, port;
    size_t pos = addr_str.find(":");
    if (pos != std::string::npos) {
        addr = addr_str.substr(0, pos);
        port = addr_str.substr(pos + 1);
    }
    int p = stoi(port);
    port_t portn = static_cast<port_t>(p);
    return conn_t{.addr = addr, .port = portn};
}

int Net::save_port(std::vector<conn_t> ports, std::string addr_str) {
    std::vector<conn_t>::iterator itr = ports.begin();
    conn_t addr = convert_addr(addr_str);
    if (ports.size() != 0) {
        for (; itr != ports.end(); itr++) {
            if (itr->addr == addr.addr && itr->port == addr.port) {
                log("addr already exists");
                return 0;
            }
        }
    }
    
    ports.push_back(addr);
    return 0;
}
// NODE
void Net::connect_service(port_t port, int sockfd){
    int res;
    std::string buff  = node_addr + ":" + std::to_string(port);
    addr_t addr = init_addr(SERVICE_PORT);
    log_error(sendto(sockfd, buff.c_str(), sizeof(buff), 0, (const struct sockaddr*)&addr.their_addr, addr.size_addr));
    log("connected to the service");
}

void Net::get_ports(int sockfd){
    addr_t addr = init_addr(SERVICE_PORT);
    int bytes;
    char ports[PORTS_SIZE];
    log("recieving...");
    while((bytes = recvfrom(sockfd, &ports, PORTS_SIZE, 0, (struct sockaddr *)&addr.their_addr, &addr.size_addr)) != -1) {
        if (bytes > 0) {
            //TODO
        }
    }
}

// init 

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
   their_addr.sin_addr.s_addr = INADDR_ANY;
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

