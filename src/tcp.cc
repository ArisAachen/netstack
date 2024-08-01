#include "tcp.hpp"
#include "def.hpp"
#include "flow.hpp"
#include "interface.hpp"
#include "sock.hpp"

#include <cstdint>

#include <memory>
#include <netinet/in.h>

namespace protocol {


tcp::tcp(interface::stack::weak_ptr stack) {
    stack_ = stack;
    listen_sock_table_ = flow_table::sock_table::create();
}

tcp::ptr tcp::create(interface::stack::weak_ptr stack) {
    return tcp::ptr(new tcp(stack));
}

def::transport_protocol tcp::get_protocol() {
    return def::transport_protocol::tcp;
}

bool tcp::pack_flow(flow::sk_buff::ptr buffer) {
    return false;
}

bool tcp::unpack_flow(flow::sk_buff::ptr buffer) {
    auto hdr = reinterpret_cast<flow::tcp_hdr*>(buffer->get_data());
    uint16_t local_port = ntohs(hdr->dst_port);
    uint16_t remote_port = ntohs(hdr->src_port);
    std::cout << "rcv tcp message, " << std::dec << remote_port 
        << " -> " << local_port << std::endl;
    // get tcp sock
    if (hdr->syn) {
        auto local_ip = std::get<uint32_t>(buffer->dst);
        auto key = flow_table::sock_key::ptr(new flow_table::sock_key(local_ip, local_port, 0, 0, def::transport_protocol::tcp));
        auto sock = listen_sock_table_->sock_get(key);
        if (sock == nullptr) {
            key = flow_table::sock_key::ptr(new flow_table::sock_key(0, local_port, 0, 0, def::transport_protocol::tcp));
            sock = listen_sock_table_->sock_get(key);
        }
        auto tcp_sock = std::dynamic_pointer_cast<flow_table::tcp_sock>(sock);
        if (tcp_sock == nullptr)
            return false;
        tcp_sock->handle_connection(buffer);
    }

    return false;
}

bool tcp::write_buffer_to_sock(flow::sk_buff::ptr buffer) {
    return false;
}

bool tcp::tcp_rcv(flow::sk_buff::ptr buffer) {
    return false;
}

bool tcp::tcp_send(flow::sk_buff::ptr buffer) {
    return false;
}

bool tcp::sock_create(flow_table::sock_key::ptr key) {
    // create listen key
    auto tcp_sock = flow_table::tcp_sock::create(key, tcp_sock_table_, stack_, flow_table::tcp_sock_type::listen);
    listen_sock_table_->sock_store(tcp_sock);
    return false;
}

bool tcp::connect(flow_table::sock_key::ptr key, struct sockaddr* addr, socklen_t len) {
    return false;
}

bool tcp::close(flow_table::sock_key::ptr key) {
    return false;
}

bool tcp::listen(flow_table::sock_key::ptr key, int backlog) {
    return false;
}

bool tcp::bind(flow_table::sock_key::ptr key, struct sockaddr* addr, socklen_t len) {
    return false;
}

std::shared_ptr<flow_table::sock_key> tcp::accept(flow_table::sock_key::ptr key, struct sockaddr* addr, socklen_t len) {
    auto sock = listen_sock_table_->sock_get(key);
    auto tcp_sock = std::dynamic_pointer_cast<flow_table::tcp_sock>(sock);
    auto accept_sock = tcp_sock->accept();
    tcp_sock_table_->sock_store(accept_sock);
    return accept_sock->key;
}

size_t tcp::write(flow_table::sock_key::ptr key, char* buf, size_t size) {
    return false;
}

size_t tcp::read(flow_table::sock_key::ptr key, char* buf, size_t size) {
    return false;
}

size_t tcp::readfrom(flow_table::sock_key::ptr key, char* buf, size_t size, struct sockaddr* addr, socklen_t* len) {
    return false;
}

size_t tcp::writeto(flow_table::sock_key::ptr key, char* buf, size_t size, struct sockaddr* addr, socklen_t len) {
    return false;
}

}


