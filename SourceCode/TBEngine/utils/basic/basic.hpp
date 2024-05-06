#pragma once

#include <string>

namespace TBE::Utils {

/**
 * @brief Set the Root Path object
 *
 * @details change the working path from the folder of executable file to the workspace
 */
void setRootPath();

/**
 * @brief Get current time
 *
 * @return std::string: current time, in the form of "%Y_%m_%d_%H_%M_%S"
 */
std::string getTime();

} // namespace TBE::Utils
