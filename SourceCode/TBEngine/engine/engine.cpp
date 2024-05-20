#include "TBEngine/engine/engine.hpp"

#include "TBEngine/utils/log/log.hpp"

extern const TBE::Utils::Log::Logger* logger;

namespace TBE::Engine
{

Engine::Engine() : winForm({1280, 720}), graphic(winForm), ui()
{
    init();
}

Engine::~Engine()
{
    exit();
}

void Engine::init()
{
    winForm.setResizeFlag(graphic.getPFrameBufferResized());
    auto info    = graphic.getImguiInfo();
    auto pWindow = winForm.getPWindow();
    ui.init(std::make_tuple(info, pWindow));

    auto tickFunc = [this](const vk::CommandBuffer& cmdBuffer) { ui.tick(cmdBuffer); };
    graphic.bindTickCmdFunc(tickFunc);
}

void Engine::runLoop()
{
    logger->trace("Start of draw loop.");
    logger->flush();

    while (!winForm.shouldClose())
    {
        tick();
    }

    logger->flush();
    logger->trace("End of draw loop.");
}

void Engine::exit()
{
    logger->trace("Exiting Engine.");
}

void Engine::tick()
{
    winForm.tick();
    graphic.tick();
}

} // namespace TBE::Engine
