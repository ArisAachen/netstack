#include "utils.hpp"

#include <cstring>
#include <fcntl.h>
#include <optional>

#include <sys/ioctl.h>
#include <linux/if.h>
#include <linux/if_tun.h>
#include <linux/rtnetlink.h>



namespace utils {

namespace device {

/**
 * @brief get kernel device status
 * @param[in] dev_name device name
 */
std::optional<bool> get_kernel_device_status(const std::string& dev_name) {
    // 


    return std::nullopt;
}


/**
 * @brief create kernel device
 * @param[in] dev_name device name
 * @param[in] type device type
 * @return 0 success, < 0 fail
 */
std::optional<int> create_kernel_device(const std::string& dev_name, const std::string& device_path, def::device_type type) {
    // try to open tun device
    int fd = open(device_path.c_str(), O_RDWR);
    if (fd < 0)
        return std::nullopt;
    // create ifreq
    struct ifreq ifr;
    memset(&ifr, 0, sizeof(ifr));
    // set ifreq
    memcpy(&ifr.ifr_name, dev_name.c_str(), dev_name.length());
    ifr.ifr_flags |= IFF_NO_PI | IFF_NAPI | IFF_NAPI_FRAGS;
    // check if device is tun or tap
    if (type == def::device_type::tun) {
        ifr.ifr_flags |= IFF_TUN;
    } else if (type == def::device_type::tap) {
        ifr.ifr_flags |= IFF_TAP;
    } else {
        return std::nullopt;
    }
    // set device type 
    if (ioctl(fd, TUNSETIFF, &ifr) < 0)
        return std::nullopt;
    return fd;
}

}

}