#include "TBEngine/engine/engine.hpp"
#include "TBEngine/utils/log/log.hpp"


const TBE::Utils::Log::Logger* logger = nullptr;

int main() {
    TBE::Utils::setRootPath();
    logger = TBE::Utils::Log::Logger::getLogger();

    TBE::Engine::Engine engine{};
    engine.init();
    engine.runLoop();
    engine.exit();

    return 0;
}
