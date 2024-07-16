#ifndef __ICMP_H__
#define __ICMP_H__

#include "def.hpp"
#include "flow.hpp"
#include "interface.hpp"

#include <atomic>
#include <cstdint>
#include <memory>

namespace protocol {

/**
 * @file icmp.h
 * @brief handle icmp flow
 * @author ArisAachen
 * @copyright Copyright (c) 2024 aris All rights reserved
 */
class icmp : public interface::network_handler {
public:
    typedef std::shared_ptr<icmp> ptr;
    /**
     * @brief not allow to create icmp obj
     */
    icmp() = delete;

    /**
     * @brief create icmp obj
     * @param[in] stack sk buffer
     * @return return if package is valid, like checksum failed
     */
    static icmp::ptr create(interface::stack::weak_ptr stack);

    /**
     * @brief release icmp obj
     */
    virtual ~icmp() {}


    /**
     * @brief package protocol
     * @return return network layer protocol
     */    
    virtual def::network_protocol get_protocol();

    /**
     * @brief package flow
     * @param[in] skb sk buffer
     * @return return if package is valid, like checksum failed
     */    
    virtual bool pack_flow(flow::sk_buff::ptr buffer);

    /**
     * @brief package flow
     * @param[in] skb sk buffer
     * @return return if package is valid, like checksum failed
     */    
    virtual bool unpack_flow(flow::sk_buff::ptr buffer);

private:
    /**
     * @brief create icmp with stack
     * @param[in] stack sk buffer
     * @return return if package is valid, like checksum failed
     */
    icmp(interface::stack::weak_ptr stack);

    /**
     * @brief handle icmp request
     * @param[in] skb sk buffer
     * @return return if package is valid, like checksum failed
     */    
    bool handle_icmp_echo_request(flow::sk_buff::ptr buffer);

    /**
     * @brief handle icmp reply
     * @param[in] skb sk buffer
     * @return return if package is valid, like checksum failed
     */    
    bool handle_icmp_echo_reply(flow::sk_buff::ptr buffer);

    /**
     * @brief send icmp request
     * @param[in] skb sk buffer
     * @return return if package is valid, like checksum failed
     */    
    bool send_icmp_echo_request(flow::sk_buff::ptr buffer);

    /**
     * @brief send icmp reply
     * @param[in] skb sk buffer
     * @return return if package is valid, like checksum failed
     */    
    bool send_icmp_echo_reply(flow::sk_buff::ptr buffer);

    /**
     * @brief handle icmp response
     * @param[in] skb sk buffer
     * @return return if package is valid, like checksum failed
     */
    bool icmp_send(flow::sk_buff::ptr buffer);

private:
    /// stack
    interface::stack::weak_ptr stack_;
    /// sequence number
    std::atomic<uint16_t> sequence_;
};

}

#endif // __ICMP_H__