#include "stack.hpp"
#include "macvlan.hpp"

namespace stack {

// get stack instance
stack::ptr stack::get_instance() {
    static stack::ptr instance = stack::ptr(new stack());
    return instance;
}

stack::stack() {
    // register macvlan device
    register_device(0, driver::macvlan_device::ptr(new driver::macvlan_device("new_eth0", "", "f6:34:95:26:90:66")));
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
    run_read_device();
    // handle_packege();
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
                
            }
        });
        // thread_vec_.push_back(thread);
    }
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