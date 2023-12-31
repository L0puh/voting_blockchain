#include "blockchain.h"

namespace Net {
    template<typename T> 
    int send_to(int sockfd, addr_t addr, T *data, int data_size) {
        int res = sendto(sockfd, data, data_size, 0, 
                        (const struct sockaddr*)&addr.their_addr, 
                        addr.size_addr);
        return res; }

    template<typename T>
    int recv_from(int sockfd, addr_t *addr, T *data, int data_size) {
        int res = recvfrom(sockfd, data, data_size, 0, 
                          (struct sockaddr*)&addr->their_addr, 
                           &addr->size_addr);
        return res;
    }

    // SERVICE //
    void accept_connection(int sockfd){
        int bytes, req = OK;
        addr_t addr;
        char addr_str[20];
        std::string init_addr = node_addr + ":9100";

        memset(&addr.their_addr, 0, sizeof(addr.their_addr));
        addr.size_addr = sizeof(addr.their_addr);
        save_port(init_addr);
        log("* accept connections...");
        while ((bytes = recv_from(sockfd, &addr, &addr_str, 20)) != -1){
            std::string ports = get_ports();
            save_port(addr_str);
            if (bytes > 0) {
               int ports_size = ports.size();
               log_error(send_to(sockfd, addr, &req, sizeof(int)));
               log_error(send_to(sockfd, addr ,&ports_size, sizeof(int)));
               log_error(send_to(sockfd, addr, ports.c_str(), ports_size));
               log("sent ports");
            }
         }
    }

