#ifndef __FLOW_H__
#define __FLOW_H__

#include "def.hpp"

#include <algorithm>
#include <any>
#include <array>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <memory>
#include <variant>

#include <netinet/in.h>
#include <vector>

// pre define interface
namespace interface {
    struct net_device;
    class stack;
}

// sock key
namespace flow_table {
    struct sock_key;
}

namespace flow {

/**
 * @file flow.hpp
 * @brief sk buff to store flow
 * @author ArisAachen
 * @copyright Copyright (c) 2024 aris All rights reserved
 */
struct sk_buff {
    // weak pointer
    typedef std::weak_ptr<sk_buff> weak_ptr;
    // share pointer
    typedef std::shared_ptr<sk_buff> ptr;

    /// include header len and data len
    uint64_t total_len;
    /// data len
    uint64_t data_len;

    /// block head
    uint16_t block_head;
    // block end
    uint16_t block_end;

    /// data begin
    uint16_t data_begin;
    /// data tail
    uint16_t data_tail;

    /// pre buffer elem
    sk_buff::weak_ptr pre;
    /// next buffer elem
    sk_buff::ptr next;
    
    /// next layer protocol
    uint16_t protocol;

    /// device mtu and path mtu
    uint16_t mtu;
    
    /// other info
    std::any info;

    /// write dst 
    std::variant<uint32_t, std::array<uint8_t, def::mac_len>> dst;

    /// recv src 
    std::variant<uint32_t, std::array<uint8_t, def::mac_len>> src;

    /// sock key
    std::shared_ptr<flow_table::sock_key> key;

    /// recv netdevice 
    std::weak_ptr<interface::net_device> dev;

    /// stack 
    std::weak_ptr<interface::stack> stack;

    /// ip fragment
    std::weak_ptr<sk_buff> parent_frag;
    /// children fragments
    std::vector<sk_buff::ptr> child_frags;

    /// data 
    char data[0];

    /**
     * @brief alloc data size buffer
     * @return sk_buff::ptr sk buff ptr
     */    
    static sk_buff::ptr alloc(size_t size) {
        sk_buff::ptr buffer = sk_buff::ptr(new (size) sk_buff);
        buffer->data_begin = 0;
        buffer->data_tail = 0;
        buffer->data_len = 0;
        buffer->block_head = 0;
        buffer->block_end = size;
        buffer->total_len = sizeof(struct sk_buff) + size;
        return buffer;
    }

    /**
     * @brief store data
     * @param[in] buf data
     * @param[in] size data size
     */
    void store_data(char* buf, size_t size) {
        std::copy(buf, buf + size, data + data_begin);
        data_len += size;
    }

    /**
     * @brief store data
     * @param[in] buf data
     * @param[in] size data size
     */
    void append_data(char* buf, size_t size) {
        std::copy(buf, buf + size, data + data_tail);
        data_len += size;
        // check if need update tail
        data_tail += size;
    }

    /**
     * @brief get data
     * @return data or nullptr
     */    
    char* get_data() {
        if (data_len == 0)
            return nullptr;
        return data + data_begin;
    }

    /**
     * @brief get data
     * @return get data len
     */   
    uint16_t get_data_len() const {
        return data_tail - data_begin;
    }

    /**
     * @brief release buff
     */  
    virtual ~sk_buff() {}

private:
    /**
     * @brief get data
     * @return data or nullptr
     */    
    void* operator new(std::size_t sk_size, uint16_t alloc_size) {
        auto buffer = malloc(sk_size + alloc_size);
        memset(buffer, 0, sk_size + alloc_size);
        return buffer;
    }
};

/**
 * @file flow.hpp
 * @brief flow head and tail
 * @author ArisAachen
 * @copyright Copyright (c) 2024 aris All rights reserved
 */
struct sk_head {
public:
    typedef std::shared_ptr<sk_head> ptr;
    sk_head(): head(nullptr), tail(nullptr) {}
    virtual ~sk_head() {}

    /**
     * @brief append sk buff to tail
     * @param[in] elem sk buff
     */
    void append(sk_buff::ptr elem) {
        if (head == nullptr) {
            head = elem;
            tail = elem;
        } else {
            tail->next = elem;
            elem->pre = tail;
            tail = elem;
        }
    }

