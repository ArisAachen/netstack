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
    // set ip hdr
    flow::skb_push(buffer, sizeof(struct flow::ip_hdr));
    auto hdr = reinterpret_cast<flow::ip_hdr*>(buffer->get_data());
    // append to tail
    hdr->version = 5;
    // TODO: ip head is not consistent
    hdr->ip_head_len = 4;
    hdr->diff_serv = 0;
    hdr->ecn = 0;
    hdr->total_len = htons(buffer->get_data_len());
    hdr->identification = htons(identification_ + 1);
    hdr->flags = 0;
    hdr->fragment_offset = 0;
    hdr->time_to_live = def::ip_time_to_live;
    hdr->protocol = buffer->protocol;
    // in case checksum failed
    hdr->head_checksum = 0;
    hdr->src_ip = htonl(std::get<uint32_t>(buffer->src));
    hdr->dst_ip = htonl(std::get<uint32_t>(buffer->dst));
    // compute checksum
    auto data_tail = buffer->data_tail;
    buffer->data_tail = buffer->data_begin + sizeof(struct flow::ip_hdr);
    hdr->head_checksum = ntohs(flow::compute_checksum(buffer));
    buffer->data_tail = data_tail;
    buffer->protocol = uint16_t(def::network_protocol::ip);
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
    // store src ip and dst ip
    buffer->src = ntohl(hdr->src_ip);
    buffer->dst = ntohl(hdr->dst_ip);
    flow::skb_pull(buffer, sizeof(struct flow::ip_hdr));

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