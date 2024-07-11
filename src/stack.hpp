#ifndef __STACK_H__
#define __STACK_H__


#include "interface.hpp"

#include <cstdint>
#include <memory>
#include <thread>
#include <unordered_map>
#include <vector>

namespace stack {


/**
 * @file stack.hpp
 * @brief stack protocol stack, include tcp/ip, arp, icmp, etc
 * @author ArisAachen
 * @copyright Copyright (c) 2024 aris All rights reserved
 */
class stack {
public:
    typedef std::shared_ptr<stack> ptr;

    /**
     * @brief get stack instance
     * @return stack instance
     */
    static stack::ptr get_instance();

    /**
     * @brief register device to stack
     * @param[in] device_id device id
     * @param[in] device device
     */
    void register_device(uint8_t device_id, interface::net_device::ptr device) {
        device_map_.insert(std::make_pair(device_id, device));
    }

    /**
     * @brief register network handler to stack
     * @param[in] device_id device id
     * @param[in] handler network handler
     */
    void register_network_handler(uint8_t handler_id, interface::network_handler::ptr handler) {
        network_handler_map_.insert(std::make_pair(handler_id, handler));
    }

    /**
     * @brief write to device
     * @param[in] buffer write buffer
     * @param[in] device_id device id
     */
    void write_to_device(flow::sk_buff::ptr buffer, uint8_t device_id);

    /**
     * @brief read and handle buffer
     */
    void run();

    /**
     * @brief release stack
     */
    virtual ~stack();

private:
    /**
     * @brief create stack
     */
    stack();

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
    void handle_network_package(uint8_t protocol, flow::sk_buff::ptr buffer);

private:
    /// network handler map
    std::unordered_map<uint8_t, interface::network_handler::ptr> network_handler_map_;
    /// device map
    std::unordered_map<uint8_t, interface::net_device::ptr> device_map_;
    /// thread vector
    std::vector<std::thread> thread_vec_;
};

}

#endif // __STACK_H__