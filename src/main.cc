#include "flow.hpp"
#include "posix.hpp"
#include "raw_stack.hpp"

#include <cstdint>
#include <cstring>

#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

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
        auto read_size = stack_readfrom(fd, buf, udp_buf_size, (struct sockaddr*)&remote_addr, &sock_len);
        if (read_size < 0) {
            std::cout << "user read from stack failed" << std::endl;
            return;
        }
        std::cout << "user read from stack success, buf: " << std::string(buf, read_size) << ", size: " << read_size << std::endl;
        auto write_size = stack_writeto(fd, buf, read_size, (struct sockaddr*)&remote_addr, sock_len);
        if (write_size < 0) {
            std::cout << "user write to stack failed" << std::endl;
            return;
        }
        std::cout << "user write to stack success, buf: " << std::string(buf, write_size) << ", size: " << read_size << std::endl;
    }
}

void tcp_server() {
    // create fd 
    int fd = sock_create(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (fd == -1) {
        std::cout << "create fd failed" << std::endl;
        return;
    }
    // create sock addr
    struct sockaddr_in local_addr;
    memset(&local_addr, 0, sizeof(struct sockaddr_in));
    local_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    local_addr.sin_port = htons(udp_listen_port);
    // bind socket 
    if (stack_bind(fd, (struct sockaddr*)&local_addr, sizeof(struct sockaddr_in)) == -1) {
        std::cout << "bind fd failed" << std::endl;
        return;
    }
    // listen socket
    if (stack_listen(fd, 10) == -1) {
        std::cout << "listen fd failed" << std::endl;
        return;
    }
    // accept socket 
    struct sockaddr_in remote_addr;
    memset(&remote_addr, 0, sizeof(struct sockaddr_in));
    socklen_t remote_len = 0;
    int accept_fd = stack_accept(fd, (struct sockaddr*)&remote_addr, &remote_len);
    std::cout << "accept sock success, fd: " << accept_fd 
        << ", addr: " << utils::generic::format_ip_address(ntohl(remote_addr.sin_addr.s_addr))
        << ":" << ntohs(remote_addr.sin_port) << std::endl;
    // create read item
    char buf[udp_buf_size];
    while (true) {
        auto read_size = stack_read(accept_fd, buf, udp_buf_size);
        std::cout << "user tcp read from stack success, buf: " << std::string(buf, read_size) << ", size: " << read_size << std::endl;
        auto write_size = stack_write(accept_fd, buf, read_size);
        std::cout << "user tcp write from stack success, buf: " << std::string(buf, read_size) << ", size: " << write_size << std::endl;
    }
}

void tcp_client() {
    sleep(10);
    // create fd 
    int fd = sock_create(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (fd == -1) {
        std::cout << "create fd failed" << std::endl;
        return;
    }
    // create sock addr
    struct sockaddr_in remote_addr;
    memset(&remote_addr, 0, sizeof(struct sockaddr_in));
    if (inet_pton(AF_INET, "172.17.0.2", &remote_addr.sin_addr.s_addr) == -1) {
        std::cout << " convert remote addr failed" << std::endl;
        return;
    }
    remote_addr.sin_port = htons(udp_listen_port);
    // try to connect
    if (stack_connect(fd, (struct sockaddr*)&remote_addr, sizeof(struct sockaddr_in)) == -1) {
        std::cout << "connect to remote failed" << std::endl;
    }
    std::cout << "connect to remote success" << std::endl;
    char buf[] = "hello!";
    while (true) {
        stack_write(fd, buf, strlen(buf));
        std::cout << "writ to remote success" << std::endl;
        sleep(5);
    }
}

int main() {
    auto stack = stack::raw_stack::get_instance();
    stack->init();
    stack->run();

    // udp_server();
    // tcp_server();
    tcp_client();

    stack->wait();
}