#ifndef __SOCK_H__
#define __SOCK_H__

#include "def.hpp"
#include "utils.hpp"
#include "flow.hpp"

#include <atomic>
#include <condition_variable>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <queue>
#include <unordered_map>
#include <shared_mutex>

#include <netinet/in.h>
#include <sys/socket.h>


namespace flow_table {

/**
 * @file sock.hpp
 * @brief sock key
 * @author ArisAachen
 * @copyright Copyright (c) 2024 aris All rights reserved
 * @link https://datatracker.ietf.org/doc/html/rfc792
 */
struct sock_key : std::enable_shared_from_this<sock_key> {
    typedef std::shared_ptr<sock_key> ptr;
    /**
    * @brief create sock key 
    * @return hash result
    */
    sock_key(uint32_t src_ip, uint16_t src_port, uint32_t dst_ip, uint32_t dst_port, def::transport_protocol protocol) :
    local_ip(src_ip), local_port(src_port), remote_ip(dst_ip), remote_port(dst_port), protocol(protocol) {}

    /**
    * @brief not allow to create empty key
    */
    sock_key() = delete;

    /// source ip
    uint32_t local_ip;
    /// source port
    uint16_t local_port;
    /// dst ip
    uint32_t remote_ip;
    /// dst port
    uint16_t remote_port;
    /// protocol
    def::transport_protocol protocol;
};

/**
 * @file sock.hpp
 * @brief calculate sock key 
 * @author ArisAachen
 * @copyright Copyright (c) 2024 aris All rights reserved
 * @link https://datatracker.ietf.org/doc/html/rfc792
 */
struct hash_sock_get_key {
    /**
    * @brief get jenkins hash key
    * @param[in] sock socket
    * @return hash result
    */
    size_t operator() (const sock_key::ptr sock) const {
        // get port
        uint32_t port = (sock->local_port << 16) + sock->remote_port;
        auto key = utils::generic::jhash_3words(sock->local_ip, sock->remote_ip, port);
        return key;
    }
};

/**
 * @file sock.hpp
 * @brief compare if key is the same
 * @author ArisAachen
 * @copyright Copyright (c) 2024 aris All rights reserved
 * @link https://datatracker.ietf.org/doc/html/rfc792
 */
struct hash_sock_equal_key {
    /**
    * @brief check if hash key is the same
    * @param[in] first socket
    * @param[in] second socket
    * @return hash result
    */
    bool operator() (const sock_key::ptr first, const sock_key::ptr second) const {
        // check protocol
        if (first->protocol != second->protocol)
            return false;
        if (first->local_ip != 0 && second->local_ip != 0 && first->local_ip != second->local_ip)
            return false;
        if (first->remote_ip != 0 && second->remote_ip != 0 && second->remote_ip != second->remote_ip)
            return false;
        if (first->remote_port != 0 && second->remote_port != 0 && first->remote_port == second->remote_port)
            return false;
        if (first->local_port != second->local_port)
            return false;
        return true;
    }
};

class sock_table;

/**
 * @file sock.hpp
 * @brief create sock 
 * @author ArisAachen
 * @copyright Copyright (c) 2024 aris All rights reserved
 * @link https://datatracker.ietf.org/doc/html/rfc792
 */
struct sock : std::enable_shared_from_this<sock> {
    friend class sock_table;
    typedef std::shared_ptr<sock> ptr;
public:
    /**
    * @brief not allow to create default sock
    */
    sock() = delete;

    /**
    * @brief write data to sock
    * @param[in] buf write buf
    * @param[in] size   write size
    * @return write size
    */
    size_t write(char* buf, size_t size);

    /**
    * @brief write data to sock
    * @param[in] buf write buf
    * @param[in] size write size
    * @param[in] addr write addr
    * @param[in] len addr len
    * @return write size
    */
    size_t writeto(char* buf, size_t size, struct sockaddr* addr, socklen_t len);

    /**
    * @brief get buffer from write queue
    * @return queue from sock
    */
    flow::sk_buff::ptr read_buffer_from_queue();

    /**
    * @brief check if hash key is the same
    * @param[in] buf write buf
    * @param[in] size   write size
    * @return read data from sock
    */
    size_t read(char* buf, size_t size);

    /**
     * @brief read buf from stack
     * @param[in] buf buffer
     * @param[in] size buf len
     * @param[in] addr recv addr
     * @param[in] len addr len
     * @return read size
     */
    size_t readfrom(char* buf, size_t size, struct sockaddr* addr, socklen_t* len);

    /**
    * @brief write data to sock
    * @param[in] buffer write buf
    */ 
    void write_buffer_to_queue(flow::sk_buff::ptr buffer);

