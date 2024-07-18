#include "sock.hpp"
#include "def.hpp"
#include "flow.hpp"

#include <cstddef>
#include <cstring>
#include <mutex>

namespace flow {

sock::sock(sock_key::ptr key, std::weak_ptr<sock_table> table) 
    : key(key), table(table) {
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
sk_buff::ptr sock::read_buffer_from_queue() {
    std::unique_lock<std::mutex> lock(write_mutex);
    write_cond.wait(lock, [&] { return !write_queue.empty(); });
    auto buffer = write_queue.front();
    write_queue.pop();
    return buffer;
}

// write buffer to sock
size_t sock::write(char* buf, size_t size) {
    // get front
    std::unique_lock<std::mutex> lock(write_mutex);
    auto offset_size = 0;
    if (protocol == def::transport_protocol::tcp) {
        offset_size = get_max_tcp_data_offset();
    } else if (protocol == def::transport_protocol::udp) {
        offset_size = get_max_udp_data_offset();
    } 
    // alloc buffer size
    sk_buff::ptr buffer = sk_buff::alloc(offset_size + size);
    skb_reserve(buffer, offset_size);
    // copy to buffer
    buffer->store_data(buf, size);
    read_queue.push(buffer);
    write_cond.notify_one();
    return size;
}

// write buffer to queue
void sock::write_buffer_to_queue(sk_buff::ptr buffer) {
    std::unique_lock<std::mutex> lock(read_mutex);
    read_queue.push(buffer);
    read_cond.notify_one();
}


}