#ifndef __MACVLAN_H__
#define __MACVLAN_H__

#include "def.hpp"
#include "flow.hpp"
#include "interface.hpp"


#include <condition_variable>
#include <cstdint>
#include <queue>
#include <string>
#include <thread>
#include <vector>

namespace driver {

/**
 * @file macvlan_device.hpp
 * @brief read and write skbuf to macvlan_device device
 * @author ArisAachen
 * @copyright Copyright (c) 2024 aris All rights reserved
 */
class macvlan_device: public interface::net_device, public std::enable_shared_from_this<macvlan_device> {
public:
    typedef std::shared_ptr<macvlan_device> ptr;

    /**
     * @brief Construct a new macvlan_device device object
     * @param[in] dev_name macvlan_device device name
     */
    macvlan_device(const std::string& dev_name, const std::string& ip_address = "", const std::string& mac_address = "");

    /**
     * @brief Destroy the macvlan_device device object
     */
    virtual ~macvlan_device();

    /**
     * @brief up macvlan_device device
     * @return 0 success, -1 fail
     */
    virtual bool up();

    /**
     * @brief down macvlan_device device
     * @return 0 success, -1 fail
     */
    virtual bool down();

    /**
     * @brief read from macvlan_device device
     * @return read buffer
     */
    virtual flow::sk_buff::ptr read_from_device();

    /**
     * @brief write to macvlan_device device
     * @param[in] buf write buffer
     * @param[in] len write buffer length
     * @return write buffer length
     */
    virtual int write_to_device(flow::sk_buff::ptr buffer);

    /**
     * @brief get net_device mac
     * @return device mac
     */
    virtual uint8_t* get_device_mac();

    /**
     * @brief get net_device ip
     * @return device ip
     */
    virtual uint32_t get_device_ip();

    /**
     * @brief get net_device ifindex
     * @return device ifindex
     */
    virtual uint8_t get_device_ifindex();

public:
    /**
     * @brief read from macvlan_device device
     * @param[in] buf macvlan_device device 
     */
    virtual void read_thread();

    /**
     * @brief write to macvlan_device device
     * @param[in] buf macvlan_device device 
     */
    virtual void write_thread();

    /**
     * @brief get macvlan_device device status from kernel status
     * @return bool macvlan_device device status
     */
    virtual bool kernel_device_status() { return false; };

    /**
     * @brief get macvlan_device device status from user status
     * @return bool macvlan_device device status
     */
    virtual bool user_device_status() { return false; };

private:
    /// device name
    std::string dev_name_;
    /// device address
    uint32_t ip_address_;
    /// device mac 
    uint8_t mac_address_[def::mac_len];
    /// device index
    uint8_t if_index_;
    /// thread to read and write 
    std::vector<std::thread> thread_vec_;
    /// macvlan_device device fd
    uint8_t macvlan_fd_;
    /// macvlan_device device mtu
    uint16_t mtu_;
    /// read buffer head
    std::queue<flow::sk_buff::ptr> read_head_;
    /// read share mutex
    std::mutex read_mutex_;
    /// read share condition
    std::condition_variable read_cond_;
    /// write buffer head
    std::queue<flow::sk_buff::ptr> write_head_;
    /// write share mutex
    std::mutex write_mutex_;
    /// write share condition
    std::condition_variable write_cond_;
    /// device status
    def::device_status status_;
};


}

#endif // __MACVLAN_H__