    /**
     * @brief pop sk buff from head
     * @return sk_buff::ptr sk buff ptr, nullptr if empty
     */
    sk_buff::ptr pop() {
        if (head == nullptr) {
            return nullptr;
        }
        sk_buff::ptr elem = head;
        head = head->next;
        if (head != nullptr) {
            head->pre.reset();
        }
        elem->next.reset();
        return elem;
    }

    /**
     * @brief empty check
     * @return true if empty, false if not empty
     */
    bool empty() {
        return head == nullptr;
    }


private:
    /// sk buff head
    sk_buff::ptr head;
    /// sk buff tail
    sk_buff::ptr tail;
};




/**
 * @file flow.hpp
 * @brief ether layer header
 * @author ArisAachen
 * @copyright Copyright (c) 2024 aris All rights reserved
 */
struct ether_hdr {
    /// src mac address
    uint8_t dst[def::mac_len];
    /// dst mac address
    uint8_t src[def::mac_len];
    /// next layer protocol
    uint16_t protocol;
} __attribute__((packed));


/**
 * @file flow.hpp
 * @brief arp layer header
 * @author ArisAachen
 * @copyright Copyright (c) 2024 aris All rights reserved
 */
struct arp_hdr {
    /// hardware type
    uint16_t hardware_type;
    /// arp request protocol, ip or ipv6
    uint16_t protocol;
    /// hardware len
    uint8_t hardware_len;
    /// protocol len
    uint8_t protocol_len;
    /// operator code
    uint16_t operator_code;
    /// src mac address
    uint8_t src_mac[def::mac_len];
    /// src ip address
    uint32_t src_ip;
    /// dst mac address
    uint8_t dst_mac[def::mac_len];
    /// dst ip address
    uint32_t dst_ip;
} __attribute__((packed));

/**
 * @file flow.hpp
 * @brief ip header
 * @author ArisAachen
 * @copyright Copyright (c) 2024 aris All rights reserved
 */
struct ip_hdr {
    // /// version
    // uint8_t version:4;
    // /// ip head length
    // uint8_t ip_head_len:4;
    uint8_t version_and_head_len;
    /// diff service
    // uint8_t diff_serv:6;
    // /// ecn 
    // uint8_t ecn:2;
    uint8_t diff_serv_and_ecn;
    /// ip fragment length
    uint16_t total_len;
    /// ip identification
    uint16_t identification;
    // /// ip flag
    // uint8_t flags:3;
    // /// fragment offset
    // uint16_t fragment_offset:13;
    /// flag and offset
    uint16_t flag_and_fragoffset;
    /// time to live
    uint8_t time_to_live;
    /// next layer protocol
    uint8_t protocol;
    /// head checksum
    uint16_t head_checksum;
    /// source ip
    uint32_t src_ip;
    /// dst ip
    uint32_t dst_ip;
} __attribute__((packed));

/**
 * @file flow.hpp
 * @brief icmp header
 * @author ArisAachen
 * @copyright Copyright (c) 2024 aris All rights reserved
 * @link https://datatracker.ietf.org/doc/html/rfc792
 */
struct icmp_hdr {
    /// icmp header
    uint8_t icmp_type;
    /// icmp code
    uint8_t icmp_code;
    /// icmp checksum
    uint16_t icmp_checksum;
    /// extend data
    char data[0];
} __attribute__((packed));


/**
 * @file flow.hpp
 * @brief icmp echo header
 * @author ArisAachen
 * @copyright Copyright (c) 2024 aris All rights reserved
 * @link https://datatracker.ietf.org/doc/html/rfc792
 */
struct icmp_echo_body {
    /// identifier id 
    uint16_t identifier;
    /// sequence number
    uint16_t sequence_number;
    /// extend data
    char data[0];
} __attribute__((packed));

/**
 * @file flow.hpp
 * @brief udp header
 * @author ArisAachen
 * @copyright Copyright (c) 2024 aris All rights reserved
 * @link https://datatracker.ietf.org/doc/html/rfc792
 */
struct udp_hdr {
    /// source port
    uint16_t src_port;
    /// dst port
    uint16_t dst_port;
    /// total length
    uint16_t total_len;
    /// udp total checksum
    uint16_t udp_checksum;
    char data[0];
} __attribute__((packed));

/**
 * @file flow.hpp
 * @brief udp header
 * @author ArisAachen
 * @copyright Copyright (c) 2024 aris All rights reserved
 * @link https://datatracker.ietf.org/doc/html/rfc792
 */
struct udp_fake_hdr {
    /// source ip
    uint32_t src_ip;
    /// dst ip
    uint32_t dst_ip;
    /// reserve 0
    uint8_t reserve;
    /// udp protocol
    uint8_t protocol;
    /// total len
    uint16_t total_len;
};

/**
 * @brief push buffer begin
 * @param[in] buffer buffer
 * @param[in] device device
 */
static void skb_push(sk_buff::ptr buffer, size_t offset) {
    buffer->data_begin -= offset;
}

/**
 * @brief put buffer data end
 * @param[in] buffer buffer
 * @param[in] offset offset
 */
static void skb_put(sk_buff::ptr buffer, size_t offset) {
    buffer->data_tail += offset;
}

/**
 * @brief pull buffer data head
 * @param[in] buffer buffer
 * @param[in] offset offset
 */
static void skb_pull(sk_buff::ptr buffer, size_t offset) {
    buffer->data_begin += offset;
}

/**
 * @brief reserve buffer data begin and end
 * @param[in] buffer buffer
 * @param[in] offset offset
 */
static void skb_reserve(sk_buff::ptr buffer, size_t offset) {
    buffer->data_begin += offset;
    buffer->data_tail += offset;
}

/**
 * @brief reset buffer 
 * @param[in] buffer buffer
 * @param[in] len len
 */
static void skb_reset(sk_buff::ptr buffer, size_t len) {
    memset(buffer->get_data(), 0, len);
}

/**
 * @brief release buffer
 * @param[in] buffer buffer
 */
static void skb_release(sk_buff::ptr buffer) {
    buffer.reset();
    return;
}

/**
 * @brief copy header buffer
 * @param[in] src source buffer
 * @param[in] dst dst buffer
 */
static void skb_header_clone(sk_buff::ptr src, sk_buff::ptr dst) {
    dst->protocol = src->protocol;
    dst->mtu = src->mtu;
    dst->src = src->src;
    dst->dst = src->dst;
    dst->key = src->key;
    dst->dev = src->dev;
    dst->stack = src->stack;
}

/**
 * @brief compute checksum
 * @param[in] buffer buffer
 */
static uint16_t compute_checksum(const sk_buff::ptr buffer) {
    // check if need append 0 to tail
    auto remain = buffer->get_data_len() % def::checksum_div_base;
    // TODO: check if need append 0
    if (remain != 0)
        std::cout << "checksum need append, count: " << remain << std::endl;
    auto div = buffer->get_data_len() / def::checksum_div_base;
    uint16_t sum = 0;
    // add all bytes
    for (uint16_t index = 0; index < div; index++) {
        auto num_ptr = reinterpret_cast<uint16_t*>(buffer->get_data() + index * def::checksum_div_base);
        // should convert to host
        auto num = ntohs(*num_ptr);
        if (def::checksum_max_num - sum > num)
            sum += num;
        else
            sum += num + 1;
    }
    return (~sum & def::checksum_max_num);
}

/**
 * @brief get ether offset
 * @return get ether offset
 */
static uint16_t get_ether_offset() {
    return sizeof(uint16_t) + def::mac_len * 2;
}

/**
 * @brief get ip offset 
 * @return get ip offset
 */
static uint16_t get_ip_offset() {
    return get_ether_offset() + sizeof(struct ip_hdr);
}

/**
 * @brief get icmp offset 
 * @return get icmp offset
 */
static uint16_t get_icmp_offset() {
    return get_ip_offset() + sizeof(struct icmp_hdr);
}

/**
 * @brief get icmp offset 
 * @return get icmp offset
 */
static uint16_t get_icmp_echo_offset() {
    return get_icmp_offset() + sizeof(struct icmp_echo_body);
}

/**
 * @brief get max tcp data offset
 * @return get max tcp data offset
 */
static uint16_t get_max_tcp_data_offset() {
    auto offset = def::max_ether_header + def::max_ip_header + def::max_tcp_header;
    return offset;
}

/**
 * @brief get max udp data offset
 * @return get max udp data offset
 */
static uint16_t get_max_udp_data_offset() {
    auto offset = def::max_ether_header + def::max_ip_header + def::max_udp_header;
    return offset;
}

}



#endif // __FLOW_H__