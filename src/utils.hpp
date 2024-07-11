#include "def.hpp"

#include <optional>
#include <string>

namespace utils {

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
}


}