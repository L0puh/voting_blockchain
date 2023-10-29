#include "blockchain.h"


Net::Net(port_t port){
    int sockfd = init_socket(port);
    if (port >= SERVICE_PORT) {
        log("detected service port");
        accept_connection(sockfd);
    } else { 

        std::thread th(&Net::recv_request, this, sockfd);

        log("detected client port");
        connect_service(port, sockfd);
        std::string ports = recv_ports(sockfd); 
        convert_ports(ports);
        print_connections();    
        recv_blockchain(sockfd);

        th.detach();
        while(true){
            //FIXME
        }

    }

}

// SERVICE
void Net::accept_connection(int sockfd){
    int bytes;

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    socklen_t addr_size = sizeof(addr);
    std::string init_addr = node_addr + ":9100";
    save_port(init_addr);
    char addr_str[20];
    while((bytes = recvfrom(sockfd, &addr_str, 20, 0, (struct sockaddr*)&addr, &addr_size)) != -1) {
        printf("new connection from %s\n", addr_str);
        std::string ports = get_ports();
        save_port(addr_str);
        printf("# ports: %s\n", ports.c_str());
        if (bytes > 0) {
           int ports_size = ports.size();
           log_error(sendto(sockfd, &ports_size, sizeof(int), 0, (struct sockaddr*)&addr, addr_size));
           log_error(sendto(sockfd, ports.c_str(), ports_size, 0, (struct sockaddr*)&addr, addr_size));
           log("sent ports");
        }
     }
}

std::string Net::get_ports(){
    std::string ports;
    std::vector<conn_t>::iterator itr = connections.begin();
    int count=0;
    for (; itr != connections.end(); itr++){
        if (count == 0) 
            ports +=  itr->addr + ":" + std::to_string(itr->port);
        else 
            ports += separator + itr->addr + ":" + std::to_string(itr->port);
        count++;
    }
    return ports;
}
int count_addr(std::string addr) {
    int count = 0;
    for (const char c: addr) {
        if (c == separator) {
            return count;
        }
        count++;
    }
    return 0;
}

void Net::print_connections(){
    std::vector<conn_t>::iterator itr = connections.begin();
    log("connections:");
    for(; itr != connections.end(); itr++){
        printf("# %s : %d\n", itr->addr.c_str(), itr->port);
    }
}

int Net::save_port(std::string addr_str) {
    std::vector<conn_t>::iterator itr = connections.begin();
    conn_t addr = convert_addr(addr_str);
    if (connections.size() != 0) {
        for (; itr != connections.end(); itr++) {
            if (itr->addr == addr.addr && itr->port == addr.port) {
                log("addr already exists");
                return 0;
            }
        }
    }
    connections.push_back(addr);
    log("connection saved");
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


void Net::recv_request(int sockfd) {
    log("* wait requests...");
    int bytes, req;
    addr_t addr;
    memset(&addr.their_addr, 0, addr.size_addr);
    while((bytes = recvfrom(sockfd, &req, sizeof(int), 0,\
                (struct sockaddr*)&addr.their_addr, &addr.size_addr )) != -1){
        json block = get_blockchain();
        std::string bl = block.dump(INDENT);
        printf("# blockchain: %s\n\n", bl.c_str());
        if (req == LENGTH) {
            log("request: length");
            int length = get_length();
            printf("# length: %d\n", length);
            log_error(sendto(sockfd, &length, sizeof(int), 0,\
                        (const struct sockaddr *)&addr.their_addr, addr.size_addr));
        } else if (req == GET) {
            log("request: get");
            log_error(sendto(sockfd, bl.c_str(), get_length(), 0,\
                        (const struct sockaddr*)&addr.their_addr, addr.size_addr));
        }
    } 
    log_error(bytes);

}

json Net::recv_blockchain(int sockfd){
    log("recv blockchain...");
    int length = 0, req = LENGTH, count=0;
    if (connections.size()  <= 2)  {
        log("no nodes found");
        json empty;
        return empty; 
    }
    std::vector<conn_t>::iterator itr = connections.begin();
    itr++; //skip init ports
    struct sockaddr_in trusted;
    for (; itr != connections.end(); itr++){
        struct sockaddr_in addr;
        memset(&addr, 0, sizeof(addr));
        /* addr.sin_addr.s_addr = inet_addr((const char*)&itr->addr); */
        addr.sin_port        = htons(itr->port);
        addr.sin_family      = AF_INET;
        socklen_t addr_size = sizeof(addr);
        log_error(sendto(sockfd, &req, sizeof(int), 0, (const struct sockaddr*)&addr, addr_size));
        log_error(recvfrom(sockfd, &length, sizeof(int), 0, (struct sockaddr*)&addr, &addr_size));
        printf("connected to %hu\n | length: %d\n", itr->port, length);
        if (length > count) {
            count = length;
            trusted = addr;
        }
        log("found trusted chain");
    }
    req = GET;
    socklen_t addr_size = sizeof(trusted);
    char *blockchain = new char[length+1];
    log_error(sendto(sockfd, &req, sizeof(int), 0, (const struct sockaddr*)&trusted, addr_size ));
    log_error(recvfrom(sockfd, &blockchain, sizeof(count), 0, (struct sockaddr*)&trusted, &addr_size));
    log("received blockchain");
    blockchain[length] = '\0';
    return json::parse(blockchain);
}

std::string Net::recv_ports(int sockfd){
    addr_t addr = init_addr(SERVICE_PORT);
    int bytes;
    int ports_size;
    while((bytes = recvfrom(sockfd, &ports_size, sizeof(int), 0, (struct sockaddr *)&addr.their_addr, &addr.size_addr)) != -1) {
        if (bytes > 0) {
            char* buff = new char[ports_size+1];
            log_error(recvfrom(sockfd, buff, ports_size, 0, (struct sockaddr *)&addr.their_addr, &addr.size_addr));
            log("received ports");
            buff[ports_size] = '\0';
            printf("ports: %s\n", buff);
            return buff;
        }
    }
    log("no ports recv");
    return "";
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

void Net::convert_ports(std::string ports){
    // ADDR:PORT_ADDR:PORT_...
    int count = 1, c;
    for (const char c: ports) {
        if (c == separator)
            count++;
    }
    log("converting ports...");
    for (; count != 1; count--) {
        if (ports.length() != 0) {
            c = count_addr(ports);
            save_port(ports.substr(0, c));
            ports.erase(0, c+1);
        }
    }
    save_port(ports);
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

