#ifndef __IP_H__
#define __IP_H__

#include "def.hpp"
#include "flow.hpp"
#include "utils.hpp"
#include "interface.hpp"

#include <atomic>
#include <cstdint>
#include <memory>
#include <shared_mutex>
#include <unordered_map>

namespace flow_table {
    struct ip_defrag_queue;
}


namespace protocol {

/**
 * @file ip.h
 * @brief handle ip flow
 * @author ArisAachen
 * @copyright Copyright (c) 2024 aris All rights reserved
 */
class ip : public interface::network_handler {
public:
    typedef std::shared_ptr<ip> ptr;
    /**
     * @brief not allow to create ip obj
     */
    ip() = delete;

    /**
     * @brief create ip obj
     * @param[in] stack sk buffer
     * @return return if package is valid, like checksum failed
     */
    static ip::ptr create(interface::stack::weak_ptr stack);

    /**
     * @brief release ip obj
     */
    virtual ~ip() {}


    /**
     * @brief package protocol
     * @return return network layer protocol
     */    
    virtual def::network_protocol get_protocol();

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

private:
    /**
     * @brief create ip with stack
     * @param[in] stack sk buffer
     * @return return if package is valid, like checksum failed
     */
    ip(interface::stack::weak_ptr stack);

    /**
     * @brief handle ip request
     * @param[in] skb sk buffer
     * @return return if package is valid, like checksum failed
     */    
    bool ip_rcv(flow::sk_buff::ptr buffer);

    /**
     * @brief handle ip response
     * @param[in] skb sk buffer
     * @return return if package is valid, like checksum failed
     */
    bool ip_send(flow::sk_buff::ptr buffer);

    /**
     * @brief make package flow
     * @param[in] skb sk buffer
     * @return return if package is valid, like checksum failed
     */
    bool ip_make_flow(flow::sk_buff::ptr buffer, size_t offset, bool more_flag);

    /**
     * @brief fragment ip
     * @param[in] skb sk buffer
     * @return return if package is valid, like checksum failed
     */
    bool ip_fragment(flow::sk_buff::ptr buffer);

    /**
     * @brief defragment ip
     * @param[in] skb sk buffer
     * @return return if package is valid, like checksum failed
     */
    bool ip_defragment(flow::sk_buff::ptr buffer);

private:
    /// stack
    interface::stack::weak_ptr stack_;
    /// identification
    std::atomic<uint16_t> identification_;
    /// defrag queue
    std::shared_ptr<flow_table::ip_defrag_queue> defrag_queue_;
};

}


namespace flow_table {

struct ip_defrag_key {
typedef std::shared_ptr<ip_defrag_key> ptr;
    ip_defrag_key(uint32_t src_ip, uint32_t dst_ip, uint16_t id, uint8_t protocol) :
    src_ip(src_ip), dst_ip(dst_ip), identification(id), protocol(protocol) {}
    /// source ip
    uint32_t src_ip;
    /// dst ip
    uint32_t dst_ip;
    /// identification 
    uint16_t identification;
    /// protocol
    uint8_t protocol;
};

struct ip_defrag_key_equal {
    // check if is the same
    bool operator() (const ip_defrag_key::ptr first, const ip_defrag_key::ptr second) const {
        if (first->src_ip == second->src_ip && first->dst_ip == second->dst_ip
            && first->identification == second->identification && first->protocol == second->protocol)
            return true;
        return false;
    }
};

struct ip_defrag_hash_key {
    /**
    * @brief get jenkins hash key
    * @param[in] key id key
    * @return hash result
    */
    size_t operator() (const ip_defrag_key::ptr key) const {
        // get port
        auto hash_code = utils::generic::jhash_3words(key->src_ip, key->dst_ip, key->identification);
        return hash_code;
    }
};

struct ip_defrag_queue {
public:
    typedef std::shared_ptr<ip_defrag_queue> ptr;
    typedef std::shared_ptr<std::unordered_map<uint16_t, flow::sk_buff::ptr>> defrag_offset_map_ptr; 
    typedef std::unordered_map<ip_defrag_key::ptr, defrag_offset_map_ptr, ip_defrag_hash_key, ip_defrag_key_equal> ip_defrag_map;

    /**
     * @brief create ip defrag
     * @return return if package is valid, like checksum failed
     */
    static ip_defrag_queue::ptr create();

    /**
     * @brief push defrag
     * @param[in] skb sk buffer
     * @return check if buffer get full package
     */
    flow::sk_buff::ptr defrag_push(flow::sk_buff::ptr buffer);

    /**
     * @brief remove defrag
     * @param[in] key defrag key
     * @return check if buffer get full package
     */
    bool defrag_remove(ip_defrag_key::ptr key);

    /**
     * @brief get defrag list
     * @param[in] key defrag key
     * @return check if defrag list exist
     */
    defrag_offset_map_ptr defrag_find(ip_defrag_key::ptr key);

    /**
     * @brief create defrag list
     * @param[in] key defrag key
     * @return check if defrag list exist
     */
    defrag_offset_map_ptr defrag_list_create(ip_defrag_key::ptr key);

    /**
     * @brief try to reassemble ip frag
     * @param[in] key defrag key
     * @return check if defrag list exist
     */
    static flow::sk_buff::ptr ip_defrag_reassemble(defrag_offset_map_ptr offset_map);

private:
    /**
     * @brief create ip defrag
     * @return return if package is valid, like checksum failed
     */
    ip_defrag_queue();

private:
    /// buffer lock
    std::shared_mutex mutex;
    /// buffer list
    ip_defrag_map defrag_map;
};

}


#endif // __IP_H__