#include "ip.hpp"
#include "def.hpp"
#include "flow.hpp"
#include "utils.hpp"

#include <cstddef>
#include <cstdint>
#include <vector>
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
    // check if need mtu
    if (!buffer->child_frags.empty()) {
        std::cout << "use ip fast fragment" << std::endl;
        ip_make_flow(buffer, 0, false);
        for (auto iter : buffer->child_frags)
            ip_make_flow(buffer, 0, false);
    } else if (buffer->mtu && buffer->get_data_len() > (buffer->mtu - sizeof(struct flow::ip_hdr))) {
        std::cout << "use slow fast fragment" << std::endl;
        ip_fragment(buffer);
    } else {
        ip_make_flow(buffer, 0, false);
    }

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
    // get real offset here
    flow::skb_put(buffer, htons(hdr->total_len));

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

// make ip flow 
bool ip::ip_make_flow(flow::sk_buff::ptr buffer, size_t offset, bool more_flag) {
    // set ip hdr
    flow::skb_push(buffer, sizeof(struct flow::ip_hdr));
    auto hdr = reinterpret_cast<flow::ip_hdr*>(buffer->get_data());
    // append to tail
    hdr->version_and_head_len = (4 << 4) + 5;
    hdr->diff_serv_and_ecn = 0;
    hdr->total_len = htons(buffer->get_data_len());
    hdr->identification = htons(identification_ + 1);
    auto flag_and_offset = uint16_t(0);
    // set flag
    if (offset || more_flag)
        flag_and_offset |= (0b00 << def::ip_flag_offset);
    if (more_flag)
        flag_and_offset |= (0b01 << def::ip_flag_offset);
    flag_and_offset |= offset;
    hdr->flag_and_fragoffset |= htons(flag_and_offset);
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

// make ip fragment
bool ip::ip_fragment(flow::sk_buff::ptr buffer) {
    std::vector<flow::sk_buff::ptr> child_buffer;
    auto frag_size = buffer->mtu - sizeof(struct flow::ip_hdr);
    // continue make len
    for (int index = 1; buffer->data_len > frag_size; index++) {
        // check rest part of data
        flow::sk_buff::ptr frag_buffer;
        auto copy_size = buffer->data_len - frag_size;
        auto more_flag = false;
        // get copy buffer
        if (copy_size > frag_size) {
            auto alloc_size = buffer->mtu + def::max_ether_header;
            frag_buffer = flow::sk_buff::alloc(alloc_size);
            frag_buffer->data_len = alloc_size;
            flow::skb_reserve(frag_buffer, def::max_ether_header);
            flow::skb_header_clone(buffer, frag_buffer);
            copy_size = frag_size;
            more_flag = true;
        } else {
            auto alloc_size = copy_size + sizeof(struct flow::ip_hdr) + def::max_ether_header;
            frag_buffer = flow::sk_buff::alloc(alloc_size);
            frag_buffer->data_len = alloc_size;
            flow::skb_reserve(frag_buffer, def::max_ether_header);
            flow::skb_header_clone(buffer, frag_buffer);
        }
        // set buffer
        flow::skb_reserve(frag_buffer, sizeof(struct flow::ip_hdr));
        // copy data 
        memccpy(frag_buffer->get_data(), buffer->get_data() + index * frag_size, 0, copy_size);
        flow::skb_put(frag_buffer, copy_size);
        // make ip header
        ip_make_flow(frag_buffer, index * frag_size / def::ip_frag_offset_base, more_flag);
        // resize data len
        buffer->data_len -= copy_size;
        child_buffer.push_back(frag_buffer);
    }
    // reset buffer tail
    buffer->data_tail = buffer->data_begin + frag_size;
    ip_make_flow(buffer, 0, true);

    // check if child buffer is empty
    if (!child_buffer.empty())
        buffer->child_frags = std::move(child_buffer);

    return true;
};

}