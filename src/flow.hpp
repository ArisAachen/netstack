#ifndef __FLOW_H__
#define __FLOW_H__

#include "def.hpp"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <memory>

// pre define interface
namespace interface {
    struct net_device;
    class stack;
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

    /// write dst 
    union {
        uint32_t ip_address;
        uint8_t mac_address[def::mac_len];
    } dst;

    /// recv netdevice 
    std::weak_ptr<interface::net_device> dev;

    /// stack 
    std::weak_ptr<interface::stack> stack;

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
        // check if need update tail
        if (data_begin + size > data_tail)
            data_tail = data_begin + size;
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
        return malloc(sk_size + alloc_size);
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
    uint8_t dst[6];
    /// dst mac address
    uint8_t src[6];
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
    /// version
    uint8_t version:4;
    /// ip head length
    uint8_t ip_head_len:4;
    /// diff service
    uint8_t diff_serv:6;
    /// ecn 
    uint8_t ecn:2;
    /// ip fragment length
    uint16_t total_len;
    /// ip identification
    uint16_t identification;
    /// ip flag
    uint8_t flags:3;
    /// fragment offset
    uint16_t fragment_offset:13;
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
 * @brief release buffer
 * @param[in] buffer buffer
 */
static void skb_release(sk_buff::ptr buffer) {
    buffer.reset();
    return;
}

/**
 * @brief get ether offset
 * @return get ether offset
 */
static uint16_t get_ether_offset() {
    return sizeof(struct ether_hdr);
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

}



#endif // __FLOW_H__