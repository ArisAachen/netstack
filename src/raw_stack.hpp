#ifndef __STACK_H__
#define __STACK_H__


#include "arp.hpp"
#include "def.hpp"
#include "flow.hpp"
#include "interface.hpp"
#include "sock.hpp"

#include <cstddef>
#include <cstdint>
#include <memory>
#include <netinet/in.h>
#include <sys/socket.h>
#include <thread>
#include <unordered_map>
#include <utility>
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
    /// raw_stack ptr
    typedef std::shared_ptr<raw_stack> ptr;
    typedef std::weak_ptr<raw_stack> weak_ptr;

    /**
     * @brief get raw_stack instance
     * @return raw_stack instance
     */
    static raw_stack::ptr get_instance();

    /**
     * @brief init raw_stack
     */
    virtual void init();

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
     * @brief register transport handler to raw_stack
     * @param[in] handler transport handler
     */
    virtual void register_transport_handler(interface::transport_handler::ptr handler) {
        transport_handler_map_.insert(std::make_pair(handler->get_protocol(), handler));
    }

    /**
     * @brief register sock handler to raw_stack
     * @param[in] handler sock handler
     */
    virtual void register_sock_handler(interface::sock_handler::ptr handler) {
        sock_handler_map_.insert(std::make_pair(def::transport_protocol::tcp, handler));
    }

    /**
     * @brief write to device
     * @param[in] buffer write buffer
     */
    virtual void write_to_device(flow::sk_buff::ptr buffer);

    /**
     * @brief update neighbor info
     * @param[in] hdr arp hdr
     */
    virtual void update_neighbor(const struct flow::arp_hdr* hdr, interface::net_device::ptr dev);

    /**
     * @brief handle network layer buffer
     * @param[in] buffer buffer remove ether header
     * @return if need next handle
     */
    virtual bool handle_network_package(flow::sk_buff::ptr buffer);

    /**
     * @brief handle transport layer buffer
     * @param[in] buffer buffer remove network header
     * @return if need next handle
     */    
    virtual bool handle_transport_package(flow::sk_buff::ptr buffer);

    /**
     * @brief write to network
     * @param[in] buffer buffer remove ether header
     * @return if need next handle
     */
    virtual bool write_network_package(flow::sk_buff::ptr buffer);

    /**
     * @brief write to transport
     * @param[in] buffer write transport
     * @return if need next handle
     */
    virtual bool write_transport_package(flow::sk_buff::ptr buffer);

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

public:
    /**
     * @brief create sock fd
     * @param[in] domain sock domain,
     * @param[in] type sock type,
     * @param[in] protocol sock protocol,
     * @return sock fd
     */
    virtual uint32_t sock_create(int domain, int type, int protocol);

    /**
     * @brief delete sock fd
     * @param[in] fd sock fd
     * @return delete fd success
     */
    virtual bool sock_delete(uint32_t fd);

    /**
     * @brief connect sock fd
     * @param[in] fd sock fd,
     * @param[in] addr remote addr,
     * @param[in] len addr len,
     * @return sock fd
     */
    virtual bool connect(uint32_t fd, struct sockaddr* addr, socklen_t len);

    /**
     * @brief close sock fd
     * @param[in] fd sock fd,
     * @return sock fd
     */
    virtual bool close(uint32_t fd);

    /**
     * @brief bind sock fd
     * @param[in] fd sock fd,
     * @param[in] addr remote addr,
     * @param[in] len addr len,
     * @return sock fd
     */
    virtual bool bind(uint32_t fd, struct sockaddr* addr, socklen_t len);

    /**
     * @brief listen sock fd
     * @param[in] fd sock fd,
     * @param[in] backlog max accept queue
     * @return listen result
     */ 
    virtual bool listen(uint32_t fd, int backlog);

    /**
     * @brief write buf to stack
     * @param[in] fd sock fd
     * @param[in] buf buffer
     * @param[in] size buf len
     * @return write size
     */
    virtual size_t write(uint32_t fd, char* buf, size_t size);

    /**
     * @brief read buf from stack
     * @param[in] fd sock fd
     * @param[in] buf buffer
     * @param[in] size buf len
     * @return read size
     */
    virtual size_t read(uint32_t fd, char* buf, size_t size);

    /**
     * @brief read buf from stack
     * @param[in] fd sock fd
     * @param[in] buf buffer
     * @param[in] size buf len
     * @param[in] addr recv addr
     * @param[in] len addr len
     * @return read size
     */
    virtual size_t readfrom(uint32_t fd, char* buf, size_t size, struct sockaddr* addr, socklen_t* len);

    /**
     * @brief write buf to stack
     * @param[in] fd sock fd
     * @param[in] buf buffer
     * @param[in] size buf len
     * @param[in] addr recv addr
     * @param[in] len addr len
     * @return read size
     */
    virtual size_t writeto(uint32_t fd, char* buf, size_t size, struct sockaddr* addr, socklen_t len);

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
     * @brief parse packet 
     */
    void handle_sock_buffer_package();

private:
    /// network handler map
    std::unordered_map<def::network_protocol, interface::network_handler::ptr> network_handler_map_;
    /// transport handler map
    std::unordered_map<def::transport_protocol, interface::transport_handler::ptr> transport_handler_map_;
    /// sock handler map 
    std::unordered_map<def::transport_protocol, interface::sock_handler::ptr> sock_handler_map_;
    /// device map
    std::unordered_map<uint8_t, interface::net_device::ptr> device_map_;
    /// thread vector
    std::vector<std::thread> thread_vec_;
    /// neighbor flow 
    flow_table::neighbor_table::ptr neighbor_table_;
    /// fd table
    flow_table::fd_table::ptr fd_table_;
    /// udp sock flow table
    flow_table::sock_table::ptr udp_sock_table_;
};

}

#endif // __STACK_H__