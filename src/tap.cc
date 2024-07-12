#include "tap.hpp"
#include "def.hpp"
#include "utils.hpp"

#include <cassert>
#include <cerrno>
#include <cstring>
#include <iostream>
#include <unistd.h>

namespace driver {

tap_device::tap_device(const std::string& dev_name, const std::string& ip_address, const std::string& mac_address)
    : dev_name_(dev_name), ip_address_(ip_address), mac_address_(mac_address), status_(def::device_status::down) {
    
}

tap_device::~tap_device() {
    down();
}

bool tap_device::up() {
    // create tap device 
    auto fd = utils::device::create_kernel_device(dev_name_, "/dev/net/tun", def::device_type::tap);
    if (!fd.has_value()) {
        std::cout << "create device failed, err: " << std::strerror(errno) << std::endl;
        return false;
    }
    tap_fd_ = fd.value();
    std::cout << "create device success" << std::endl;
    // set tap device up
    if (!utils::device::set_kernel_device_status(dev_name_, def::device_status::up).has_value()) {
        std::cout << "set device up failed, err: " << std::strerror(errno) << std::endl;
        return false;
    }
    std::cout << "set device up success" << std::endl;
    return 0;
}

bool tap_device::down() {
    if (tap_fd_ != 0)
        close(tap_fd_); 
    return 0;
}

int tap_device::read_from_device(void* buf, int len) {


    return 0;
}

int tap_device::write_to_device(const void* buf, int len) {
    return 0;
}

void tap_device::read_thread() {
    // check if fd is valid
    assert(tap_fd_ != 0);
    char msg[218];
    while (true) {
        size_t size = read(tap_fd_, msg, 218);
        if (size < 0) {
            std::cout << "read tap buffer failed" << std::endl;
            break;
        } else if (size == 0) {
            std::cout << "read tap buffer end" << std::endl;
        }
        std::cout << "read tap buffer success, buf: " << msg << std::endl;
    }
}

void tap_device::write_thread() {
    // check if fd is valid
    assert(tap_fd_ != 0);
    char buf[512] = "Hello tap";
    while (true) {
        size_t size = write(tap_fd_, buf, 512);
        if (size < 0) {
            std::cout << "write tap buffer failed" << std::endl;
            break;
        } else if (size == 0) {
            std::cout << "write tap buffer end" << std::endl;
        } 
        std::cout << "write tap buffer, buf: " << std::endl;
    }
}

}