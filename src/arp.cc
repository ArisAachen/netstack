#include "arp.hpp"
#include "def.hpp"
#include "flow.hpp"
#include "utils.hpp"

#include <cstddef>
#include <cstdint>
#include <iostream>
#include <mutex>
#include <optional>
#include <shared_mutex>
#include <utility>

#include <netinet/in.h>

namespace protocol {

arp::arp(interface::stack::weak_ptr stack) : stack_(stack) {}

// create arp 
arp::ptr arp::create(interface::stack::weak_ptr stack) {
    return arp::ptr(new arp(stack));
}

// get network protocol
def::network_protocol arp::get_protocol() {
    return def::network_protocol::arp;
}

bool arp::pack_flow(flow::sk_buff::ptr buffer) {
    return false;
}

// unpack flow
bool arp::unpack_flow(flow::sk_buff::ptr buffer) {
    // get arp header
    auto hdr = reinterpret_cast<const flow::arp_hdr*>(buffer->get_data());
    if (hdr == nullptr) 
        return false;
    uint16_t opcode = ntohs(hdr->operator_code);
    switch (def::arp_op_code(opcode)) {
    // handle arp request
    case def::arp_op_code::request:
        return handle_arp_request(buffer);
    case def::arp_op_code::reply:
        return handle_arp_response(buffer);
    default:
        std::cout << "recv unknown arp request, code: " << std::hex << opcode << std::endl;
    }

    return true;
}

// handle arp request
bool arp::handle_arp_request(flow::sk_buff::ptr buffer) {
    auto req_hdr = reinterpret_cast<const flow::arp_hdr*>(buffer->get_data());
    // check target ip 
    if (ntohl(req_hdr->dst_ip) != def::global_def_ip) {
        return false;
    }
    size_t size = sizeof(struct flow::arp_hdr) + sizeof(struct flow::ether_hr);
    flow::sk_buff::ptr resp_buffer = flow::sk_buff::alloc(size);
    // create response header
    struct flow::arp_hdr resp_hdr;
    resp_hdr.hardware_type = req_hdr->hardware_type;
    resp_hdr.hardware_len = req_hdr->hardware_len;
    resp_hdr.protocol = req_hdr->protocol;
    resp_hdr.protocol_len = req_hdr->protocol_len;
    resp_hdr.operator_code = htons(uint16_t(def::arp_op_code::reply));
    // check if device has expired
    if (buffer->dev.expired()) {
        std::cout << "device has expired" << std::endl;
        return false;
    }
    // get device
    auto dev = buffer->dev.lock();
    // copy source device info
    resp_hdr.src_ip = htonl(dev->get_device_ip());
    memccpy(resp_hdr.src_mac, dev->get_device_mac(), 0, sizeof(uint8_t) * def::mac_len);
    // copy dst device info
    resp_hdr.dst_ip = req_hdr->src_ip;
    memccpy(resp_hdr.dst_mac, req_hdr->src_mac, 0, sizeof(uint8_t) * def::mac_len);
    // copy buffer to skb
    flow::skb_pull(resp_buffer, sizeof(struct flow::ether_hr));
    resp_buffer->protocol = uint16_t(def::network_protocol::arp);
    resp_buffer->store_data((char *)&resp_hdr, sizeof(struct flow::arp_hdr));
    memccpy(resp_buffer->dst.mac_address, req_hdr->src_mac, 0, sizeof(uint8_t) * def::mac_len);
    // check if stack has expired
    if (buffer->stack.expired()) {
        std::cout << "stack has expired" << std::endl;
        return false;
    }
    // get stack
    auto stack = buffer->stack.lock();
    stack->update_neighbor(req_hdr, dev);
    stack->write_to_device(resp_buffer);
    send_arp_request(req_hdr->src_ip, dev);

    return true;
}

// handle arp response
bool arp::handle_arp_response(flow::sk_buff::ptr buffer) {
    std::cout << "handle arp response" << std::endl;
    auto resp_hdr = reinterpret_cast<const flow::arp_hdr*>(buffer->get_data());
    // check target ip
    if (ntohl(resp_hdr->dst_ip) != def::global_def_ip) {
        return false;
    }
    // check if stack has expired
    if (buffer->stack.expired()) {
        std::cout << "stack has expired" << std::endl;
        return false;
    }
    // get stack
    auto stack = buffer->stack.lock();
    stack->update_neighbor(resp_hdr, buffer->dev.lock());
    return true;
}

// send arp request
bool arp::send_arp_request(uint32_t ip, interface::net_device::ptr dev) {
    // create buffer
    auto buffer = flow::sk_buff::alloc(sizeof(struct flow::arp_hdr) + sizeof(struct flow::ether_hr));
    // create arp header
    struct flow::arp_hdr hdr;
    hdr.hardware_type = htons(uint16_t(def::hardware_type::ethernet));
    hdr.protocol = htons(uint16_t(def::network_protocol::ip));
    hdr.hardware_len = def::mac_len;
    hdr.protocol_len = def::ip_len;
    hdr.operator_code = htons(uint16_t(def::arp_op_code::request));
    // copy source device info
    hdr.src_ip = htonl(dev->get_device_ip());
    memccpy(hdr.src_mac, dev->get_device_mac(), 0, sizeof(uint8_t) * def::mac_len);
    // copy dst device info
    hdr.dst_ip = ip;
    memccpy(hdr.dst_mac, def::broadcast_mac, 0, sizeof(uint8_t) * def::mac_len);
    // copy buffer to skb
    flow::skb_pull(buffer, sizeof(struct flow::ether_hr));
    buffer->protocol = uint16_t(def::network_protocol::arp);
    buffer->store_data((char *)&hdr, sizeof(struct flow::arp_hdr));
    memccpy(buffer->dst.mac_address, def::broadcast_mac, 0, sizeof(uint8_t) * def::mac_len);
    // store device
    buffer->dev = dev;
    // check if stack has expired
    if (stack_.expired()) {
        std::cout << "stack has expired" << std::endl;
        return false;
    }
    // get stack
    auto stack = stack_.lock();
    stack->write_to_device(buffer);

    return true;
}


}


namespace flow_table {

neighbor_table::neighbor_table() {

}

neighbor_table::~neighbor_table() {

}

// insert neighbor table
void neighbor_table::insert(uint32_t key, neighbor::ptr neigh, bool replace) {
    // check if table already exist, and need replace
    if (get(key).has_value() && !replace) 
        return;
    // update ip neigh table
    std::lock_guard<std::shared_mutex> lock(mutex_);
    neigh_map_.insert(std::make_pair(key, neigh));
    std::cout << "insert neighbor table, ip: " << utils::generic::format_ip_address(htonl(key))
        << ", mac: " << utils::generic::format_mac_address(neigh->mac_address) << std::endl;
}

// remove neighbor table
void neighbor_table::remove(uint32_t key) {
    std::lock_guard<std::shared_mutex> lock(mutex_);
    neigh_map_.erase(neigh_map_.find(key));
}

// get neighbor table
std::optional<neighbor::ptr> neighbor_table::get(uint32_t key) {
    std::shared_lock<std::shared_mutex> lock(mutex_);
    auto elem = neigh_map_.find(key);
    if (elem == neigh_map_.end())
        return std::nullopt;
    return elem->second;
}

}