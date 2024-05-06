#include "TBEngine/engine/engine.hpp"

#include "TBEngine/utils/log/log.hpp"

extern const TBE::Utils::Log::Logger* logger;

namespace TBE::Engine {

Engine::Engine() {}

void Engine::init() {
    logger->trace("Initializing Engine.");

    winForm.init();
    graphic.initVulkan();

    winForm.setResizeFlag(graphic.getPFrameBufferResized());

    logger->trace("Engine initialized.");
}

void Engine::runLoop() {
    logger->trace("Start of draw loop.");
    logger->flush();

    while (!winForm.shouldClose()) {
        tick();
    }

    logger->flush();
    logger->trace("End of draw loop.");
}

void Engine::exit() {
    logger->trace("Exiting Engine.");

    graphic.cleanup();
    winForm.exit();

    logger->trace("Engine exited.");
}

void Engine::tick() {
    winForm.tick();
    graphic.tick();
}

} // namespace TBE::Engine
