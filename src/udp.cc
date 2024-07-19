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

    return false;
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
    // copy message
    auto msg = std::string(buffer->get_data(), msg_len);
    std::cout << "rcv udp message, " << std::dec << ntohs(hdr->src_port) << " -> " 
        << ntohs(hdr->dst_port) << ", msg: " << msg << std::endl;
    auto local_ip = std::get<uint32_t>(buffer->dst);
    auto local_port = ntohs(hdr->dst_port);
    auto remote_ip = std::get<uint32_t>(buffer->src);
    auto remote_port = ntohs(hdr->src_port);
    // create key
    buffer->key = flow_table::sock_key::ptr(new flow_table::sock_key(local_ip, local_port, 
        remote_ip, remote_port, def::transport_protocol::udp));
    buffer->protocol = uint16_t(def::transport_protocol::udp);
    return false;
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