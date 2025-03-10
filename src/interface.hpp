#ifndef __INTERFACE_H__
#define __INTERFACE_H__

#include "def.hpp"
#include "flow.hpp"

#include <any>
#include <cstdint>
#include <memory>
#include <optional>
#include <sys/socket.h>


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

    /**
     * @brief get net_device mac
     * @return device mac
     */
    virtual uint8_t* get_device_mac() = 0;

    /**
     * @brief get net_device ip
     * @return device ip
     */
    virtual uint32_t get_device_ip() = 0;

    /**
     * @brief get net_device ifindex
     * @return device ifindex
     */
    virtual uint8_t get_device_ifindex() = 0;

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
     * @brief package protocol
     * @return return network layer protocol
     */    
    virtual def::network_protocol get_protocol() = 0;

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

struct transport_handler {
    typedef std::shared_ptr<transport_handler> ptr;

    /**
     * @brief package protocol
     * @return return network layer protocol
     */    
    virtual def::transport_protocol get_protocol() = 0;

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

struct sock_handler {
    typedef std::shared_ptr<sock_handler> ptr;

    /**
     * @brief create sock fd
     * @param[in] domain sock domain,
     * @param[in] type sock type,
     * @param[in] protocol sock protocol,
     * @return sock fd
     */
    virtual bool sock_create(std::shared_ptr<flow_table::sock_key> key, def::transport_sock_type type) = 0;

    /**
     * @brief connect sock fd
     * @param[in] key sock fd,
     * @param[in] addr remote addr,
     * @param[in] len addr len,
     * @return sock fd
     */
    virtual bool connect(std::shared_ptr<flow_table::sock_key> key, struct sockaddr* addr, socklen_t len) = 0;

    /**
     * @brief close sock fd
     * @param[in] key sock fd,
     * @return sock fd
     */
    virtual bool close(std::shared_ptr<flow_table::sock_key> key) = 0;

    /**
     * @brief bind sock fd
     * @param[in] key sock fd,
     * @param[in] addr remote addr,
     * @param[in] len addr len,
     * @return sock fd
     */
    virtual bool bind(std::shared_ptr<flow_table::sock_key> key, struct sockaddr* addr, socklen_t len) = 0;

    /**
     * @brief listen sock fd
     * @param[in] key sock fd,
     * @param[in] backlog addr len,
     * @return sock fd
     */
    virtual bool listen(std::shared_ptr<flow_table::sock_key> key, int backlog) = 0;

    /**
     * @brief accept sock fd
     * @param[in] key sock fd,
     * @param[in] addr remote addr,
     * @param[in] len addr len,
     * @return sock fd
     */
    virtual std::shared_ptr<flow_table::sock_key> accept(std::shared_ptr<flow_table::sock_key> key, struct sockaddr* addr, socklen_t* len) = 0;

    /**
     * @brief write buf to stack
     * @param[in] key sock fd
     * @param[in] buf buffer
     * @param[in] size buf len
     * @return write size
     */
    virtual size_t write(std::shared_ptr<flow_table::sock_key> key, char* buf, size_t size) = 0;

    /**
     * @brief read buf from stack
     * @param[in] key sock fd
     * @param[in] buf buffer
     * @param[in] size buf len
     * @return read size
     */
    virtual size_t read(std::shared_ptr<flow_table::sock_key> key, char* buf, size_t size) = 0;

    /**
     * @brief read buf from stack
     * @param[in] key sock fd
     * @param[in] buf buffer
     * @param[in] size buf len
     * @param[in] addr recv addr
     * @param[in] len addr len
     * @return read size
     */
    virtual size_t readfrom(std::shared_ptr<flow_table::sock_key> key, char* buf, size_t size, struct sockaddr* addr, socklen_t* len) = 0;

    /**
     * @brief write buf to stack
     * @param[in] key sock fd
     * @param[in] buf buffer
     * @param[in] size buf len
     * @param[in] addr recv addr
     * @param[in] len addr len
     * @return read size
     */
    virtual size_t writeto(std::shared_ptr<flow_table::sock_key> key, char* buf, size_t size, struct sockaddr* addr, socklen_t len) = 0;

    /**
    * @brief get buffer from write queue
    * @return queue from sock
    */
    virtual flow::sk_buff::ptr read_buffer_from_queue() = 0;
};

/**
 * @file interface.hpp
 * @brief stack protocol stack, include tcp/ip, arp, icmp, etc
 * @author ArisAachen
 * @copyright Copyright (c) 2024 aris All rights reserved
 */
class stack {
public:
    typedef std::shared_ptr<stack> ptr;
    typedef std::weak_ptr<stack> weak_ptr;

    /**
     * @brief init stack
     * @return init stack
     */
    virtual void init() = 0;

    /**
     * @brief register device to stack
     * @param[in] device device
     */
    virtual void register_device(interface::net_device::ptr device) = 0;

