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
 * @brief transport layer type
 * @author ArisAachen
 * @copyright Copyright (c) 2024 aris All rights reserved
 */
enum class transport_protocol : uint8_t {
    none,
    tcp = 0x6,
    udp = 0x11,
    gre = 0x2F,
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

/**
 * @file def.h
 * @brief sock 
 * @author ArisAachen
 * @copyright Copyright (c) 2024 aris All rights reserved
 */
enum class sock_op_flag : uint8_t {
    none,
    non_block
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

// ip flag offset
const uint8_t ip_flag_offset = 13;

// ether package size
const uint32_t flow_buffer_size = 65535;

// ether broadcast mac address
const uint8_t broadcast_mac[mac_len] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };

// global def ip
// const uint32_t global_def_ip = 0xc0a879fd;
const uint32_t global_def_ip = 0xac1100fd;

// compute checksum div base
const uint8_t checksum_div_base = 2;

// compute checksum sperate base
const uint8_t checksum_sperate_base = 16;

// ip fragment offset base
const uint8_t ip_frag_offset_base = 8;

// checksum max num
const uint16_t checksum_max_num = 0xffff;

// max arp header
const uint8_t max_arp_header = 28;

// max ip header
const uint8_t max_ip_header = 60;

// max ether header
const uint8_t max_ether_header = 14;

// max tcp header
const uint8_t max_tcp_header = 60;

// max udp header should include fake header
const uint8_t max_udp_header = 8 + 12;

// max transport wait time
const uint8_t max_transport_wait_time = 10;

/**
 * @file def.h
 * @brief tcp option code
 * @author ArisAachen
 * @copyright Copyright (c) 2024 aris All rights reserved
 */
enum class tcp_option_kind : uint8_t {
    /// end of option
    eol,
    /// pand option
    nop,
    /// max segment size
    mss,
    /// windows scale 
    scale,
    /// sack perm
    sack_perm,
    /// sack data
    sack,
    /// timestamp
    timestamp,
};

/**
 * @file def.h
 * @brief tcp connection state
 * @author ArisAachen
 * @copyright Copyright (c) 2024 aris All rights reserved
 */
enum class tcp_connection_state {
    none,
    established,
    syn_sent,
    syn_recv,
    fin_wait_1,
    fin_wait_2,
    time_wait,
    close,
    close_wait,
    last_ack,
    listen,
    closing,
    new_syn_recv,
};

enum class transport_sock_type {
    none,
    client,
    server,
};

}

#endif // __DEF_H__