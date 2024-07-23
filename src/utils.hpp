#include "def.hpp"

#include <cstdint>
#include <optional>
#include <string>

namespace utils {


namespace generic {

/**
 * @brief format mac output
 * @param[in] src_mac source mac address
 * @return format mac address
 */
std::string format_mac_address(const uint8_t* src_mac);

/**
 * @brief format ip output
 * @param[in] src_ip source ip address
 * @return format ip address
 */
std::string format_ip_address(const uint32_t src_ip);

/**
 * @brief convert mac back
 * @param[in] src_mac source mac address
 * @param[out] dst_mac dst mac address
 * @return if convert success
 */
bool convert_string_to_mac(const std::string& src_mac, uint8_t* dst_mac);

/**
 * @brief convert ip back
 * @param[in] src_ip source mac address
 * @param[out] dst_ip dst mac address
 * @return if convert success
 */
bool convert_string_to_ip(const std::string& src_ip, uint32_t* dst_ip);

/**
 * @brief get jenkins hash
 * @param[in] first first num
 * @param[in] second second num
 * @param[in] third third num
 * @return hash result
 */
uint32_t jhash_3words(uint32_t first, uint32_t second, uint32_t third);
}


namespace device {
/**
 * @brief get kernel device status
 * @param[in] dev_name device name
 * @return device status, std::nullopt fail
 */
std::optional<bool> get_kernel_device_status(const std::string& dev_name);


/**
 * @brief create kernel device
 * @param[in] dev_name device name
 * @param[in] type device type
 * @return ifindex success, std::nullopt fail
 */
std::optional<int> create_kernel_device(const std::string& dev_name, const std::string& device_path, def::device_type type);

/**
 * @brief set kernel device status
 * @param[in] dev_name device name
 * @param[in] status device status
 * @return true success, false fail
 */
std::optional<bool> set_kernel_device_status(const std::string& device_name, def::device_status status);

}



namespace algorithm {

}

}