    /**
     * @brief register network handler to stack
     * @param[in] device_id device id
     * @param[in] handler network handler
     */
    virtual void register_network_handler(interface::network_handler::ptr handler) = 0;

    /**
     * @brief register transport handler to stack
     * @param[in] device_id device id
     * @param[in] handler network handler
     */
    virtual void register_transport_handler(interface::transport_handler::ptr handler) = 0;

    /**
     * @brief register sock handler to raw_stack
     * @param[in] handler sock handler
     */
    virtual void register_sock_handler(interface::sock_handler::ptr handler) = 0;

    /**
     * @brief write to device
     * @param[in] buffer write buffer
     * @param[in] device_id device id
     */
    virtual void write_to_device(flow::sk_buff::ptr buffer) = 0;

    /**
     * @brief update neighbor info
     * @param[in] hdr arp hdr
     */
    virtual void update_neighbor(const struct flow::arp_hdr* hdr, interface::net_device::ptr dev) = 0;

    /**
     * @brief write to device
     * @param[in] protocol ether protocol
     * @param[in] buffer buffer remove ether header
     * @return if need next handle
     */
    virtual bool handle_network_package(flow::sk_buff::ptr buffer) = 0;

    /**
     * @brief write to network
     * @param[in] buffer buffer remove ether header
     * @return if need next handle
     */
    virtual bool write_network_package(flow::sk_buff::ptr buffer) = 0;

    /**
     * @brief read and handle buffer
     */
    virtual void run() = 0;

    /**
     * @brief wait for end
     */    
    virtual void wait() = 0;

public:
    /**
     * @brief create sock fd
     * @param[in] domain sock domain,
     * @param[in] type sock type,
     * @param[in] protocol sock protocol,
     * @return sock fd
     */
    virtual uint32_t sock_create(int domain, int type, int protocol) = 0;

    /**
     * @brief connect sock fd
     * @param[in] fd sock fd,
     * @param[in] addr remote addr,
     * @param[in] len addr len,
     * @return sock fd
     */
    virtual bool connect(uint32_t fd, struct sockaddr* addr, socklen_t len) = 0;

    /**
     * @brief close sock fd
     * @param[in] fd sock fd,
     * @return sock fd
     */
    virtual bool close(uint32_t fd) = 0;

    /**
     * @brief bind sock fd
     * @param[in] fd sock fd,
     * @param[in] addr remote addr,
     * @param[in] len addr len,
     * @return sock fd
     */
    virtual bool bind(uint32_t fd, struct sockaddr* addr, socklen_t len) = 0;

    /**
     * @brief accept sock fd
     * @param[in] fd sock fd,
     * @param[in] addr remote addr,
     * @param[in] len addr len,
     * @return sock fd
     */
    virtual int accept(uint32_t, struct sockaddr* addr, socklen_t* len) = 0;

    /**
     * @brief listen sock fd
     * @param[in] fd sock fd,
     * @param[in] backlog max accept queue
     * @return listen result
     */ 
    virtual bool listen(uint32_t fd, int backlog) = 0;

    /**
     * @brief write buf to stack
     * @param[in] fd sock fd
     * @param[in] buf buffer
     * @param[in] size buf len
     * @return write size
     */
    virtual size_t write(uint32_t fd, char* buf, size_t size) = 0;

    /**
     * @brief read buf from stack
     * @param[in] fd sock fd
     * @param[in] buf buffer
     * @param[in] size buf len
     * @return read size
     */
    virtual size_t read(uint32_t fd, char* buf, size_t size) = 0;

    /**
     * @brief read buf from stack
     * @param[in] fd sock fd
     * @param[in] buf buffer
     * @param[in] size buf len
     * @param[in] addr recv addr
     * @param[in] len addr len
     * @return read size
     */
    virtual size_t readfrom(uint32_t fd, char* buf, size_t size, struct sockaddr* addr, socklen_t* len) = 0;

    /**
     * @brief write buf to stack
     * @param[in] fd sock fd
     * @param[in] buf buffer
     * @param[in] size buf len
     * @param[in] addr recv addr
     * @param[in] len addr len
     * @return read size
     */
    virtual size_t writeto(uint32_t fd, char* buf, size_t size, struct sockaddr* addr, socklen_t len) = 0;
};

/**
 * @file interface.hpp
 * @brief store flow table, include arp table, route table, etc
 * @author ArisAachen
 * @copyright Copyright (c) 2024 aris All rights reserved
 */
struct flow_table {
    /**
     * @brief store flow table
     * @param[in] key store key
     * @param[in] value store value
     */
    virtual void store(std::any key, std::any value) = 0;

    /**
     * @brief get flow table value
     * @param[in] key get key
     * @return return value
     */
    virtual std::optional<std::any> lookup(std::any key) = 0;
};

}

#endif // __INTERFACE_H__