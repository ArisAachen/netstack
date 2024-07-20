#include "raw_stack.hpp"
#include "arp.hpp"
#include "def.hpp"
#include "flow.hpp"
#include "icmp.hpp"
#include "interface.hpp"
#include "ip.hpp"
#include "macvlan.hpp"
#include "sock.hpp"
#include "udp.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <memory>
#include <thread>
#include <utility>
#include <variant>

#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>


namespace stack {

// get raw_stack instance
raw_stack::ptr raw_stack::get_instance() {
    static raw_stack::ptr instance = raw_stack::ptr(new raw_stack());
    return instance;
}

raw_stack::raw_stack() {
    neighbor_table_ = flow_table::neighbor_table::create();
    udp_sock_table_ = flow_table::sock_table::create();
    fd_table_ = flow_table::fd_table::create();
}

// init raw_stack
void raw_stack::init() {
    // register macvlan device
    register_device(driver::macvlan_device::ptr(new driver::macvlan_device("new_eth0", "192.168.121.253", "f6:34:95:26:90:66")));
    // register network handler
    register_network_handler(protocol::arp::create(weak_from_this()));
    register_network_handler(protocol::ip::create(weak_from_this()));
    register_network_handler(protocol::icmp::create(weak_from_this()));
    // register transport handler
    register_transport_handler(protocol::udp::create(weak_from_this()));
}

// write buffer to device
void raw_stack::write_to_device(flow::sk_buff::ptr buffer) {
    // look for dst mac address
    if (std::holds_alternative<uint32_t>(buffer->dst)) {
        auto ip_address = std::get<uint32_t>(buffer->dst);
        auto neigh = neighbor_table_->get(ip_address);
        if (!neigh.has_value()) {
            std::cout << "cant find neigh, ip: " << std::hex << ip_address << std::endl;
            return; 
        }
        std::array<uint8_t, def::mac_len> dst;
        memccpy(dst.data(), neigh.value()->mac_address, 0, def::mac_len);
        buffer->dst = dst;
    }

    // check if device exist
    if (!buffer->dev.expired()) {
        auto dev = buffer->dev.lock();
        dev->write_to_device(buffer);
        return;
    }
    // find device
    if (device_map_.empty())
        return;

    device_map_.begin()->second->write_to_device(buffer);
}

void raw_stack::run() {
    handle_packege();
    run_read_device();
}

// wait signal to end
void raw_stack::wait() {
    for (auto& thread : thread_vec_) {
        thread.join();
    }
    for (auto& device : device_map_) {
        device.second->down();
    }
}

// update neighbor info 
void raw_stack::update_neighbor(const struct flow::arp_hdr* hdr, interface::net_device::ptr dev) {
    // check if arp is empty
    if (hdr == nullptr)
        return;
    auto opcode = def::arp_op_code(htons(hdr->operator_code));
    // check if op is request or reply
    if (opcode != def::arp_op_code::request && opcode != def::arp_op_code::reply)
        return;
    auto ip_address = ntohl(hdr->src_ip);
    auto neigh = flow_table::neighbor::create(ip_address, hdr->src_mac, dev);
    neighbor_table_->insert(ip_address, neigh, false);
}

void raw_stack::run_read_device() {
    for (auto& device : device_map_) {
        device.second->up();
        // read buffer from device
        thread_vec_.push_back(std::thread(&interface::net_device::read_thread, device.second));
        // write buffer from device
        thread_vec_.push_back(std::thread(&interface::net_device::write_thread, device.second));
    }
}

void raw_stack::handle_packege() {
    // handle all packages
    for (auto& device : device_map_) {
        auto thread = std::thread([&] {
            while (true) {
                // read from device
                auto buffer = device.second->read_from_device();
                if (buffer == nullptr)
                    continue;
                buffer->stack = weak_from_this();
                // search handle 
                if (!handle_network_package(buffer))
                    continue;
                // check if is icmp request, if is, need send to network layer
                if (def::network_protocol(buffer->protocol) == def::network_protocol::icmp) {
                    handle_network_package(buffer);
                    continue;
                }
                if (!handle_transport_package(buffer))
                    continue;
                // search for socket
                if (def::transport_protocol(buffer->protocol) == def::transport_protocol::udp) {
                    auto sock = udp_sock_table_->sock_get(buffer->key);
                    if (sock != nullptr)
                        sock->write_buffer_to_queue(buffer);
                    else
                        std::cout << "udp sock recv unsaved package" << std::endl;
                } else if (def::transport_protocol(buffer->protocol) == def::transport_protocol::tcp) {
                    std::cout << "current sock is tcp protocol" << std::endl;
                }
            }
        });
        thread_vec_.push_back(std::move(thread));
    }
}

// write transport package
void raw_stack::handle_sock_buffer_package() {
    auto thread = std::thread([&] {
        while (true) {
            // read buffer from sock
            auto buffer = udp_sock_table_->read_buffer();
            if (!write_transport_package(buffer))
                continue;
            if (!write_network_package(buffer))
                continue;
        }
    });
    thread_vec_.push_back(std::move(thread));
    return;
}

// handle network package
bool raw_stack::handle_network_package(flow::sk_buff::ptr buffer) {
    // get network handler
    auto handler = network_handler_map_.find(def::network_protocol(buffer->protocol));
    if (handler == network_handler_map_.end()) {
        std::cout << "recv unknown network protocol flow, protocol: " << std::hex << buffer->protocol << std::endl;
        return false;
    }
    // handle flow
    return handler->second->unpack_flow(buffer);
}


// handle transport package
bool raw_stack::handle_transport_package(flow::sk_buff::ptr buffer) {
    // get transport handler
    auto handler = transport_handler_map_.find(def::transport_protocol(buffer->protocol));
    if (handler == transport_handler_map_.end()) {
        std::cout << "recv unknown transport protocol flow, protocol: " << std::hex << buffer->protocol << std::endl;
        return false;
    }
    // handle flow
    if (!handler->second->unpack_flow(buffer))
        return false;
    // check if key exist
    if (buffer->key == nullptr)
        return true;
    return true;
}

// write network package
bool raw_stack::write_network_package(flow::sk_buff::ptr buffer) {
    auto protocol = def::network_protocol::none;
    switch (buffer->protocol) {
    case uint16_t(def::network_protocol::icmp):
    case uint16_t(def::transport_protocol::tcp):
    case uint16_t(def::transport_protocol::udp):
        protocol = def::network_protocol::ip;
        break;
    default:
        std::cout << "send unknown network package, protocol: " << buffer->protocol << std::endl; 
    }

    // get network handler
    auto handler = network_handler_map_.find(protocol);
    if (handler == network_handler_map_.end()) {
        std::cout << "recv unknown network protocol flow, protocol: " << std::hex << buffer->protocol << std::endl;
        return false;
    }
    // handle flow
    if (!handler->second->pack_flow(buffer))
        return false;
    write_to_device(buffer);
    return true;
}

// write transport package
bool raw_stack::write_transport_package(flow::sk_buff::ptr buffer) {
    // get network handler
    auto handler = transport_handler_map_.find(def::transport_protocol(buffer->protocol));
    if (handler == transport_handler_map_.end()) {
        std::cout << "recv unknown transport protocol flow, protocol: " << std::hex << buffer->protocol << std::endl;
        return false;
    }
    // handle flow
    return handler->second->pack_flow(buffer);
}

raw_stack::~raw_stack() {
    std::cout << "stack release" << std::endl;
    // release device map
    device_map_.clear();
    // release network handler map
    network_handler_map_.clear();
}

// create socket 
uint32_t raw_stack::sock_create(int domain, int type, int protocol) {
    if (type == SOCK_DGRAM) {
       return fd_table_->fd_create(def::transport_protocol::udp); 
    } 
    return -1;
}

// delete socket
bool raw_stack::sock_delete(uint32_t fd) {
    auto key = fd_table_->sock_key_get(fd);
    if (key == nullptr)
        return false;
    fd_table_->fd_delete(fd);
    if (key->protocol == def::transport_protocol::udp) {
        udp_sock_table_->sock_delete(key);
    } else {
        std::cout << "delete unknown protocol key, protocol: " << uint16_t(key->protocol) << std::endl;
    }
    return true;
}

// 
bool raw_stack::connect(uint32_t fd, struct sockaddr* addr, socklen_t len) {
    return false;
}

// close fd
bool raw_stack::close(uint32_t fd) {
    auto key = fd_table_->sock_key_get(fd);
    if (key == nullptr)
        return false;
    if (key->protocol == def::transport_protocol::udp)
        udp_sock_table_->sock_delete(key);
    return true;
}

// bind fd
bool raw_stack::bind(uint32_t fd, struct sockaddr* addr, socklen_t len) {
    auto local_addr = reinterpret_cast<const struct sockaddr_in*>(addr);
    auto key = fd_table_->sock_key_get(fd);
    if (key == nullptr)
        return false;
    if (key->protocol == def::transport_protocol::udp) {
        if (local_addr->sin_addr.s_addr != 0)
            key->local_ip = ntohl(local_addr->sin_addr.s_addr);
        key->local_port = ntohs(local_addr->sin_port);
        udp_sock_table_->sock_create(key);
    }
    return true;
}

// read buffer from stack
size_t raw_stack::read(uint32_t fd, char* buf, size_t size) {
    auto key = fd_table_->sock_key_get(fd);
    if (key == nullptr)
        return false;
    if (key->protocol == def::transport_protocol::udp) {
        auto elem = udp_sock_table_->sock_get(key);
        if (elem == nullptr) {
            std::cout << "write fd failed, fd not exist, fd: " << fd << std::endl;
            return -1; 
        }
        return elem->read(buf, size);
    } 

    return size;
}

// write buffer to stack
size_t raw_stack::write(uint32_t fd, char* buf, size_t size) {
    auto key = fd_table_->sock_key_get(fd);
    if (key == nullptr)
        return false;
    if (key->protocol == def::transport_protocol::udp) {
        auto elem = udp_sock_table_->sock_get(key);
        if (elem == nullptr) {
            std::cout << "write fd failed, fd not exist, fd: " << fd << std::endl;
            return -1; 
        }
        return elem->write(buf, size);
    } 

    return size;
}

// read from stack
size_t raw_stack::readfrom(uint32_t fd, char* buf, size_t size, struct sockaddr* addr, socklen_t* len) {
    auto key = fd_table_->sock_key_get(fd);
    if (key == nullptr)
        return false;
    if (key->protocol == def::transport_protocol::udp) {
        auto elem = udp_sock_table_->sock_get(key);
        if (elem == nullptr) {
            std::cout << "write fd failed, fd not exist, fd: " << fd << std::endl;
            return -1; 
        }
        return elem->readfrom(buf, size, addr, len);
    } 

    return size;
}

// read write to stack
size_t raw_stack::writeto(uint32_t fd, char* buf, size_t size, struct sockaddr* addr, socklen_t len) {
    auto key = fd_table_->sock_key_get(fd);
    if (key == nullptr)
        return false;
    if (key->protocol == def::transport_protocol::udp) {
        auto elem = udp_sock_table_->sock_get(key);
        if (elem == nullptr) {
            std::cout << "write fd failed, fd not exist, fd: " << fd << std::endl;
            return -1; 
        }
        return elem->writeto(buf, size, addr, len);
    } 

    return size;
}

}