#include "macvlan.hpp"
#include "def.hpp"
#include "flow.hpp"
#include "utils.hpp"

#include <array>
#include <cassert>
#include <cerrno>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <mutex>
#include <string>

#include <unistd.h>
#include <net/if.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <linux/if_packet.h>
#include <linux/if_ether.h>

namespace driver {

macvlan_device::macvlan_device(const std::string& dev_name, const std::string& ip_address, const std::string& mac_address)
    : dev_name_(dev_name), status_(def::device_status::down) {
    utils::generic::convert_string_to_mac(mac_address, mac_address_);
    utils::generic::convert_string_to_ip(ip_address, &ip_address_);
    // get ifindex by device name
    if_index_ = if_nametoindex(dev_name_.c_str());
    if (if_index_ == 0) {
        std::cout << "get macvlan ifindex failed, err: " << std::strerror(errno) << std::endl;
    }
}

macvlan_device::~macvlan_device() {
    down();
}

bool macvlan_device::up() {
    // connect to socket
    macvlan_fd_ = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if (macvlan_fd_ < 0) {
        std::cout << "create raw socket failed" << std::strerror(errno) << std::endl;
        return false;
    }
    // create link addr
    struct sockaddr_ll source_link;
    memset(&source_link, 0, sizeof(struct sockaddr_ll));
    source_link.sll_family = AF_PACKET;
    source_link.sll_ifindex = if_index_;
    source_link.sll_protocol = htons(ETH_P_ALL);
    // bind socket to device
    if (bind(macvlan_fd_, (struct sockaddr*)&source_link, sizeof(struct sockaddr_ll)) < 0) {
        std::cout << "bind raw socket failed" << std::strerror(errno) << std::endl;
        return false;
    }
    std::cout << "up macvlan device success, device name: " << dev_name_ << ", ifindex: " << (int)if_index_ 
        << ", mac: " << std::hex << utils::generic::format_mac_address(mac_address_) 
        << ", ip: " << utils::generic::format_ip_address(ip_address_) << std::endl;
    return true;
}

bool macvlan_device::down() {
    if (macvlan_fd_ != 0)
        close(macvlan_fd_); 
    return 0;
}

// read buffer from device
flow::sk_buff::ptr macvlan_device::read_from_device() {
    // unique lock
    std::unique_lock<std::mutex> lock(read_mutex_);
    read_cond_.wait(lock, [this] { return !this->read_head_.empty(); });
    // get buffer
    auto buffer = read_head_.front();
    read_head_.pop();
    return buffer;
}

// write buffer to device
int macvlan_device::write_to_device(flow::sk_buff::ptr buffer) {
    append_buffer_to_write_queue(buffer);
    if (!buffer->child_frags.empty()) {
        for (auto iter : buffer->child_frags)
            append_buffer_to_write_queue(iter);
    }
    return 0;
}

// get macvlan device mac
uint8_t* macvlan_device::get_device_mac() {
    return mac_address_;
}

// get macvlan device ip
uint32_t macvlan_device::get_device_ip() {
    return ip_address_;
}

// get device index
uint8_t macvlan_device::get_device_ifindex() {
    return if_index_;
}

void macvlan_device::read_thread() {
    // check if fd is valid
    assert(macvlan_fd_ != 0);
    char buf[def::flow_buffer_size];
    while (true) {
        size_t size = read(macvlan_fd_, buf, def::flow_buffer_size);
        if (size < 0) {
            std::cout << "read macvlan buffer failed" << std::endl;
            break;
        } else if (size == 0) {
            std::cout << "read macvlan buffer end" << std::endl;
        }
        // get ether mac 
        const flow::ether_hdr* hdr = reinterpret_cast<const flow::ether_hdr*>(buf);
        // check if should ignore
        if (!memcmp(mac_address_, hdr->dst, def::mac_len) &&
            !memcmp(def::broadcast_mac, hdr->dst, def::mac_len))
            continue;
        std::cout << "rcv ether msg, " << utils::generic::format_mac_address(hdr->src) << " -> "
            << utils::generic::format_mac_address(hdr->dst) << std::endl;
        // malloc flow
        flow::sk_buff::ptr skb = flow::sk_buff::alloc(size);
        // copy buffer
        skb->protocol = htons(hdr->protocol);
        skb->dev = weak_from_this();
        skb->store_data(buf, size);
        flow::skb_reserve(skb, flow::get_ether_offset());
        // push to queue
        std::lock_guard<std::mutex> lock(read_mutex_);
        read_head_.push(skb);
        // std::cout << utils::generic::format_mac_address(hdr->src) << " -> " 
        //     << utils::generic::format_mac_address(hdr->dst) << ", sizes: " 
        //     << std::dec << size << ", protocol: "<< std::hex << (int)skb->protocol << std::endl;
        read_cond_.notify_one();
    }
}


// write buffer to device
void macvlan_device::write_thread() {
    // init write buffer
    flow::sk_buff::ptr buffer = nullptr;
    while (true) {
        {
            std::unique_lock<std::mutex> lock(write_mutex_);
            write_cond_.wait(lock, [&] () { return !write_head_.empty(); });
            buffer = write_head_.front();
            write_head_.pop();
        }
        // write buffer to device
        size_t size = write(macvlan_fd_, buffer->get_data(), buffer->get_data_len());
        if (size < 0) {
            std::cout << "write macvlan buffer failed" << std::endl;
            break;
        } else if (size == 0) {
            std::cout << "write macvlan buffer end" << std::endl;
        }
    }
}

// apend buffer
void macvlan_device::append_buffer_to_write_queue(const flow::sk_buff::ptr buffer) {
    // push to ether header
    flow::skb_push(buffer, sizeof(struct flow::ether_hdr));
    // create ether header
    struct flow::ether_hdr* ether_hdr = reinterpret_cast<struct flow::ether_hdr*>(buffer->get_data());
    ether_hdr->protocol = htons(buffer->protocol);
    memcpy(ether_hdr->src, mac_address_, def::mac_len);
    memcpy(ether_hdr->dst, std::get<std::array<uint8_t, def::mac_len>>(buffer->dst).data(), def::mac_len);
    std::unique_lock<std::mutex> lock(write_mutex_);
    write_head_.push(buffer);
    write_cond_.notify_one();
}


}