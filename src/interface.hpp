#ifndef __INTERFACE_H__
#define __INTERFACE_H__

#include "flow.hpp"

#include <memory>

namespace interface {

struct net_device {
    typedef std::shared_ptr<net_device> ptr;

    /**
     * @brief up net_device device
     * @return 0 success, -1 fail
     */
    virtual bool up() = 0;

    /**
     * @brief down net_device device
     * @return 0 success, -1 fail
     */
    virtual bool down() = 0;

    /**
     * @brief read from net_device device
     * @return read buffer
     */
    virtual flow::sk_buff::ptr read_from_device() = 0;

    /**
     * @brief write to net_device device
     * @param[in] buf write buffer
     * @param[in] len write buffer length
     * @return write buffer length
     */
    virtual int write_to_device(flow::sk_buff::ptr buffer) = 0;


public:
    /**
     * @brief read from net_device device
     * @param[in] buf net_device device 
     */
    virtual void read_thread() = 0;

    /**
     * @brief write to net_device device
     * @param[in] buf net_device device 
     */
    virtual void write_thread() = 0;

    /**
     * @brief get net_device device status from kernel status
     * @return bool net_device device status
     */
    virtual bool kernel_device_status() = 0;

    /**
     * @brief get net_device device status from user status
     * @return bool net_device device status
     */
    virtual bool user_device_status() = 0;
};

struct network_handler {
    typedef std::shared_ptr<network_handler> ptr;

    /**
     * @brief package flow
     * @param[in] skb sk buffer
     * @return return if package is valid, like checksum failed
     */    
    virtual bool pack_flow(flow::sk_buff::ptr skb) = 0;

    /**
     * @brief unpackage flow
     * @param[in] skb sk buffer
     * @return return if package is valid, like checksum failed
     */        
    virtual bool unpack_flow(flow::sk_buff::ptr skb) = 0;
};

}

#endif // __INTERFACE_H__