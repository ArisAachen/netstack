#ifndef __SOCK_H__
#define __SOCK_H__

#include "def.hpp"
#include "flow.hpp"

#include <condition_variable>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <iostream>
#include <queue>
#include <unordered_map>

namespace flow {

/**
 * @brief get rol 
 * @param[in] word num
 * @param[in] shift shift bit
 * @return rol result
 */
static uint32_t rol32(uint32_t word, unsigned int shift)
{
	return (word << (shift & 31)) | (word >> ((-shift) & 31));
}

/**
 * @brief get jenkins hash
 * @param[in] first first num
 * @param[in] second second num
 * @param[in] third third num
 * @return hash result
 */
#define jhash_final(first, second, third)                             \
{                                                                     \
    third ^= second; third -= rol32(second, 14);                      \
    first ^= third; first -= rol32(third, 11);                        \
    second ^= first; second -= rol32(first, 15);                      \
    third ^= second; third -= rol32(second, 16);                      \
    first ^= third; first -= rol32(third, 4);                         \
    second ^= first; second -= rol32(first, 14);                      \
    third ^= second; third -= rol32(second, 24);                      \
    return 0;                                                         \
}

/**
 * @brief get jenkins hash
 * @param[in] first first num
 * @param[in] second second num
 * @param[in] third third num
 * @return hash result
 */
static inline uint32_t jhash_3words(uint32_t first, uint32_t second, uint32_t third) {
    jhash_final(first, second, third);
    return third;
}

/**
 * @file sock.hpp
 * @brief sock key
 * @author ArisAachen
 * @copyright Copyright (c) 2024 aris All rights reserved
 * @link https://datatracker.ietf.org/doc/html/rfc792
 */
struct sock_key {
    typedef std::shared_ptr<sock_key> ptr;
    /// source ip
    uint32_t src_ip;
    /// source port
    uint16_t src_port;
    /// dst ip
    uint32_t dst_ip;
    /// dst port
    uint16_t dst_port;
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
    size_t operator() (const sock_key& sock) const {
        // get port
        uint32_t port = (sock.src_port << 16) + sock.dst_port;
        auto key = jhash_3words(sock.src_ip, sock.dst_ip, port);
        std::cout << "hask key, " << sock.src_ip << ":" << sock.src_port
            << " -> " << sock.dst_ip << ":" << sock.dst_port
            << ", port: " << port << ", result: " << key << std::endl;
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
    bool operator() (const sock_key& first, const sock_key& second) const {
        // all connection info should equal
        if (first.src_ip == second.src_ip && first.src_port == second.src_port
            && first.dst_ip == second.dst_ip && first.dst_port == second.dst_port)
            return true;
        return false;
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
struct sock {
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
    * @brief get buffer from write queue
    * @return queue from sock
    */
    sk_buff::ptr read_buffer_from_queue();

    /**
    * @brief check if hash key is the same
    * @param[in] buf write buf
    * @param[in] size   write size
    * @return read data from sock
    */
    size_t read(char* buf, size_t size);

    /**
    * @brief write data to sock
    * @param[in] buffer write buf
    */ 
    void write_buffer_to_queue(sk_buff::ptr buffer);

public:
    /// sock key store src dst info
    sock_key::ptr key;
    /// sock table
    std::weak_ptr<sock_table> table;
    /// write buffer queue
    std::queue<sk_buff::ptr> write_queue;
    /// write buffer condition
    std::condition_variable_any write_cond;
    /// write share lock 
    std::mutex write_mutex;
    /// read buffer queue
    std::queue<sk_buff::ptr> read_queue;
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

private:
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
 * @file flow.hpp
 * @brief compare if key is the same
 * @author ArisAachen
 * @copyright Copyright (c) 2024 aris All rights reserved
 * @link https://datatracker.ietf.org/doc/html/rfc792
 */
class sock_table {
public:
    typedef std::shared_ptr<sock_table> ptr;

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
    sock::ptr create_sock(uint32_t src_ip, uint16_t src_port, uint32_t dst_ip, uint16_t dst_port);

private:
    /**
    * @brief create sock table
    */
    sock_table();

private:
    /// socket map to get socket
    std::unordered_map<sock_key::ptr, sock::ptr, hash_sock_get_key, hash_sock_equal_key> sock_map_;
};

}

#endif // __SOCK_H__