namespace flow_table {

// ceate sock table
tcp_sock::ptr tcp_sock::create(sock_key::ptr key, sock_table::weak_ptr table, interface::stack::weak_ptr stack, tcp_sock_type type) {
    return tcp_sock::ptr(new tcp_sock(key, table, stack, type));
}

tcp_sock::tcp_sock(sock_key::ptr key, sock_table::weak_ptr table, interface::stack::weak_ptr stack, tcp_sock_type type) : sock(key, table) {
    type_ = type;
    stack_ = stack;
}

void tcp_sock::handle_connection(flow::sk_buff::ptr buffer) {
    // get tcp header
    auto req_hdr = reinterpret_cast<struct flow::tcp_hdr*>(buffer->get_data());
    // create info 
    auto local_ip = std::get<uint32_t>(buffer->dst);
    auto remote_ip = std::get<uint32_t>(buffer->src);
    auto local_port = ntohs(req_hdr->dst_port);
    auto remote_port = ntohs(req_hdr->src_port);
    // check syn 
    if (req_hdr->syn) {
        // create dst sock
        auto dst_key = sock_key::ptr(new sock_key(local_ip, local_port, remote_ip, remote_port, def::transport_protocol::tcp));
        auto dst_sock = tcp_sock::create(dst_key, this->table, stack_, tcp_sock_type::established);
        // create response header
        auto alloc_size = sizeof(struct flow::tcp_hdr) + sizeof(struct flow::ip_hdr) + def::max_ether_header;
        flow::sk_buff::ptr resp_buffer = flow::sk_buff::alloc(alloc_size);
        resp_buffer->protocol = uint16_t(def::transport_protocol::tcp);
        resp_buffer->data_len = alloc_size;
        resp_buffer->src = buffer->dst;
        resp_buffer->dst = buffer->src;
        // append to tcp header
        flow::skb_reserve(resp_buffer, alloc_size);
        flow::skb_push(resp_buffer, sizeof(struct flow::tcp_hdr));
        // get response tcp header
        auto resp_hdr = reinterpret_cast<flow::tcp_hdr*>(resp_buffer->get_data());
        resp_hdr->ack_number = htonl(ntohl(req_hdr->sequence_number) + 1);
        resp_hdr->src_port = req_hdr->dst_port;
        resp_hdr->dst_port = req_hdr->src_port;
        resp_hdr->sequence_number = htonl(1);
        resp_hdr->syn = 0b1;
        resp_hdr->ack = 0b1;
        resp_hdr->header_len = 0x5;
        resp_hdr->window_size = def::checksum_max_num;
        resp_hdr->tcp_checksum = 0;
        // add fake udp header
        flow::skb_push(resp_buffer, sizeof(struct flow::transport_fake_hdr));
        auto fake_hdr = reinterpret_cast<flow::transport_fake_hdr*>(resp_buffer->get_data());
        fake_hdr->src_ip = htonl(local_ip);
        fake_hdr->dst_ip = htonl(remote_ip);
        fake_hdr->reserve = 0;
        fake_hdr->protocol = uint8_t(def::transport_protocol::tcp);
        fake_hdr->total_len = htons(sizeof(struct flow::tcp_hdr));
        // get checksum 
        resp_hdr->tcp_checksum = htons(flow::compute_checksum(resp_buffer));
        // drop fake header
        flow::skb_pull(resp_buffer, sizeof(struct flow::transport_fake_hdr));
        // send stack back
        if (!stack_.expired()) {
            stack_.lock()->write_network_package(resp_buffer);
        }
        // save syn list
        syn_list_.push_back(dst_sock);
    }
}

// accept tcp sock
tcp_sock::ptr tcp_sock::accept() {
    auto sock = accept_queue_.front();
    accept_queue_.pop();
    return sock;
}

}