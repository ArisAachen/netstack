#ifndef __DEF_H__
#define __DEF_H__

#include <string_view>


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
    unknown = 10
};

const std::string_view tun_device_path = "/dev/net/tun";

}

#endif // __DEF_H__