    /**
    * @brief release sock
    */ 
    virtual ~sock() {}

public:
    /// sock key store src dst info
    sock_key::ptr key;
    /// sock table
    std::weak_ptr<sock_table> table;
    /// write buffer queue
    std::queue<flow::sk_buff::ptr> write_queue;
    /// write buffer condition
    std::condition_variable_any write_cond;
    /// write share lock 
    std::mutex write_mutex;
    /// read buffer queue
    std::queue<flow::sk_buff::ptr> read_queue;
    /// read buffer condition
    std::condition_variable_any read_cond;
    /// read share lock
    std::mutex read_mutex;
    /// sock flags
    def::sock_op_flag flags;
    /// max buffer size
    uint32_t max_buffer_size;
    /// transport protocol
    def::transport_protocol protocol;

public:
    /**
    * @brief create sock
    * @param[in] src_ip source ip
    * @param[in] src_port source port
    * @param[in] dst_ip dst ip
    * @param[in] dst_port dst port
    */
    sock(sock_key::ptr key, std::weak_ptr<sock_table> table);
};

/**
* @brief sock clone
* @param[in] sock source sock
* @return sock 
*/
static sock::ptr sock_clone(sock::ptr sock);

/**
 * @file flow.hpp
 * @brief compare if key is the same
 * @author ArisAachen
 * @copyright Copyright (c) 2024 aris All rights reserved
 * @link https://datatracker.ietf.org/doc/html/rfc792
 */
class sock_table : std::enable_shared_from_this<sock_table> {
public:
    typedef std::shared_ptr<sock_table> ptr;
    typedef std::weak_ptr<sock_table> weak_ptr;

    /**
    * @brief create sock
    * @return sock_table
    */
    static sock_table::ptr create();

    /**
    * @brief create sock
    * @param[in] src_ip source ip
    * @param[in] src_port source port
    * @param[in] dst_ip dst ip
    * @param[in] dst_port dst port
    * @return sock 
    */
    sock::ptr sock_create(sock_key::ptr key);

    /**
    * @brief create sock
    * @param[in] sock store sock
    * @return sock 
    */
    sock_key::ptr sock_store(sock::ptr sock);

    /**
    * @brief get sock
    * @param[in] key sock key
    * @return sock 
    */
    sock::ptr sock_get(sock_key::ptr key);

    /**
    * @brief delete sock
    * @param[in] key sock key
    * @return sock 
    */
    void sock_delete(sock_key::ptr key);

    /**
    * @brief read buffer from frame
    * @return buffer 
    */
    flow::sk_buff::ptr read_buffer();

private:
    /**
    * @brief create sock table
    */
    sock_table();

private:
    /// sock mutex
    std::shared_mutex sock_mutex_;
    /// socket map to get socket
    std::unordered_map<sock_key::ptr, sock::ptr, hash_sock_get_key, hash_sock_equal_key> sock_map_;
};

/**
 * @file sock.hpp
 * @brief create fd table
 * @author ArisAachen
 * @copyright Copyright (c) 2024 aris All rights reserved
 * @link https://datatracker.ietf.org/doc/html/rfc792
 */
class fd_table {
public:
    typedef std::shared_ptr<fd_table> ptr;

    /**
    * @brief create fd table
    * @return fd table
    */
    static fd_table::ptr create();

    /**
    * @brief create fd 
    * @param[in] protocol transport protocol
    * @return sock 
    */
    uint32_t fd_create(def::transport_protocol protocol);

    /**
    * @brief delete fd 
    * @param[in] fd fd key
    * @return delete result
    */
    bool fd_delete(uint32_t fd);

    /**
    * @brief get sock key from fd
    * @param[in] fd fd 
    * @return get result
    */
    sock_key::ptr sock_key_get(uint32_t fd);

    /**
    * @brief connect
    * @param[in] fd fd 
    * @return true if connect success
    */
    bool connect(uint32_t fd, struct sockaddr_in& addr);

    /**
    * @brief bind
    * @param[in] fd fd 
    * @return true if bind success
    */
    bool bind(uint32_t fd, struct sockaddr_in& addr);

private:
    /**
    * @brief create fd table
    */
    fd_table();

private:
    /// current fd number 
    /// TODO: should use bit map here
    std::atomic<uint32_t> fd_num_ {3};
    /// port number
    std::atomic<uint16_t> port_num_ { 2000 };
    /// share mutex
    std::shared_mutex fd_mutex_;
    /// fd table
    std::unordered_map<uint32_t, sock_key::ptr> fd_table_;
};

}


#endif // __SOCK_H__