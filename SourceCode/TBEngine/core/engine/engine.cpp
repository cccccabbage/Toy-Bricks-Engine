#include "engine.hpp"
#include "TBEngine/utils/log/log.hpp"
#include "TBEngine/editor/editor.hpp"
#include "TBEngine/settings.hpp"
#include "TBEngine/enums.hpp"

#include <any>

extern const TBE::Utils::Log::Logger* logger;

namespace TBE::Engine {
using namespace TBE::Editor;

template <typename... Args>
static std::any bindFuncToAny(Args...) {
    return std::make_any(std::function<void()>());
}

Engine::Engine()
    : winForm({WINDOW_WIDTH, WINDOW_HEIGHT})
    , graphic(winForm)
    , editor(graphic.getImguiInfo(), winForm.getPWindow()) {
    winForm.setResizeFlag(graphic.getPFrameBufferResized());

    loadScene();
    graphic.initSceneInterface();

    bindTickGPUFuncs();
    bindCallBackFuncs();
}

Engine::~Engine() {
    logger->trace("Exiting Engine.");
}

void Engine::runLoop() {
    logger->trace("Start of draw loop.");
    logger->flush();

    while ((!winForm.shouldClose()) && (!shouldClose)) {
        tick();
    }

    logger->flush();
    logger->trace("End of draw loop.");
}

void Engine::bindTickGPUFuncs() {
    auto tickFunc = std::bind(&TBE::Editor::Editor::tickGPU, &editor, std::placeholders::_1);
    graphic.bindTickCmdFunc(tickFunc);
}

void Engine::bindCallBackFuncs() {
    std::vector<std::tuple<InputType, std::any>> funcs{};
    std::any                                     func1 = std::function<void(KeyStateMap)>(
        std::bind(&TBE::Engine::Engine::captureKeyInput, this, std::placeholders::_1));
    funcs.push_back(std::make_tuple(InputType::eKeyBoard, func1));
    auto graphicFuncs = scene.getBindFuncs();
    for (auto& typeFunc : graphicFuncs) {
        funcs.push_back(typeFunc);
    }

    std::unordered_map<InputType, uint32_t> delegateExsist{};
    for (auto& [type, funcAny] : funcs) {
        if (auto it = delegateExsist.find(type); it == delegateExsist.end()) {
            delegateExsist[type] = editor.addDelegate(type);
        }
        switch (type) {
            case InputType::eMouseMove:
                editor.bindFunc<uint32_t, uint32_t>(delegateExsist[type], funcAny);
                break;
            case InputType::eMouseClick:
                editor.bindFunc<bool>(delegateExsist[type], funcAny);
                break;
            case InputType::eKeyBoard:
                editor.bindFunc<KeyStateMap>(delegateExsist[type], funcAny);
                break;
            case InputType::eUnknown:
                Utils::Log::logErrorMsg("bad InputType");
                break;
            default:
                Utils::Log::logErrorMsg("bad InputType");
                break;
        }
    }
}

void Engine::loadScene() {
    scene.addShader("Shaders/vert.spv", ShaderType::eVertex);
    scene.addShader("Shaders/frag.spv", ShaderType::eFrag);
    scene.addModel("Resources/Models/viking_room.obj", "Resources/Textures/viking_room.png");
    scene.read();
}

void Engine::tick() {
    winForm.tick();
    scene.tickCPU();
    graphic.tick();
    editor.tickCPU();
}

} // namespace TBE::Engine
