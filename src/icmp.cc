#include "icmp.hpp"
#include "def.hpp"
#include "flow.hpp"
#include "utils.hpp"

#include <iostream>
#include <netinet/in.h>

namespace protocol {

icmp::icmp(interface::stack::weak_ptr stack) {
    stack_ = stack;
}

// create icmp 
icmp::ptr icmp::create(interface::stack::weak_ptr stack) {
    return icmp::ptr(new icmp(stack));
}

// get network icmp protocol
def::network_protocol icmp::get_protocol() {
    return def::network_protocol::icmp;
}

// pack icmp flow
bool icmp::pack_flow(flow::sk_buff::ptr buffer) {
    return true;
}

// unpack flow
bool icmp::unpack_flow(flow::sk_buff::ptr buffer) {
    // get icmp header
    auto hdr = reinterpret_cast<const struct flow::icmp_hdr*>(buffer->get_data());
    if (hdr == nullptr)
        return false;
    flow::skb_pull(buffer, sizeof(struct flow::icmp_hdr));
    // check echo type 
    switch (def::icmp_type(hdr->icmp_type)) {
    case def::icmp_type::request:
        return icmp_echo_request(buffer);
    case def::icmp_type::reply:

    default:
        std::cout << "rcv unknown icmp type" << hdr->icmp_type << std::endl;
        return false;
    }
    
    return false;
}

// handle icmp echo request 
bool icmp::handle_icmp_echo_request(flow::sk_buff::ptr req_buffer) {
    auto hdr = reinterpret_cast<const struct flow::icmp_echo_body*>(req_buffer->get_data());
    if (hdr == nullptr)
        return false;
    std::cout << "rcv icmp msg, identifier: " << std::dec << ntohs(hdr->identifier) 
        << ", seq: " << ntohs(hdr->sequence_number) << std::endl;
    // send back length is the same
    auto total_len = req_buffer->data_len;
    auto buf_len = req_buffer->get_data_len();
    auto resp_buffer = flow::sk_buff::alloc(total_len);

    return false;
}

// handle icmp echo request 
bool icmp::handle_icmp_echo_reply(flow::sk_buff::ptr buffer) {
    


    return false;
}

bool icmp::icmp_send(flow::sk_buff::ptr buffer) {

    return false;
}

}