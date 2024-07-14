#include "raw_stack.hpp"
#include "arp.hpp"
#include "def.hpp"
#include "macvlan.hpp"

#include <algorithm>
#include <iostream>

namespace stack {

// get raw_stack instance
raw_stack::ptr raw_stack::get_instance() {
    static raw_stack::ptr instance = raw_stack::ptr(new raw_stack());
    return instance;
}

raw_stack::raw_stack() {
    // register macvlan device
    register_device(driver::macvlan_device::ptr(new driver::macvlan_device("new_eth0", "192.168.121.253", "f6:34:95:26:90:66")));
    register_network_handler(protocol::arp::ptr(new protocol::arp()));
}

// write buffer to device
void raw_stack::write_to_device(flow::sk_buff::ptr buffer) {
    // check if device exist
    if (!buffer->dev.expired()) {
        auto dev = buffer->dev.lock();
        dev->write_to_device(buffer);
    }
    // find device
    if (device_map_.empty())
        return;
    device_map_.begin()->second->write_to_device(buffer);
}

void raw_stack::run() {
    handle_packege();
    run_read_device();
}

// wait signal to end
void raw_stack::wait() {
    for (auto& thread : thread_vec_) {
        thread.join();
    }
    for (auto& device : device_map_) {
        device.second->down();
    }
}

void raw_stack::run_read_device() {
    for (auto& device : device_map_) {
        device.second->up();
        // read buffer from device
        thread_vec_.push_back(std::thread(&interface::net_device::read_thread, device.second));
        // write buffer from device
        thread_vec_.push_back(std::thread(&interface::net_device::write_thread, device.second));
    }
}

void raw_stack::handle_packege() {
    // handle all packages
    for (auto& device : device_map_) {
        auto thread = std::thread([&] {
            while (true) {
                // read from device
                auto buffer = device.second->read_from_device();
                if (buffer == nullptr)
                    continue;
                buffer->stack = weak_from_this();
                // search handle 
                handle_network_package(buffer);
            }
        });
        thread_vec_.push_back(std::move(thread));
    }
}

// handle network package
void raw_stack::handle_network_package(flow::sk_buff::ptr buffer) {
    // get network handler
    auto handler = network_handler_map_.find(def::network_protocol(buffer->protocol));
    if (handler == network_handler_map_.end()) {
        std::cout << "recv unknown network protocol flow" << std::endl;
        return;
    }
    // handle flow
    handler->second->unpack_flow(buffer);
}

raw_stack::~raw_stack() {
    std::cout << "stack release" << std::endl;
    // release device map
    device_map_.clear();
    // release network handler map
    network_handler_map_.clear();
}

}