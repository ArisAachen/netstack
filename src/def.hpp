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
 * @brief hardware type
 * @author ArisAachen
 * @copyright Copyright (c) 2024 aris All rights reserved
 */
enum class hardware_type : uint16_t {
    none,
    ethernet = 0x1,
    unknown = 0xFFFF,
};

/**
 * @file def.h
 * @brief network layer type
 * @author ArisAachen
 * @copyright Copyright (c) 2024 aris All rights reserved
 */
enum class network_protocol : uint16_t {
    none,
    icmp = 0x1,
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

/**
 * @file def.h
 * @brief ip version 
 * @author ArisAachen
 * @copyright Copyright (c) 2024 aris All rights reserved
 */
enum class ip_version : uint8_t {
    none = 0,
    ipv4 = 0x4,
    ipv6 = 0x6,
};


/**
 * @file def.h
 * @brief icmp type
 * @author ArisAachen
 * @copyright Copyright (c) 2024 aris All rights reserved
 */
enum class icmp_type: uint8_t {
    reply = 0x0,
    request = 0x8,
};

enum class icmp_code : uint8_t {
    none = 0x0,
};

struct netlink_request {
    struct nlmsghdr hdr;
    struct ifinfomsg info;
    char data[512];
};

// mac address len
const uint8_t mac_len = 6;

// crc len
const uint8_t crc_len = 4;

// ip protocol len
const uint8_t ip_len = 4;

// ip time to live
const uint8_t ip_time_to_live = 64;

// ether package size
const uint32_t flow_buffer_size = 65535;

// ether broadcast mac address
const uint8_t broadcast_mac[mac_len] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };

// global def ip
const uint32_t global_def_ip = 0xc0a879fd;

// compute checksum div base
const uint8_t checksum_div_base = 2;

// compute checksum sperate base
const uint8_t checksum_sperate_base = 16;

// checksum max num
const uint16_t checksum_max_num = 0xffff;
}

#endif // __DEF_H__