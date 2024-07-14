#ifndef __STACK_H__
#define __STACK_H__


#include "arp.hpp"
#include "def.hpp"
#include "flow.hpp"
#include "interface.hpp"

#include <cstdint>
#include <memory>
#include <thread>
#include <unordered_map>
#include <vector>

namespace stack {


/**
 * @file raw_stack.hpp
 * @brief raw_stack protocol raw_stack, include tcp/ip, arp, icmp, etc
 * @author ArisAachen
 * @copyright Copyright (c) 2024 aris All rights reserved
 */
class raw_stack : public interface::stack, public std::enable_shared_from_this<raw_stack> {
public:
    typedef std::shared_ptr<raw_stack> ptr;

    /**
     * @brief get raw_stack instance
     * @return raw_stack instance
     */
    static raw_stack::ptr get_instance();

    /**
     * @brief register device to raw_stack
     * @param[in] device device
     */
    virtual void register_device(interface::net_device::ptr device) {
        device_map_.insert(std::make_pair(device->get_device_ifindex(), device));
    }

    /**
     * @brief register network handler to raw_stack
     * @param[in] handler network handler
     */
    virtual void register_network_handler(interface::network_handler::ptr handler) {
        network_handler_map_.insert(std::make_pair(handler->get_protocol(), handler));
    }

    /**
     * @brief write to device
     * @param[in] buffer write buffer
     * @param[in] device_id device id
     */
    virtual void write_to_device(flow::sk_buff::ptr buffer);

    /**
     * @brief update neighbor info
     * @param[in] hdr arp hdr
     */
    virtual void update_neighbor(const struct flow::arp_hdr* hdr, interface::net_device::ptr dev);

    /**
     * @brief read and handle buffer
     */
    virtual void run();

    /**
     * @brief wait signal to end
     */
    virtual void wait();

    /**
     * @brief release raw_stack
     */
    virtual ~raw_stack();

private:
    /**
     * @brief create raw_stack
     */
    raw_stack();

    /**
     * @brief read device
     */
    void run_read_device();

    /**
     * @brief parse packet
     */
    void handle_packege();

    /**
     * @brief write to device
     * @param[in] protocol ether protocol
     * @param[in] buffer buffer remove ether header
     */
    void handle_network_package(flow::sk_buff::ptr buffer);

private:
    /// network handler map
    std::unordered_map<def::network_protocol, interface::network_handler::ptr> network_handler_map_;
    /// device map
    std::unordered_map<uint8_t, interface::net_device::ptr> device_map_;
    /// thread vector
    std::vector<std::thread> thread_vec_;
    /// neighbor flow 
    flow_table::neighbor_table::ptr neighbor_table_;
};

}

#endif // __STACK_H__