#ifndef __ARP_H__
#define __ARP_H__

#include "flow.hpp"
#include "interface.hpp"

#include <memory>
namespace protocol {

/**
 * @file arp.h
 * @brief handle arp flow
 * @author ArisAachen
 * @copyright Copyright (c) 2024 aris All rights reserved
 */
class arp : public interface::network_handler {
public:
    typedef std::shared_ptr<arp> ptr;
    /**
     * @brief not allow to create arp obj
     */
    arp() {};

    /**
     * @brief release arp obj
     */
    virtual ~arp() {}

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
     * @brief handle arp request
     * @param[in] skb sk buffer
     * @return return if package is valid, like checksum failed
     */    
    bool handle_arp_request(flow::sk_buff::ptr buffer);
};

}




#endif // __ARP_H__