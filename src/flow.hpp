#ifndef __FLOW_H__
#define __FLOW_H__

#include "def.hpp"

#include <cstddef>
#include <cstdint>
#include <memory>

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
    uint16_t head;
    // block tail
    uint16_t tail;

    /// data begin
    uint16_t begin;
    /// data end
    uint16_t end;

    /// pre buffer elem
    sk_buff::weak_ptr pre;
    /// next buffer elem
    sk_buff::ptr next;
    
    /// next layer protocol
    uint8_t protocol;

    /// data 
    char data[0];
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
     * @param elem sk buff
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
     * @return sk buff ptr, nullptr if empty
     * @param elem sk buff
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
struct ether_hr {
    /// src mac address
    uint8_t dst[6];
    /// dst mac address
    uint8_t src[6];
    /// next layer protocol
    uint16_t protocol;
};


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
    uint16_t src_ip;
    /// dst mac address
    uint8_t dst_mac[def::mac_len];
    /// dst ip address
    uint16_t dst_ip;
};


/**
 * @brief push buffer data
 * @param[in] buffer buffer
 * @param[in] device device
 */
void skb_push(sk_buff::ptr buffer, size_t offset);


void skb_put(sk_buff::ptr, size_t offset);


void skb_pull(sk_buff::ptr, size_t offset);

void skb_reserve(sk_buff::ptr, size_t offset);
}



#endif // __FLOW_H__