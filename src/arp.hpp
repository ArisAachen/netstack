#ifndef __ARP_H__
#define __ARP_H__

#include "def.hpp"
#include "flow.hpp"
#include "interface.hpp"

#include <cstdint>
#include <memory>
#include <optional>
#include <shared_mutex>
#include <unordered_map>

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
    arp() = delete;

    /**
     * @brief create arp obj
     * @param[in] stack sk buffer
     * @return return if package is valid, like checksum failed
     */
    static arp::ptr create(interface::stack::weak_ptr stack);

    /**
     * @brief release arp obj
     */
    virtual ~arp() {}


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
     * @brief create arp with stack
     * @param[in] stack sk buffer
     * @return return if package is valid, like checksum failed
     */
    arp(interface::stack::weak_ptr stack);

    /**
     * @brief handle arp request
     * @param[in] skb sk buffer
     * @return return if package is valid, like checksum failed
     */    
    bool handle_arp_request(flow::sk_buff::ptr buffer);

    /**
     * @brief handle arp response
     * @param[in] skb sk buffer
     * @return return if package is valid, like checksum failed
     */
    bool handle_arp_response(flow::sk_buff::ptr buffer);

    /**
     * @brief send arp request
     * @param[in] ip ip address
     * @param[in] dev net device
     * @return return if package is valid, like checksum failed
     */
    bool send_arp_request(uint32_t ip, interface::net_device::ptr dev);

private:
    /// stack
    interface::stack::weak_ptr stack_;
};

}


namespace flow_table {

struct neighbor {
typedef std::shared_ptr<neighbor> ptr;
public:
    /**
     * @brief not allow create neighbor direct
     */
    neighbor() = delete;

    static neighbor::ptr create(uint32_t ip, const uint8_t* mac, interface::net_device::ptr dev) {
        auto neigh = neighbor::ptr(new neighbor(ip, mac, dev));
        return neigh;
    }

    /**
     * @brief release neighbor
     */
    virtual ~neighbor() {

    }

private:
    // create 
    neighbor(uint32_t ip, const uint8_t* mac, interface::net_device::ptr dev) {
        ip_address = ip;
        memccpy(mac_address, mac, 0, def::mac_len);
        device = dev;
    }

public:
    /// ip address
    uint32_t ip_address;
    /// store neigh mac
    uint8_t mac_address[def::mac_len];
    /// output device 
    interface::net_device::ptr device;
};

/**
 * @file arp.h
 * @brief ip neighbor table
 * @author ArisAachen
 * @copyright Copyright (c) 2024 aris All rights reserved
 */
class neighbor_table {
public:
    typedef std::shared_ptr<neighbor_table> ptr;

    /**
     * @brief create neighbor table
     */
    static neighbor_table::ptr create();

    /**
     * @brief release neighbor table
     */
    virtual ~neighbor_table();

    /**
     * @brief insert ip neighbor info
     * @param[in] key ip key 
     * @param[in] dev dev value
     */
    virtual void insert(uint32_t key, neighbor::ptr neigh, bool replace);

    /**
     * @brief remove ip neighbor info
     * @param[in] key ip key 
     */   
    virtual void remove(uint32_t key);

    /**
     * @brief handle arp request
     * @param[in] key ip key
     * @return return if neigh device exist
     */
    virtual std::optional<neighbor::ptr> get(uint32_t key);

private:    
    /**
     * @brief create neighbor table
     */
    neighbor_table();

private:
    /// share lock 
    std::shared_mutex mutex_;
    /// neighbor table
    std::unordered_map<uint32_t, neighbor::ptr> neigh_map_;
};


}



#endif // __ARP_H__