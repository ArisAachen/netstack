#ifndef __TCP_H__
#define __TCP_H__

#include "def.hpp"
#include "flow.hpp"
#include "sock.hpp"
#include "interface.hpp"

#include <atomic>
#include <cstdint>
#include <list>
#include <memory>
#include <queue>

namespace flow_table {
    struct tcp_connection_queue;
}

namespace protocol {

/**
 * @file tcp.h
 * @brief handle tcp flow
 * @author ArisAachen
 * @copyright Copyright (c) 2024 aris All rights reserved
 */
class tcp : public interface::transport_handler, public interface::sock_handler {
public:
    typedef std::shared_ptr<tcp> ptr;
    /**
     * @brief not allow to create tcp obj
     */
    tcp() = delete;

    /**
     * @brief create tcp obj
     * @param[in] stack sk buffer
     * @return return if package is valid, like checksum failed
     */
    static tcp::ptr create(interface::stack::weak_ptr stack);

    /**
     * @brief release tcp obj
     */
    virtual ~tcp() {}

    /**
     * @brief package protocol
     * @return return transport layer protocol
     */    
    virtual def::transport_protocol get_protocol();

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

    /**
     * @brief write buffer to sock
     * @param[in] skb sk buffer
     * @return return if package is valid, like checksum failed
     */ 
    virtual bool write_buffer_to_sock(flow::sk_buff::ptr buffer);

public:
    /**
     * @brief create sock fd
     * @param[in] domain sock domain,
     * @param[in] type sock type,
     * @param[in] protocol sock protocol,
     * @return sock fd
     */
    virtual bool sock_create(flow_table::sock_key::ptr key);

    /**
     * @brief connect sock fd
     * @param[in] key sock fd,
     * @param[in] addr remote addr,
     * @param[in] len addr len,
     * @return sock fd
     */
    virtual bool connect(flow_table::sock_key::ptr key, struct sockaddr* addr, socklen_t len);

    /**
     * @brief close sock fd
     * @param[in] key sock fd,
     * @return sock fd
     */
    virtual bool close(flow_table::sock_key::ptr key);

    /**
     * @brief bind sock fd
     * @param[in] key sock fd,
     * @param[in] addr remote addr,
     * @param[in] len addr len,
     * @return sock fd
     */
    virtual bool bind(flow_table::sock_key::ptr key, struct sockaddr* addr, socklen_t len);

    /**
     * @brief listen sock fd
     * @param[in] key sock fd,
     * @param[in] backlog max backlog
     * @return sock fd
     */
    virtual bool listen(flow_table::sock_key::ptr key, int backlog);

    /**
     * @brief accept sock fd
     * @param[in] key sock fd,
     * @param[in] addr remote addr,
     * @param[in] len addr len,
     * @return sock fd
     */
    virtual std::shared_ptr<flow_table::sock_key> accept(flow_table::sock_key::ptr key, struct sockaddr* addr, socklen_t len);

    /**
     * @brief write buf to stack
     * @param[in] key sock fd
     * @param[in] buf buffer
     * @param[in] size buf len
     * @return write size
     */
    virtual size_t write(flow_table::sock_key::ptr key, char* buf, size_t size);

    /**
     * @brief read buf from stack
     * @param[in] key sock fd
     * @param[in] buf buffer
     * @param[in] size buf len
     * @return read size
     */
    virtual size_t read(flow_table::sock_key::ptr key, char* buf, size_t size);

    /**
     * @brief read buf from stack
     * @param[in] key sock fd
     * @param[in] buf buffer
     * @param[in] size buf len
     * @param[in] addr recv addr
     * @param[in] len addr len
     * @return read size
     */
    virtual size_t readfrom(flow_table::sock_key::ptr key, char* buf, size_t size, struct sockaddr* addr, socklen_t* len);

    /**
     * @brief write buf to stack
     * @param[in] key sock fd
     * @param[in] buf buffer
     * @param[in] size buf len
     * @param[in] addr recv addr
     * @param[in] len addr len
     * @return read size
     */
    virtual size_t writeto(flow_table::sock_key::ptr key, char* buf, size_t size, struct sockaddr* addr, socklen_t len);

private:
    /**
     * @brief create tcp with stack
     * @param[in] stack sk buffer
     * @return return if package is valid, like checksum failed
     */
    tcp(interface::stack::weak_ptr stack);

    /**
     * @brief handle tcp request
     * @param[in] skb sk buffer
     * @return return if package is valid, like checksum failed
     */    
    bool tcp_rcv(flow::sk_buff::ptr buffer);

    /**
     * @brief handle tcp response
     * @param[in] skb sk buffer
     * @return return if package is valid, like checksum failed
     */
    bool tcp_send(flow::sk_buff::ptr buffer);

private:
    /// stack
    interface::stack::weak_ptr stack_;
    /// identification
    std::atomic<uint16_t> identification_;
    /// listen sock table
    flow_table::sock_table::ptr listen_sock_table_;
    /// tcp sock table
    flow_table::sock_table::ptr tcp_sock_table_;
};

}

namespace flow_table {
enum class tcp_sock_type : uint8_t {
    none,
    listen,
    established
};

/**
 * @file tcp.h
 * @brief handle tcp sock flow
 * @author ArisAachen
 * @copyright Copyright (c) 2024 aris All rights reserved
 */
class tcp_sock : public sock {
public:
typedef std::shared_ptr<tcp_sock> ptr;
    /**
     * @brief create tcp sock
     * @param[in] key tcp key
     * @param[in] type tcp type
     * @return return if package is valid, like checksum failed
     */
    static tcp_sock::ptr create(sock_key::ptr key, sock_table::weak_ptr table, interface::stack::weak_ptr stack, tcp_sock_type type);

    /**
     * @brief not allow to create direct
     */
    tcp_sock() = delete;

    /**
     * @brief handle tcp connection
     * @param[in] skb sk buffer
     * @return return if package is valid, like checksum failed
     */
    virtual void handle_connection(flow::sk_buff::ptr buffer);

    /**
     * @brief accept sock
     * @return return if package is valid, like checksum failed
     */
    tcp_sock::ptr accept();

private:
    /**
     * @brief create tcp sock
     * @param[in] key tcp key
     * @param[in] type tcp type
     * @return return if package is valid, like checksum failed
     */
    tcp_sock(sock_key::ptr key, sock_table::weak_ptr table, interface::stack::weak_ptr stack, tcp_sock_type type);

private:
    /// sock type 
    tcp_sock_type type_;
    /// tcp connection state
    def::tcp_connection_state state_;
    /// accept queue
    std::queue<tcp_sock::ptr> accept_queue_;
    /// syn queue
    std::list<tcp_sock::ptr> syn_list_;
    /// stack
    interface::stack::weak_ptr stack_;
};

}


#endif // __TCP_H__