    std::string get_ports(){
        std::string ports;

        int count=0;
        for (auto& itr: connections){
            if (count == 0) 
                ports +=  itr.addr + ":" + std::to_string(itr.port);
            else 
                ports += separator + itr.addr + ":" + std::to_string(itr.port);
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

    void print_connections(){
        std::vector<conn_t>::iterator itr = connections.begin();
        log("connections:");
        for(; itr != connections.end(); itr++){
            printf("# %s : %d\n", itr->addr.c_str(), itr->port);
        }
    }

    int save_port(std::string addr_str) {
        conn_t addr = convert_addr(addr_str);
        if (connections.size() != 0) {
            for (auto& itr: connections) {
                if (itr.addr == addr.addr && itr.port == addr.port) {
                    log("addr already exists");
                    return 0;
                }
            }
        }
        connections.push_back(addr);
        return 0;
    }

    // NODE //


    void create_block(int sockfd,  int vote){
        log("create block...");

        Block_t bl = Block::get_last(Block::get_blockchain());
        Block_t block = Vote::vote(bl, vote); 

        std::pair<EVP_PKEY*, EVP_PKEY*> k = Vote::generate_keys(2000);
        unsigned char sign[EVP_PKEY_size(k.first)];
        size_t sign_len; 
        Vote::get_signature(k.first, Block::block_to_json(block).dump(INDENT), sign, &sign_len);
        BIO* bio = BIO_new(BIO_s_mem());
        log_error(PEM_write_bio_PUBKEY(bio, k.second));
        char* data;
        long key_len = BIO_get_mem_data(bio, &data);
        send_miner(sockfd, block, sign, sign_len, data, key_len);
        
        // cleanup
        EVP_PKEY_free(k.first);
        EVP_PKEY_free(k.second);
        log("done");
    }

    void connect_service(port_t port, int sockfd){
        int res, req;
        std::string buff  = node_addr + ":" + std::to_string(port);
        addr_t addr = init_addr(SERVICE_PORT);
        log_error(send_to(sockfd, addr, buff.c_str(), sizeof(buff)));
        log("* connect service...");
        while ((res = recv_from(sockfd, &addr, &req, sizeof(int)))){
            if (req == OK) {
                log("connected to the service");
                break;
            }
            else
                log_error(send_to(sockfd, addr, buff.c_str(), sizeof(buff)));
        }
    }


    void recv_request(int sockfd) {
        log("* wait requests...");
        int bytes, req, length;
        addr_t addr;

        memset(&addr.their_addr, 0, sizeof(addr.their_addr));
        addr.size_addr = sizeof(addr.their_addr);
            while((bytes = recv_from(sockfd, &addr, &req, sizeof(int))) != -1){
                std::string bl = Block::get_blockchain().dump(INDENT);
                switch(req) {
                    case LENGTH:
                    {
                        log("request: length");
                        length = Block::get_length();
                        log_error(send_to(sockfd, addr, &length, sizeof(int)));
                        log("sent:    length");
                        break;
                    }
                    case GET:
                    {
                        log("request: get");
                        if (bl.length() == Block::get_length()){
                            log_error(send_to(sockfd, addr, bl.c_str(), bl.length()));
                        }
                        log("sent:    blockchain");
                        break;
                    }
                    case SEND:
                    {
                        log("get new block");
                        //TODO: verify and get block -> make blockchain
                        break;
                    }
                    case NOT_VALID:
                    case VALID:
                    {
                        printf("blockchain is %d\n", req);
                        break;
                    }
                    default:
                        log("get unknown request");
                }
            } 
            log_error(bytes);
        }

        int recv_length(int sockfd, addr_t *tr_addr){
            int length = 0, req = LENGTH, count=0;
            std::vector<conn_t>::iterator itr = connections.begin();
            itr++; //skip init ports
            struct sockaddr_in trusted;
            for (; itr != connections.end(); itr++){
                addr_t addr = init_addr(itr->port);
                addr.size_addr = sizeof(addr.their_addr);
                log_error(send_to(sockfd, addr, &req, sizeof(int)));
                log_error(recv_from(sockfd, &addr, &length, sizeof(int)));
            printf("connected to %hu\nlength: %d\n", itr->port, length);
            if (length > count) {
                count = length;
                trusted = addr.their_addr;
            }
            log("found trusted chain");
        }
        tr_addr->their_addr = trusted;
        return length;
    }

    json recv_blockchain(int sockfd){
        addr_t addr;
        addr.size_addr = sizeof(addr.their_addr);
        log("recv blockchain...");

        if (connections.size()  <= 1)  {
            log("no nodes found");
            return json{nullptr}; 
        }

        int length = recv_length(sockfd, &addr);
        int req = GET;
        char *buff = new char[length+1];
        log_error(send_to(sockfd, addr, &req, sizeof(int)));
        log_error(recv_from(sockfd, &addr, buff, length));
        buff[length] = '\0';
        log("done");
        return json::parse(buff);
    }

    std::string recv_ports(int sockfd){
        addr_t addr = init_addr(SERVICE_PORT);
        int bytes, ports_size;
        while((bytes = recv_from(sockfd, &addr, &ports_size, sizeof(int))) != -1) {
            char* buff = new char[ports_size+1];
            log_error(recv_from(sockfd, &addr, buff, ports_size));
            buff[ports_size] = '\0';
            log("recv ports");
            return buff;
        }
        log("no ports recv");
        return "";
    }

    conn_t convert_addr(std::string addr_str) {
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

    void convert_ports(std::string ports){
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

    void send_miner(int sockfd, Block_t block, unsigned char* sign, size_t sign_len, char* key, long key_len){
        addr_t addr = init_addr(MINER_PORT);

        std::string bl_str = Block::block_to_json(block).dump(INDENT);
        int size_bl = bl_str.length(), req = BLOCK;

        send_to(sockfd, addr, &req, sizeof(int));
        send_to(sockfd, addr, &size_bl, sizeof(int));
        send_to(sockfd, addr, bl_str.c_str(), bl_str.length());
        log("sent block");
        req = SIGN;

        send_to(sockfd, addr, &req, sizeof(int));
        send_to(sockfd, addr, &sign_len, sizeof(int));


        send_to(sockfd, addr, sign, sign_len);
        send_to(sockfd, addr, &key_len, sizeof(long));
        send_to(sockfd, addr, key, key_len);
        printf("sent key size:%ld\nsign_size:%zu\nblock size:%zu\n", key_len, sign_len, bl_str.length());
        log("sent sign and key");
    }
    // MINER //


    std::string recv_block(int sockfd, addr_t *user){
        log("* recv block...");
        addr_t addr;
        memset(&addr.their_addr, 0, sizeof(addr.their_addr));
        addr.size_addr = sizeof(addr.their_addr);
        int bytes, req, block_size;
        while((bytes = recv_from(sockfd, &addr, &req, sizeof(int))) !=-1) {
            if ( req == BLOCK ){
                log_error(recv_from(sockfd, &addr, &block_size, sizeof(int)));
                char* block = new char[block_size];
                log_error(recv_from(sockfd, &addr, block, block_size));
                log("recv block");
                printf("block[%d]: %s\n", block_size, block);

                *user = addr;
                return block;
            }
        }
        return "";
    }

    std::pair<unsigned char*, EVP_PKEY*> recv_sign(int sockfd, size_t *len_sign) {
        addr_t addr;
        memset(&addr.their_addr, 0, addr.size_addr);
        addr.size_addr = sizeof(addr.their_addr);

        int bytes, req, size_sign; long key_size;
        while((bytes = recv_from(sockfd, &addr, &req, sizeof(int))) != -1){
            if ( req == SIGN ){
                log_error(recv_from(sockfd, &addr, &size_sign, sizeof(int)));
                unsigned char* sign = new unsigned char[size_sign];
                log_error(recv_from(sockfd, &addr, sign, size_sign));
                log("recv sign");
                

                log_error(recv_from(sockfd, &addr, &key_size, sizeof(long)));
                char* buff = new char[key_size];
                log_error(recv_from(sockfd, &addr, buff, key_size));

                BIO* bio = BIO_new_mem_buf(buff, key_size);
                *len_sign = size_sign;
                printf("sign[%d]: %s\nkey[%ld]:%s\n", size_sign, sign, key_size, buff);
                EVP_PKEY* key = PEM_read_bio_PUBKEY(bio, NULL, NULL, NULL);
                log("recv key");
                return std::make_pair(sign, key);
            }
        }
        return std::make_pair(nullptr, nullptr);
    }

    Block_t proof_of_work(std::string block) {
        Block_t bl = Block::json_to_block(json::parse(block));
        return Block::init_block(bl.header.prev_hash, bl.result);
    }


    int commit_block(int sockfd, Block_t block){
        int req = SEND, res;
        std::vector<int> results;
        Block::link_block(block);
        std::string bl = Block::get_blockchain().dump(INDENT);
        addr_t addr;
        for(auto& i: connections){
            addr = init_addr(i.port);
            log_error(send_to(sockfd, addr, &req, sizeof(int)));
            log_error(send_to(sockfd, addr, bl.c_str(), sizeof(bl)));
            log_error(recv_from(sockfd, &addr, &res, sizeof(int)));
            results.push_back(res);
        }
        res = 0;
        for (auto& i: results){
            res += i;
        }
        return res;
    }

    void send_response(int sockfd, int res, addr_t *addr){
        int req=NOT_VALID;
        if (res == 0){
            log_error(send_to(sockfd, *addr,  &req, sizeof(int) ));
            return;
        }
        req = VALID;
        if ( res == connections.size() or res > connections.size()/2){
            log_error(send_to(sockfd, *addr, &req, sizeof(int)));
        } 
    }

    // init 

    int init_socket(port_t port) {
        struct sockaddr_in servinfo;
        memset(&servinfo, 0, sizeof(servinfo));
        servinfo.sin_family = AF_INET; 
        servinfo.sin_port = htons(port);
        int sockfd = socket(servinfo.sin_family, SOCK_DGRAM, 0);
        log_error(bind(sockfd, (const struct sockaddr *)&servinfo, sizeof(servinfo)));
        return sockfd;
    }

    addr_t init_addr(port_t port){
       struct sockaddr_in their_addr;
       memset(&their_addr, 0, sizeof(their_addr));
       their_addr.sin_family = AF_INET;
       their_addr.sin_port = htons(port);
       their_addr.sin_addr.s_addr = INADDR_ANY;
       socklen_t size_addr = sizeof(their_addr);
       return addr_t{.size_addr = size_addr, .their_addr = their_addr};
    }

}
// LOG

void log_error(int result) {
    if (result == -1){
        printf("error: %s\n", strerror(errno));
        exit(-1);
    }
}
void log(std::string message) {
    printf("%s\n", message.c_str());
}

