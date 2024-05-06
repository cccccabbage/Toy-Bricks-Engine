#include "TBEngine/utils/log/log.hpp"


#ifdef NDEBUG
const bool inDebug = false;
#else
const bool inDebug = true;
#endif

namespace TBE::Utils::Log {

void Logger::trace(std::string msg) const {
    if (inDebug) spdlog::trace(msg);
    logger->trace(msg);
}
void Logger::debug(std::string msg) const {
    if (inDebug) spdlog::debug(msg);
    logger->debug(msg);
}
void Logger::info(std::string msg) const {
    if (inDebug) spdlog::info(msg);
    logger->info(msg);
}
void Logger::warn(std::string msg) const {
    if (inDebug) spdlog::warn(msg);
    logger->warn(msg);
}
void Logger::error(std::string msg) const {
    if (inDebug) spdlog::error(msg);
    logger->error(msg);
}
void Logger::critical(std::string msg) const {
    if (inDebug) spdlog::critical(msg);
    logger->critical(msg);
}

void logErrorMsg(const std::string& msg) {
    logger->error(msg);
    throw std::runtime_error(msg);
}

} // namespace TBE::Utils::Log
