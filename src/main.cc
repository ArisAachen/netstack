#include "posix.hpp"
#include "raw_stack.hpp"

#include <cstdint>
#include <cstring>

#include <netinet/in.h>
#include <sys/socket.h>

const uint16_t udp_listen_port = 8888;
const uint16_t udp_buf_size = 512;

// add udp server
void udp_server() {
    // create fd 
    int fd = sock_create(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (fd == -1) {
        std::cout << "create fd failed" << std::endl;
        return;
    }
    // create sock addr
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(struct sockaddr_in));
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(udp_listen_port);
    // bind socket 
    if (stack_bind(fd, (struct sockaddr*)&addr, sizeof(struct sockaddr_in)) == -1) {
        std::cout << "bind fd failed" << std::endl;
        return;
    }
    // create read item
    char buf[udp_buf_size];
    struct sockaddr_in remote_addr;
    memset(&remote_addr, 0, sizeof(struct sockaddr_in));
    socklen_t sock_len = 0;
    while (true) {
        auto size = stack_readfrom(fd, buf, udp_buf_size, (struct sockaddr*)&remote_addr, &sock_len);
        if (size < 0) {
            std::cout << "user read from stack failed" << std::endl;
            return;
        }
        std::cout << "user read from stack success, buf: " << std::string(buf, size) << ", size: " << size << std::endl;
    }
}

int main() {
    auto stack = stack::raw_stack::get_instance();
    stack->init();
    stack->run();

    udp_server();

    stack->wait();
}