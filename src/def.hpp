#ifndef __DEF_H__
#define __DEF_H__

#include <cstdint>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>

namespace def {

/**
 * @file def.h
 * @brief device status
 * @author ArisAachen
 * @copyright Copyright (c) 2024 aris All rights reserved
 */
enum class device_status {
    none,
    up,
    down,
    unknown = 10
};

/**
 * @file def.h
 * @brief device type
 * @author ArisAachen
 * @copyright Copyright (c) 2024 aris All rights reserved
 */
enum class device_type {
    none,
    tap,
    tun,
    macvlan,
    unknown = 10
};

/**
 * @file def.h
 * @brief network layer type
 * @author ArisAachen
 * @copyright Copyright (c) 2024 aris All rights reserved
 */
enum class network_protocol : uint16_t {
    none,
    arp = 0x806,
    ip = 0x800,
    ipv6 = 0x86DD,
    unkown = 0xFFFF,
};

/**
 * @file def.h
 * @brief arp operation code
 * @author ArisAachen
 * @copyright Copyright (c) 2024 aris All rights reserved
 */
enum class arp_op_code : uint16_t {
    none,
    request,
    reply,
    unkown = 0xFFFF,
};

struct netlink_request {
    struct nlmsghdr hdr;
    struct ifinfomsg info;
    char data[512];
};

// mac address len
const uint8_t mac_len = 6;

// ether package size
const uint32_t flow_buffer_size = 65535;

// ether broadcast mac address
const uint8_t broadcast_mac[mac_len] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };

// global def ip
const uint32_t global_def_ip = 0xc0a879fd;
}

#endif // __DEF_H__