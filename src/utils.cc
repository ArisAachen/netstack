#include "utils.hpp"
#include "def.hpp"

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <pstl/glue_algorithm_defs.h>
#include <sstream>
#include <optional>

#include <fcntl.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <linux/if.h>
#include <linux/if_tun.h>
#include <linux/if_link.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>



namespace utils {


namespace generic {

// format mac address
std::string format_mac_address(uint8_t* src_mac) {
    std::stringstream ss;
    // use hex format
    ss << std::hex << std::setfill('0');
    // format mac address
    for (int index = 0; index < 6; index++) {
        if (index == 0)
            ss << std::setw(2) << (int)src_mac[0];
        else
            ss << ":" << std::setw(2) << (int)src_mac[index];
    }
    return ss.str();
}

// convert string to mac
bool convert_string_to_mac(const std::string& src_mac, uint8_t* dst_mac) {
    std::istringstream iss(src_mac);
    std::string byte_str;
    int i = 0;
    // read mac address
    while (std::getline(iss, byte_str, ':') && i < 6) {
        dst_mac[i] = static_cast<uint8_t>(std::stoi(byte_str, nullptr, 16));
        ++i;
    }
    return true;
}

}


namespace device {

// pre define
std::optional<bool> send_nl_request(struct def::netlink_request& request);
std::optional<int> create_kernel_tap_device(const std::string& dev_name, const std::string& device_path, def::device_type type);

// create kernel tun or tap device
std::optional<int> create_kernel_device(const std::string& dev_name, const std::string& device_path, def::device_type type) {
    switch (type) {
    case def::device_type::tap:
        return create_kernel_tap_device(dev_name, device_path, type);
    default:
        return std::nullopt;
    }
    return std::nullopt;
}

// set kernel device status
std::optional<bool> set_kernel_device_status(const std::string &device_name, def::device_status status) {
    // get interface index by name
    int ifindex = if_nametoindex(device_name.c_str());
    if (ifindex < 0)
        return std::nullopt;
    // create netlink request
    struct def::netlink_request request;
    memset(&request, 0, sizeof(request));
    // set netlink request 
    request.hdr.nlmsg_flags = NLM_F_REQUEST;
    request.hdr.nlmsg_type = RTM_SETLINK;
    request.hdr.nlmsg_len = NLMSG_LENGTH(sizeof(struct ifinfomsg));
    request.hdr.nlmsg_seq = getpid();
    request.hdr.nlmsg_pid =  getpid();
    // set rtnetlink request 
    request.info.ifi_family = AF_UNSPEC;
    request.info.ifi_index = ifindex;
    request.info.ifi_flags = IFF_UP;
    // check if current is up or down
    if (status == def::device_status::up)
        request.info.ifi_change = IFF_UP;
    else if (status == def::device_status::down)
        request.info.ifi_change &= ~IFF_UP;
    return send_nl_request(request);
}

// create kernel macvlan device
std::optional<int> create_kernel_macvlan_device(const std::string& device_name, def::device_type type) {
    // get interface index by name
    int ifindex = if_nametoindex(device_name.c_str());
    if (ifindex < 0)
        return std::nullopt;
    // create netlink request
    struct def::netlink_request request;
    memset(&request, 0, sizeof(request));
    // set netlink request 
    request.hdr.nlmsg_flags = NLM_F_CREATE | NLM_F_REQUEST;
    request.hdr.nlmsg_type = RTM_NEWLINK;
    request.hdr.nlmsg_len = NLMSG_LENGTH(sizeof(struct ifinfomsg));
    request.hdr.nlmsg_seq = getpid();
    request.hdr.nlmsg_pid =  getpid();
    // set rtnetlink request 
    struct rtattr* linkinfo = (struct rtattr*)(&request + NLMSG_ALIGN(request.hdr.nlmsg_len));
    


    return send_nl_request(request);
}

// create kernel tun or tap device
std::optional<int> create_kernel_tap_device(const std::string& dev_name, const std::string& device_path, def::device_type type) {
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


// send netlink request to kernerl
std::optional<bool> send_nl_request(struct def::netlink_request& request) {
    // create connection to kernel
    int fd = socket(AF_NETLINK, SOCK_DGRAM, NETLINK_ROUTE);
    if (fd < 0)
        return std::nullopt;
    if (send(fd, &request, request.hdr.nlmsg_len, 0) < 0)
        return std::nullopt;
    return true;
}


}

}