#include "ip.hpp"
#include "def.hpp"
#include "flow.hpp"
#include "utils.hpp"

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <unordered_map>
#include <utility>
#include <vector>
#include <iostream>

#include <netinet/in.h>


namespace protocol {

ip::ip(interface::stack::weak_ptr stack) {
    stack_ = stack;
    defrag_queue_ = flow_table::ip_defrag_queue::create();
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
    std::cout << "rcv ip msg, src: " << utils::generic::format_ip_address(ntohl(hdr->src_ip))
        << ", dst: " << utils::generic::format_ip_address(ntohl(hdr->dst_ip)) 
        << ", protocol: " << (int)(def::network_protocol(hdr->protocol)) << std::endl;
    // get protocol
    buffer->protocol = hdr->protocol;
    // store src ip and dst ip
    buffer->src = ntohl(hdr->src_ip);
    buffer->dst = ntohl(hdr->dst_ip);
    // get more frag
    auto flag_and_fragoffset = ntohs(hdr->flag_and_fragoffset);
    auto more_flag = (uint16_t)(flag_and_fragoffset >> def::ip_flag_offset & 0b001);
    // get offset 
    auto offset = uint16_t(flag_and_fragoffset << 3) >> 3;
    if (!more_flag && offset == 0) {
        std::cout << "rcv ip msg dont need defrag" << std::endl;
        flow::skb_pull(buffer, sizeof(struct flow::ip_hdr));
        // get real offset here
        flow::skb_put(buffer, htons(hdr->total_len));
        return true;
    }
    return ip_defragment(buffer);
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

bool ip::ip_defragment(flow::sk_buff::ptr buffer) {
    // check if find full package
    auto defrag_buffer = defrag_queue_->defrag_push(buffer);
    if (defrag_buffer == nullptr)
        return false;
    buffer = defrag_buffer;
    return true;
}


}


namespace flow_table {

ip_defrag_queue::ip_defrag_queue() {
    
}

ip_defrag_queue::ptr ip_defrag_queue::create() {
    return ip_defrag_queue::ptr(new ip_defrag_queue());
}

flow::sk_buff::ptr ip_defrag_queue::defrag_push(flow::sk_buff::ptr buffer) {    
    // get ip header
    auto hdr = reinterpret_cast<const struct flow::ip_hdr*>(buffer->get_data());
    if (hdr == nullptr)
        return nullptr;
    // create key 
    auto src_ip = ntohl(hdr->src_ip);
    auto dst_ip = ntohl(hdr->dst_ip);
    auto id = ntohs(hdr->identification);
    auto protocol = hdr->protocol;
    auto flag_and_fragoffset = ntohs(hdr->flag_and_fragoffset);
    uint16_t fragoffset = uint16_t(flag_and_fragoffset << 3) >> 3;
    auto key = ip_defrag_key::ptr(new ip_defrag_key(src_ip, dst_ip, id, protocol));
    auto offset_map = defrag_find(key);
    if (offset_map == nullptr) {
        offset_map = defrag_list_create(key);
    }
    // get mutex
    std::unique_lock<std::shared_mutex> lock(mutex);
    offset_map->insert(std::make_pair(fragoffset, buffer));
    auto defrag_buffer = ip_defrag_reassemble(offset_map);
    if (defrag_buffer == nullptr)
        return nullptr;
    lock.unlock();
    // remove map
    defrag_remove(key);
    return defrag_buffer;
}

// remove defrag
bool ip_defrag_queue::defrag_remove(ip_defrag_key::ptr key) {
    std::lock_guard<std::shared_mutex> lock(mutex);
    defrag_map.erase(defrag_map.find(key));
    return false;
}

ip_defrag_queue::defrag_offset_map_ptr ip_defrag_queue::defrag_find(ip_defrag_key::ptr key) {
    std::shared_lock<std::shared_mutex> lock(mutex);
    auto elem = defrag_map.find(key);
    if (elem == defrag_map.end())
        return nullptr;
    return elem->second;
}

// get defrag list
ip_defrag_queue::defrag_offset_map_ptr ip_defrag_queue::defrag_list_create(ip_defrag_key::ptr key) {
    std::lock_guard<std::shared_mutex> lock(mutex);
    auto offset_map = defrag_offset_map_ptr(new std::unordered_map<uint16_t, flow::sk_buff::ptr>());
    defrag_map.insert(std::make_pair(key, offset_map));
    return offset_map;
}

// try to reassemble bufer
flow::sk_buff::ptr ip_defrag_queue::ip_defrag_reassemble(defrag_offset_map_ptr offset_map) {
    // try to get offset 0, if not exist, if not complete
    auto offset_frag = offset_map->find(0);
    if (offset_frag == offset_map->end()) {
        std::cout << "ip first frag cant found, need more frag" << std::endl;
        return nullptr;
    }
    auto full_frag = false;
    auto total_buf_len = 0;
    // check if recv all frag
    while (true) {
        auto hdr = reinterpret_cast<flow::ip_hdr*>(offset_frag->second->get_data());
        // get frag
        auto flag_and_fragoffset = ntohs(hdr->flag_and_fragoffset);
        auto more_flag = flag_and_fragoffset >> def::ip_flag_offset & 0b001;
        // get current offset
        uint16_t offset = uint16_t(flag_and_fragoffset << 3) >> 3;
        // get header len
        auto header_len = (hdr->version_and_head_len & 0b00001111) * def::ip_len;
        // get data len 
        auto data_len = ntohs(hdr->total_len) - header_len;
        if (more_flag == 0) {
            full_frag = true;
            total_buf_len = offset * def::ip_frag_offset_base + data_len;
            std::cout << "ip rcv frag complete" << std::endl;
            break;
        }
        // get next offset
        offset += data_len / def::ip_frag_offset_base;
        offset_frag = offset_map->find(offset);
        if (offset_frag == offset_map->end()) {
            std::cout << "ip rcv frag not complete, need more frag" << std::endl;
            return nullptr;
        }
    }
    // check if full frag set
    if (!full_frag)
        return nullptr;
    // create buffer
    auto reassemble_buffer = flow::sk_buff::alloc(total_buf_len);
    reassemble_buffer->data_len = total_buf_len;
    flow::skb_pull(reassemble_buffer, sizeof(struct flow::ip_hdr));
    // copy buffer
    offset_frag = offset_map->find(0);
    // check if recv all frag
    while (true) {
        auto hdr = reinterpret_cast<flow::ip_hdr*>(offset_frag->second->get_data());
        // get frag
        auto flag_and_fragoffset = ntohs(hdr->flag_and_fragoffset);
        // get current offset
        uint16_t offset = uint16_t(flag_and_fragoffset << 3) >> 3;
        // get header len
        auto header_len = (hdr->version_and_head_len & 0b00001111) * def::ip_len;
        auto data_len = ntohs(hdr->total_len) - header_len;
        auto frag_buffer = offset_frag->second;
        flow::skb_pull(frag_buffer, header_len);
        memccpy(reassemble_buffer->get_data(), frag_buffer->get_data(), offset * def::ip_frag_offset_base, data_len);
        // check if is last frag
        auto more_flag = flag_and_fragoffset >> def::ip_flag_offset & 0b001;
        if (!more_flag) {
            reassemble_buffer->src = htonl(hdr->src_ip);
            reassemble_buffer->dst = htonl(hdr->dst_ip);
            std::cout << "ip rcv frag copy data complete" << std::endl;
            return reassemble_buffer;
        }
        offset += data_len / def::ip_frag_offset_base;
        offset_frag = offset_map->find(offset);
        if (offset_frag == offset_map->end()) {
            std::cout << "ip rcv frag copy failed" << std::endl;
            return nullptr;
        }
    }
    return nullptr;
}

}