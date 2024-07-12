#include "stack.hpp"
#include "arp.hpp"
#include "def.hpp"
#include "macvlan.hpp"

#include <iostream>
#include <utility>

namespace stack {

// get stack instance
stack::ptr stack::get_instance() {
    static stack::ptr instance = stack::ptr(new stack());
    return instance;
}

stack::stack() {
    // register macvlan device
    register_device(0, driver::macvlan_device::ptr(new driver::macvlan_device("new_eth0", "", "f6:34:95:26:90:66")));
    register_network_handler(def::network_protocol::arp, protocol::arp::ptr(new protocol::arp()));
}

void stack::write_to_device(flow::sk_buff::ptr buffer, uint8_t device_id) {
    auto device = device_map_.find(device_id);
    if (device != device_map_.end()) {
        device->second->write_to_device(buffer);
    } else {
        device_map_.begin()->second->write_to_device(buffer);
    }
}

void stack::run() {
    handle_packege();
    run_read_device();
}

void stack::run_read_device() {
    for (auto& device : device_map_) {
        device.second->up();
        // read buffer from device
        thread_vec_.push_back(std::thread(&interface::net_device::read_thread, device.second));
        // write buffer from device
        // thread_vec_.push_back(std::thread(&interface::net_device::write_thread, device.second));
    }
}

void stack::handle_packege() {
    // handle all packages
    for (auto& device : device_map_) {
        auto thread = std::thread([&] {
            while (true) {
                // read from device
                auto buffer = device.second->read_from_device();
                if (buffer == nullptr)
                    continue;
                // search handle 
                handle_network_package(buffer);
            }
        });
        thread_vec_.push_back(std::move(thread));
    }
}

// handle network package
void stack::handle_network_package(flow::sk_buff::ptr buffer) {
    // get network handler
    auto handler = network_handler_map_.find(def::network_protocol(buffer->protocol));
    if (handler == network_handler_map_.end()) {
        std::cout << "recv unknown network protocol flow" << std::endl;
        return;
    }
    // handle flow
    handler->second->unpack_flow(buffer);
}

stack::~stack() {
    for (auto& thread : thread_vec_) {
        thread.join();
    }
    for (auto& device : device_map_) {
        device.second->down();
    }
    // release device map
    device_map_.clear();
    // release network handler map
    network_handler_map_.clear();
}

}