#include "sock.hpp"
#include "def.hpp"
#include "flow.hpp"

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <mutex>
#include <netinet/in.h>
#include <shared_mutex>
#include <sys/socket.h>
#include <utility>

namespace flow_table {

sock::sock(sock_key::ptr key, std::weak_ptr<sock_table> table) 
    : key(key), table(table), protocol(key->protocol) {
}

// read buffer from sock
size_t sock::read(char* buf, size_t size) {
    // get front
    std::unique_lock<std::mutex> lock(read_mutex);
    // check if is non block
    if (flags == def::sock_op_flag::non_block) {
        // check if is empty, if is return eagain
        if (read_queue.empty())
            return -1;
    } else {
        read_cond.wait(lock, [&] { return !read_queue.empty(); });
    }
    // get buffer
    auto buffer = read_queue.front();
    // save key first
    buffer->key = key;
    if (buffer->get_data_len() > size) {
        // copy data to bufer
        memccpy(buf, buffer->get_data(), 0, size);
        skb_pull(buffer, size);
        return size;
    } else {
        // read all data and pop this buffer
        memccpy(buf, buffer->get_data(), 0, buffer->get_data_len());
        read_queue.pop();
        return buffer->get_data_len();
    }
    return size;
}

// read buffer from sock
size_t sock::readfrom(char* buf, size_t size, struct sockaddr* addr, socklen_t* len) {
    // get front
    std::unique_lock<std::mutex> lock(read_mutex);
    // check if is non block
    if (flags == def::sock_op_flag::non_block) {
        // check if is empty, if is return eagain
        if (read_queue.empty())
            return -1;
    } else {
        read_cond.wait(lock, [&] { return !read_queue.empty(); });
    }
    // get buffer
    auto buffer = read_queue.front();
    auto remote_addr = reinterpret_cast<struct sockaddr_in*>(addr);
    remote_addr->sin_addr.s_addr = htonl(buffer->key->remote_ip);
    remote_addr->sin_port = htons(buffer->key->remote_port);
    *len = sizeof(struct sockaddr_in);
    if (buffer->get_data_len() > size) {
        // copy data to bufer
        memccpy(buf, buffer->get_data(), 0, size);
        skb_pull(buffer, size);
        return size;
    } else {
        // read all data and pop this buffer
        memccpy(buf, buffer->get_data(), 0, buffer->get_data_len());
        read_queue.pop();
        return buffer->get_data_len();
    }
    return size;
}

// read buffer from write queue
flow::sk_buff::ptr sock::read_buffer_from_queue() {
    std::unique_lock<std::mutex> lock(write_mutex);
    write_cond.wait_for(lock, std::chrono::seconds(def::max_transport_wait_time), 
        [&] { return !write_queue.empty(); });
    if (write_queue.empty())
        return nullptr;
    auto buffer = write_queue.front();
    write_queue.pop();
    return buffer;
}

// write buffer to stack
size_t sock::writeto(char* buf, size_t size, struct sockaddr* addr, socklen_t len) {
    // get front
    std::unique_lock<std::mutex> lock(write_mutex);
    auto offset_size = 0;
    if (key->protocol == def::transport_protocol::tcp) {
        offset_size = flow::get_max_tcp_data_offset();
    } else if (key->protocol == def::transport_protocol::udp) {
        offset_size = flow::get_max_udp_data_offset();
    }
    // get remote ip and port
    auto remote_addr = reinterpret_cast<struct sockaddr_in*>(addr);
    // get send key
    auto send_key = sock_key::ptr(new sock_key(key->local_ip, key->local_port, 
        ntohl(remote_addr->sin_addr.s_addr), ntohs(remote_addr->sin_port), key->protocol));
    // alloc buffer size
    flow::sk_buff::ptr buffer = flow::sk_buff::alloc(offset_size + size);
    buffer->key = send_key;
    buffer->protocol = uint16_t(buffer->key->protocol);
    buffer->mtu = 1500;
    skb_reserve(buffer, offset_size);
    // copy to buffer
    buffer->store_data(buf, size);
    flow::skb_put(buffer, size);
    write_queue.push(buffer);
    write_cond.notify_one();
    return size;
}

// write buffer to sock
size_t sock::write(char* buf, size_t size) {
    // get front
    std::unique_lock<std::mutex> lock(write_mutex);
    auto offset_size = 0;
    if (protocol == def::transport_protocol::tcp) {
        offset_size = flow::get_max_tcp_data_offset();
    } else if (protocol == def::transport_protocol::udp) {
        offset_size = flow::get_max_udp_data_offset();
    } 
    // alloc buffer size
    flow::sk_buff::ptr buffer = flow::sk_buff::alloc(offset_size + size);
    skb_reserve(buffer, offset_size);
    // copy to buffer
    buffer->store_data(buf, size);
    flow::skb_put(buffer, size);
    read_queue.push(buffer);
    write_cond.notify_one();
    return size;
}

// write buffer to queue
void sock::write_buffer_to_queue(flow::sk_buff::ptr buffer) {
    std::unique_lock<std::mutex> lock(read_mutex);
    read_queue.push(buffer);
    read_cond.notify_one();
}


