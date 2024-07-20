#include "posix.hpp"
#include "raw_stack.hpp"


int sock_create(int domain, int type, int protocol) {
    return stack::raw_stack::get_instance()->sock_create(domain, type, protocol);
}


int stack_connect(uint32_t fd, struct sockaddr* addr, socklen_t len) {
    if (stack::raw_stack::get_instance()->connect(fd, addr, len))
        return 0;
    return -1;
}

int stack_close(uint32_t fd) {
    if (stack::raw_stack::get_instance()->close(fd))
        return 0;
    return -1;
}

int stack_bind(uint32_t fd, struct sockaddr* addr, socklen_t len) {
    if (stack::raw_stack::get_instance()->bind(fd, addr, len))
        return 0;
    return -1;
}

size_t stack_write(uint32_t fd, char* buf, size_t size) {
    return stack::raw_stack::get_instance()->write(fd, buf, size);
}

size_t stack_read(uint32_t fd, char* buf, size_t size) {
    return stack::raw_stack::get_instance()->read(fd, buf, size);
}

size_t stack_readfrom(uint32_t fd, char* buf, size_t size, struct sockaddr* addr, socklen_t* len) {
    return stack::raw_stack::get_instance()->readfrom(fd, buf, size, addr, len);
}

size_t stack_writeto(uint32_t fd, char* buf, size_t size, struct sockaddr* addr, socklen_t len) {
    return stack::raw_stack::get_instance()->writeto(fd, buf, size, addr, len);
}