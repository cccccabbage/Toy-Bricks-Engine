#pragma once

#include "TBEngine/utils/basic/basic.hpp"

#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/spdlog.h>


namespace TBE::Utils::Log {

/**
 * @brief A class for logger, singleton.
 *
 * Change the consoleFlag in the template to control the output to the console.
 * In the whole program, the consoleFlag should be the same, otherwise there would be two log
 * file with the same name or failed to create a log file with an existed file name.
 */
class Logger final {
public:
    static const Logger* getLogger() {
        static Logger logger = Logger();
        return &logger;
    }

private:
    Logger()
        : fileName("Logs/" + getTime() + ".log"),
          logger(spdlog::basic_logger_mt("sbasic_logger", fileName)) {
        logger->set_level(spdlog::level::trace);
        spdlog::set_level(spdlog::level::trace);
        // logger->set_level(spdlog::level::err);
        // spdlog::set_level(spdlog::level::err);
    }

private:
    const std::string                     fileName;
    const std::shared_ptr<spdlog::logger> logger;

public:
    void trace(std::string msg) const;
    void debug(std::string msg) const;
    void info(std::string msg) const;
    void warn(std::string msg) const;
    void error(std::string msg) const;
    void critical(std::string msg) const;

public:
    // TODO: this should be called at a period.
    inline void flush() const { logger->flush(); }
};

} // namespace TBE::Utils::Log

extern const TBE::Utils::Log::Logger* logger;

namespace TBE::Utils::Log {

void logErrorMsg(const std::string& msg);

}
