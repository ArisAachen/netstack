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
 * @brief convert mac back
 * @param[in] src_mac source mac address
 * @param[out] dst_mac dst mac address
 * @return if convert success
 */
bool convert_string_to_mac(const std::string& src_mac, uint8_t* dst_mac);
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


}