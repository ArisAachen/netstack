#include "macvlan.hpp"
#include "def.hpp"
#include "flow.hpp"
#include "utils.hpp"

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
    : dev_name_(dev_name), ip_address_(ip_address), status_(def::device_status::down) {
    utils::generic::convert_string_to_mac(mac_address, mac_address_);
}

macvlan_device::~macvlan_device() {
    down();
}

bool macvlan_device::up() {
    // get ifindex by device name
    int ifindex = if_nametoindex(dev_name_.c_str());
    if (ifindex == 0) {
        std::cout << "get macvlan ifindex failed, err: " << std::strerror(errno) << std::endl;
        return false;
    }
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
    source_link.sll_ifindex = ifindex;
    source_link.sll_protocol = htons(ETH_P_ALL);
    // bind socket to device
    if (bind(macvlan_fd_, (struct sockaddr*)&source_link, sizeof(struct sockaddr_ll)) < 0) {
        std::cout << "bind raw socket failed" << std::strerror(errno) << std::endl;
        return false;
    }
    std::cout << "up macvlan device success, device name: " << dev_name_ << ", ifindex: " << ifindex 
        << ", mac: " << std::hex << utils::generic::format_mac_address(mac_address_) << std::endl;
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
    std::unique_lock<std::mutex> lock(write_mutex_);
    write_head_.push(buffer);
    write_cond_.notify_one();
    return 0;
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
        const flow::ether_hr* hdr = reinterpret_cast<const flow::ether_hr*>(buf);
        // check if should ignore
        if (!memcmp(mac_address_, hdr->dst, sizeof(uint8_t) * def::mac_len) &&
            !memcmp(def::broadcast_mac, hdr->dst, sizeof(uint8_t) * def::mac_len))
            continue;
        // malloc flow
        flow::sk_buff::ptr skb = flow::sk_buff::ptr(new flow::sk_buff());
        skb->total_len = sizeof(struct flow::sk_buff) + size;
        skb->data_len = size;
        skb->protocol = htons(hdr->protocol);
        skb->store_data(buf, size);
        flow::skb_pull(skb, sizeof(struct flow::ether_hr));
        // push to queue
        std::lock_guard<std::mutex> lock(read_mutex_);
        read_head_.push(skb);
        std::cout << utils::generic::format_mac_address(hdr->src) << " -> " 
            << utils::generic::format_mac_address(hdr->dst) << ", protocol: " << std::hex << (int)skb->protocol << std::endl;
        read_cond_.notify_one();
    }
}

void macvlan_device::write_thread() {
    // check if fd is valid
    assert(macvlan_fd_ != 0);
    char buf[512] = "Hello macvlan";
    while (true) {
        size_t size = write(macvlan_fd_, buf, 512);
        if (size < 0) {
            std::cout << "write macvlan buffer failed" << std::endl;
            break;
        } else if (size == 0) {
            std::cout << "write macvlan buffer end" << std::endl;
        } 
        std::cout << "write macvlan buffer, buf: " << std::endl;
    }
}

}