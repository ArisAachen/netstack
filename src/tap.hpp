#ifndef __TAP_H__
#define __TAP_H__

#include "def.hpp"

#include <any>
#include <condition_variable>
#include <list>
#include <shared_mutex>
#include <string>
#include <thread>
#include <vector>

namespace driver {

/**
 * @file tap.hpp
 * @brief read and write skbuf to tap device
 * @author ArisAachen
 * @copyright Copyright (c) 2024 aris All rights reserved
 */
class tap_device: public std::enable_shared_from_this<tap_device> {
public:
    typedef std::shared_ptr<tap_device> ptr;

    /**
     * @brief Construct a new tap device object
     * @param[in] dev_name tap device name
     */
    tap_device(const std::string& dev_name, const std::string& ip_address = "", const std::string& mac_address = "");

    /**
     * @brief Destroy the tap device object
     */
    virtual ~tap_device();

    /**
     * @brief up tap device
     * @return 0 success, -1 fail
     */
    bool up();

    /**
     * @brief down tap device
     * @return 0 success, -1 fail
     */
    bool down();

    /**
     * @brief read from tap device
     * @param[out] buf read buffer
     * @param[in] len read buffer length
     * @return read buffer length
     */
    int read_from_device(void* buf, int len);

    /**
     * @brief write to tap device
     * @param[in] buf write buffer
     * @param[in] len write buffer length
     * @return write buffer length
     */
    int write_to_device(const void* buf, int len);


public:
    /**
     * @brief read from tap device
     * @param[in] buf tap device 
     */
    void read_thread();

    /**
     * @brief write to tap device
     * @param[in] buf tap device 
     */
    void write_thread();

    /**
     * @brief get tap device status from kernel status
     * @return bool tap device status
     */
    bool kernel_device_status();

    /**
     * @brief get tap device status from user status
     * @return bool tap device status
     */
    bool user_device_status();

private:
    /// device name
    std::string dev_name_;
    /// device address
    std::string ip_address_;
    /// device mac 
    std::string mac_address_;
    /// thread to read and write 
    std::vector<std::thread> thread_vec_;
    /// tap device fd
    uint8_t tap_fd_;
    /// tap device mtu
    uint16_t mtu_;
    /// read buffer
    std::list<std::any> read_buf_;
    /// read share mutex
    std::shared_mutex read_mutex_;
    /// read share condition
    std::condition_variable read_cond_;
    /// write buffer
    std::list<std::any> write_buf_;
    /// write share mutex
    std::shared_mutex write_mutex_;
    /// write share condition
    std::condition_variable write_cond_;
    /// device status
    def::device_status status_;
};


}

#endif // __TAP_H__