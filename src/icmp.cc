#include "icmp.hpp"
#include "def.hpp"
#include "flow.hpp"
#include "utils.hpp"

#include <cstdint>
#include <iostream>
#include <netinet/in.h>
#include <sys/types.h>

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
        return handle_icmp_echo_request(buffer);
    case def::icmp_type::reply:

    default:
        std::cout << "rcv unknown icmp type" << hdr->icmp_type << std::endl;
        return false;
    }
    
    return false;
}

// handle icmp echo request 
bool icmp::handle_icmp_echo_request(flow::sk_buff::ptr req_buffer) {
    auto req_hdr = reinterpret_cast<const struct flow::icmp_echo_body*>(req_buffer->get_data());
    if (req_hdr == nullptr)
        return false;
    std::cout << "rcv icmp request, identifier: " << std::dec << ntohs(req_hdr->identifier) 
        << ", seq: " << ntohs(req_hdr->sequence_number) << std::endl;
    // send back length is the same
    auto total_len = req_buffer->data_len;
    auto buf_len = req_buffer->get_data_len() - sizeof(struct flow::icmp_echo_body);
    auto resp_buffer = flow::sk_buff::alloc(total_len);
    resp_buffer->data_len = total_len;
    // define next layer protocol
    resp_buffer->protocol = uint16_t(def::network_protocol::icmp);
    resp_buffer->dev = req_buffer->dev;
    resp_buffer->stack = stack_;
    // set tail
    flow::skb_reserve(resp_buffer, total_len);
    flow::skb_push(resp_buffer, buf_len + sizeof(struct flow::icmp_echo_body));
    // copy request src ip to response dst ip
    resp_buffer->src = std::get<uint32_t>(req_buffer->dst);
    resp_buffer->dst = std::get<uint32_t>(req_buffer->src);
    auto icmp_echo_body = reinterpret_cast<struct flow::icmp_echo_body*>(resp_buffer->get_data());
    if (icmp_echo_body == nullptr)
        std::cout << "icmp echo body is invalid" << std::endl;
    // set icmp echo body
    icmp_echo_body->identifier = req_hdr->identifier;
    icmp_echo_body->sequence_number = req_hdr->sequence_number;
    memccpy(icmp_echo_body->data, req_hdr->data, 0, buf_len);
    // set icmp request hdr
    flow::skb_push(resp_buffer, sizeof(struct flow::icmp_hdr));
    auto icmp_hdr = reinterpret_cast<flow::icmp_hdr*>(resp_buffer->get_data());
    icmp_hdr->icmp_code = uint8_t(def::icmp_type::reply);
    icmp_hdr->icmp_code = uint8_t(def::icmp_code::none);
    // set checksum to 0 first, in case compute error checksum
    icmp_hdr->icmp_checksum = 0;
    icmp_hdr->icmp_checksum = htons(flow::compute_checksum(resp_buffer));
    if (stack_.expired())
        return false;
    auto stack = stack_.lock();
    stack->write_network_package(resp_buffer);
    std::cout << "send icmp response, identifier: " << std::dec << ntohs(icmp_echo_body->identifier) 
        << ", seq: " << ntohs(icmp_echo_body->sequence_number) << std::endl;
    return false;
}

// handle icmp echo request 
bool icmp::handle_icmp_echo_reply(flow::sk_buff::ptr buffer) {
    


    return false;
}

bool icmp::send_icmp_echo_request(flow::sk_buff::ptr buffer) {
    return false;
}

bool icmp::send_icmp_echo_reply(flow::sk_buff::ptr req_buffer) {
    return false;
}

bool icmp::icmp_send(flow::sk_buff::ptr buffer) {

    return false;
}

}