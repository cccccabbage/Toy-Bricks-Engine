#include "TBEngine/core/engine/engine.hpp"
#include "TBEngine/utils/log/log.hpp"


int main() {
    TBE::Utils::setRootPath();
    logger = TBE::Utils::Log::Logger::getLogger();

    logger->trace("Initializing Engine.");
    TBE::Engine::Engine engine{};
    logger->trace("Engine initialized.");
    engine.runLoop();

    return 0;
}