    // /// sock key store src dst info
    // sock_key::ptr key;
    // /// sock table
    // std::weak_ptr<sock_table> table;
    // /// write buffer queue
    // std::queue<flow::sk_buff::ptr> write_queue;
    // /// write buffer condition
    // std::condition_variable_any write_cond;
    // /// write share lock 
    // std::mutex write_mutex;
    // /// read buffer queue
    // std::queue<flow::sk_buff::ptr> read_queue;
    // /// read buffer condition
    // std::condition_variable_any read_cond;
    // /// read share lock
    // std::mutex read_mutex;
    // /// sock flags
    // def::sock_op_flag flags;
    // /// max buffer size
    // uint32_t max_buffer_size;
    // /// transport protocol
    // def::transport_protocol protocol;

// sock clone
sock::ptr sock_clone(sock::ptr src_sock) {
    // create dst sock
    auto dst_sock = sock::ptr(new sock(nullptr, src_sock->table));
    return dst_sock;
}

sock_table::sock_table() {

}

// create sock table 
sock_table::ptr sock_table::create() {
    return sock_table::ptr(new sock_table());
}

// create sock
sock::ptr sock_table::sock_create(sock_key::ptr key) {
    auto elem = sock::ptr(new sock(key, weak_from_this()));
    std::lock_guard<std::shared_mutex> lock(sock_mutex_);
    sock_map_.insert(std::make_pair(key, elem));
    std::cout << "create sock, local ip: " << key->local_ip << ", local port: " << key->local_port
        << ", protocol: " << uint16_t(key->protocol) << std::endl;
    return elem;
}

// store sock
sock_key::ptr sock_table::sock_store(sock::ptr sock) {
    std::lock_guard<std::shared_mutex> lock(sock_mutex_);
    sock_map_.insert(std::make_pair(sock->key, sock));
    return sock->key;
}

// get key sock ptr
sock::ptr sock_table::sock_get(sock_key::ptr key) {
    std::shared_lock<std::shared_mutex> lock(sock_mutex_);
    auto elem = sock_map_.find(key);
    if (elem == sock_map_.end())
        return nullptr;
    return elem->second;
}

// delete sock key
void sock_table::sock_delete(sock_key::ptr key) {
    std::lock_guard<std::shared_mutex> lock(sock_mutex_);
    sock_map_.erase(sock_map_.find(key));
}

flow::sk_buff::ptr sock_table::read_buffer() {
    for (auto iter : sock_map_) {
        return iter.second->read_buffer_from_queue();
    }
    return nullptr;
}

fd_table::fd_table() {

}

// create table
fd_table::ptr fd_table::create() {
    auto table = fd_table::ptr(new fd_table());
    return table;
}

// create fd 
uint32_t fd_table::fd_create(def::transport_protocol protocol) {
    std::lock_guard<std::shared_mutex> lock(fd_mutex_);
    // create sock key
    auto key = sock_key::ptr(new sock_key(0, 0, 0, 0, protocol));
    uint32_t fd = fd_num_;
    fd_num_++;
    fd_table_.insert(std::make_pair(fd, key));
    return fd;
}

// delete fd
bool fd_table::fd_delete(uint32_t fd) {
    std::lock_guard<std::shared_mutex> lock(fd_mutex_);
    auto iter = fd_table_.find(fd);
    // check if fd exist
    if (iter == fd_table_.end())
        return false;
    fd_table_.erase(iter);
    return true;
}

// get key
sock_key::ptr fd_table::sock_key_get(uint32_t fd) {
    std::shared_lock<std::shared_mutex> lock(fd_mutex_);
    auto iter = fd_table_.find(fd);
    if (iter == fd_table_.end())
        return nullptr;
    return iter->second;
}

// connect addr to fd
bool fd_table::connect(uint32_t fd, struct sockaddr_in& addr) {
    std::lock_guard<std::shared_mutex> lock(fd_mutex_);
    auto iter = fd_table_.find(fd);
    if (iter == fd_table_.end())
        return false;
    auto sock_key = iter->second;
    sock_key->remote_ip = ntohl(addr.sin_addr.s_addr);
    sock_key->remote_port = ntohs(addr.sin_port);
    sock_key->local_port = port_num_;
    port_num_++;
    return true;
}

// bind addr to fd
bool fd_table::bind(uint32_t fd, struct sockaddr_in& addr) {
    // get fd table
    std::lock_guard<std::shared_mutex> lock(fd_mutex_);
    auto iter = fd_table_.find(fd);
    if (iter == fd_table_.end())
        return false;
    auto sock_key = iter->second;
    sock_key->local_ip = ntohl(addr.sin_addr.s_addr);
    sock_key->local_port = ntohs(addr.sin_port);
    sock_key->local_port = ntohs(addr.sin_port);
    port_num_ = ntohs(addr.sin_port) + 1;
    return true;
}


}