#include "arp.hpp"
#include "def.hpp"
#include "flow.hpp"
#include "utils.hpp"

#include <cstdint>
#include <iostream>

#include <netinet/in.h>

namespace protocol {

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

    default:
        std::cout << "recv unknown arp request" << std::endl;
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
    std::cout << "arp request target is local" << std::endl;
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
    stack->write_to_device(resp_buffer);

    return true;
}


}