#include "udp.hpp"
#include "def.hpp"
#include "flow.hpp"
#include "sock.hpp"

#include <cstdint>
#include <string>

#include <netinet/in.h>

namespace protocol {

// create udp handler
udp::ptr udp::create(interface::stack::weak_ptr stack) {
    auto udp_handler = udp::ptr(new udp(stack));
    return udp_handler;
}

udp::udp(interface::stack::weak_ptr stack) {
    stack_ = stack;
}

// get transport udp 
def::transport_protocol udp::get_protocol() {
    return def::transport_protocol::udp;
}

bool udp::pack_flow(flow::sk_buff::ptr buffer) {
    auto key = buffer->key;
    auto local_ip = key->local_ip;
    // set src and dst
    if (local_ip == 0)
        local_ip = def::global_def_ip;
    buffer->src = local_ip;
    buffer->dst = key->remote_ip; 
    // get hdr
    flow::skb_push(buffer, sizeof(struct flow::udp_hdr));
    // get udp data len 
    auto len = buffer->get_data_len();
    // fix header
    auto hdr = reinterpret_cast<flow::udp_hdr*>(buffer->get_data());
    hdr->src_port = htons(key->local_port);
    hdr->dst_port = htons(key->remote_port);
    hdr->total_len = htons(len);
    // set checksum
    hdr->udp_checksum = 0;
    // add fake udp header
    flow::skb_push(buffer, sizeof(struct flow::udp_fake_hdr));
    auto fake_hdr = reinterpret_cast<flow::udp_fake_hdr*>(buffer->get_data());
    fake_hdr->src_ip = htonl(local_ip);
    fake_hdr->dst_ip = htonl(key->remote_ip);
    fake_hdr->reserve = 0;
    fake_hdr->protocol = uint8_t(def::transport_protocol::udp);
    fake_hdr->total_len = htons(len);
    // get checksum 
    hdr->udp_checksum = htons(flow::compute_checksum(buffer));
    // drop fake header
    flow::skb_pull(buffer, sizeof(struct flow::udp_fake_hdr));
    buffer->data_len += sizeof(struct flow::udp_hdr);
    return true;
}

// unpack udp flow
bool udp::unpack_flow(flow::sk_buff::ptr buffer) {
    // get udp hdr
    auto hdr = reinterpret_cast<flow::udp_hdr*>(buffer->get_data());
    if (hdr == nullptr)
        return false;
    // get msg
    auto msg_len = ntohs(hdr->total_len) - sizeof(struct flow::udp_hdr);
    flow::skb_pull(buffer, sizeof(struct flow::udp_hdr));
    auto local_ip = std::get<uint32_t>(buffer->dst);
    auto local_port = ntohs(hdr->dst_port);
    auto remote_ip = std::get<uint32_t>(buffer->src);
    auto remote_port = ntohs(hdr->src_port);
    // create key
    buffer->key = flow_table::sock_key::ptr(new flow_table::sock_key(local_ip, local_port, 
        remote_ip, remote_port, def::transport_protocol::udp));
    buffer->protocol = uint16_t(def::transport_protocol::udp);
    // copy message
    auto msg = std::string(buffer->get_data(), msg_len);
    std::cout << "rcv udp message, " << std::dec << remote_port << " -> " 
        << local_port << ", msg: " << msg << std::endl;
    return true;
}

bool udp::write_buffer_to_sock(flow::sk_buff::ptr buffer) {
    return false;
}
  
bool udp::udp_rcv(flow::sk_buff::ptr buffer) {
    return false;
}


bool udp::udp_send(flow::sk_buff::ptr buffer) {
    return false;
}

}