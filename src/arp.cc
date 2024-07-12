#include "arp.hpp"
#include "def.hpp"
#include "flow.hpp"

#include <cstdint>
#include <iostream>
#include <netinet/in.h>

namespace protocol {

bool arp::pack_flow(flow::sk_buff::ptr buffer) {
    return false;
}


// unpack flow
bool arp::unpack_flow(flow::sk_buff::ptr buffer) {
    // get arp header
    auto hdr = reinterpret_cast<const flow::arp_hdr*>(buffer->get_data());
    if (hdr == nullptr) 
        return false;
    std::cout << "arp buffer" << std::hex << buffer->get_data() << std::endl;
    uint16_t opcode = htons(hdr->operator_code);
    std::cout << "op code: " << std::hex << opcode << std::endl;
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
    auto hdr = reinterpret_cast<const flow::arp_hdr*>(buffer->get_data());
    // check target ip 
    if (htonl(hdr->dst_ip) != def::global_def_ip) {
        std::cout << "arp request target is not local" << std::endl;
        return false;
    }
    std::cout << "arp request target is local" << std::endl;

    return true;
}


}