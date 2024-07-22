#ifndef __IP_H__
#define __IP_H__

#include "def.hpp"
#include "flow.hpp"
#include "interface.hpp"

#include <atomic>
#include <memory>

namespace protocol {

/**
 * @file ip.h
 * @brief handle ip flow
 * @author ArisAachen
 * @copyright Copyright (c) 2024 aris All rights reserved
 */
class ip : public interface::network_handler {
public:
    typedef std::shared_ptr<ip> ptr;
    /**
     * @brief not allow to create ip obj
     */
    ip() = delete;

    /**
     * @brief create ip obj
     * @param[in] stack sk buffer
     * @return return if package is valid, like checksum failed
     */
    static ip::ptr create(interface::stack::weak_ptr stack);

    /**
     * @brief release ip obj
     */
    virtual ~ip() {}


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
     * @brief create ip with stack
     * @param[in] stack sk buffer
     * @return return if package is valid, like checksum failed
     */
    ip(interface::stack::weak_ptr stack);

    /**
     * @brief handle ip request
     * @param[in] skb sk buffer
     * @return return if package is valid, like checksum failed
     */    
    bool ip_rcv(flow::sk_buff::ptr buffer);

    /**
     * @brief handle ip response
     * @param[in] skb sk buffer
     * @return return if package is valid, like checksum failed
     */
    bool ip_send(flow::sk_buff::ptr buffer);

    /**
     * @brief make package flow
     * @param[in] skb sk buffer
     * @return return if package is valid, like checksum failed
     */
    bool ip_make_flow(flow::sk_buff::ptr buffer, size_t offset, bool more_flag);

    /**
     * @brief fragment ip
     * @param[in] skb sk buffer
     * @return return if package is valid, like checksum failed
     */
    bool ip_fragment(flow::sk_buff::ptr buffer);

private:
    /// stack
    interface::stack::weak_ptr stack_;
    /// identification
    std::atomic<uint16_t> identification_;
};

}

#endif // __IP_H__