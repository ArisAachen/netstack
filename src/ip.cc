#include "ip.hpp"
#include "def.hpp"
#include "flow.hpp"
#include "utils.hpp"

#include <iostream>
#include <netinet/in.h>

namespace protocol {

ip::ip(interface::stack::weak_ptr stack) {
    stack_ = stack;
}

// create ip 
ip::ptr ip::create(interface::stack::weak_ptr stack) {
    return ip::ptr(new ip(stack));
}

// get network ip protocol
def::network_protocol ip::get_protocol() {
    return def::network_protocol::ip;
}

// pack ip flow
bool ip::pack_flow(flow::sk_buff::ptr buffer) {
    return true;
}

// unpack flow
bool ip::unpack_flow(flow::sk_buff::ptr buffer) {
    // get ip header
    auto hdr = reinterpret_cast<const struct flow::ip_hdr*>(buffer->get_data());
    if (hdr == nullptr)
        return false;
    buffer->protocol = hdr->protocol;
    std::cout << "rcv ip msg, src: " << utils::generic::format_ip_address(ntohl(hdr->src_ip))
        << ", dst: " << utils::generic::format_ip_address(ntohl(hdr->dst_ip)) 
        << ", protocol: " << (int)(def::network_protocol(buffer->protocol)) << std::endl;
    flow::skb_pull(buffer, sizeof(struct flow::ip_hdr));

    //check if stack expired
    if (stack_.expired())
        return false;


    return true;
}

// ip rcv 
bool ip::ip_rcv(flow::sk_buff::ptr buffer) {
    return true;
}

// ip send
bool ip::ip_send(flow::sk_buff::ptr buffer) {
    return true;
}

}