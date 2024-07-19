#ifndef __UDP_H__
#define __UDP_H__

#include "def.hpp"
#include "flow.hpp"
#include "interface.hpp"

#include <atomic>
#include <memory>

namespace protocol {

/**
 * @file udp.h
 * @brief handle udp flow
 * @author ArisAachen
 * @copyright Copyright (c) 2024 aris All rights reserved
 */
class udp : public interface::transport_handler {
public:
    typedef std::shared_ptr<udp> ptr;
    /**
     * @brief not allow to create udp obj
     */
    udp() = delete;

    /**
     * @brief create udp obj
     * @param[in] stack sk buffer
     * @return return if package is valid, like checksum failed
     */
    static udp::ptr create(interface::stack::weak_ptr stack);

    /**
     * @brief release udp obj
     */
    virtual ~udp() {}

    /**
     * @brief package protocol
     * @return return transport layer protocol
     */    
    virtual def::transport_protocol get_protocol();

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

    /**
     * @brief write buffer to sock
     * @param[in] skb sk buffer
     * @return return if package is valid, like checksum failed
     */ 
    virtual bool write_buffer_to_sock(flow::sk_buff::ptr buffer);

private:
    /**
     * @brief create udp with stack
     * @param[in] stack sk buffer
     * @return return if package is valid, like checksum failed
     */
    udp(interface::stack::weak_ptr stack);

    /**
     * @brief handle udp request
     * @param[in] skb sk buffer
     * @return return if package is valid, like checksum failed
     */    
    bool udp_rcv(flow::sk_buff::ptr buffer);

    /**
     * @brief handle udp response
     * @param[in] skb sk buffer
     * @return return if package is valid, like checksum failed
     */
    bool udp_send(flow::sk_buff::ptr buffer);

private:
    /// stack
    interface::stack::weak_ptr stack_;
    /// identification
    std::atomic<uint16_t> identification_;
};

}

#endif // __IP_H__