#include "TBEngine/engine/engine.hpp"
#include "TBEngine/utils/log/log.hpp"
#include "TBEngine/editor/editor.hpp"

#include <any>

extern const TBE::Utils::Log::Logger* logger;

namespace TBE::Engine {
using namespace TBE::Editor;

template<typename... Args>
static std::any bindFuncToAny(Args...) {
    return std::make_any(std::function<void()>());
}

Engine::Engine() 
    : winForm({1280, 720})
    , graphic(winForm)
    , editor(graphic.getImguiInfo(), winForm.getPWindow()) {
    init();
}

Engine::~Engine() {
    exit();
}

void Engine::init() {
    winForm.setResizeFlag(graphic.getPFrameBufferResized());

    auto tickFunc = std::bind(&TBE::Editor::Editor::tickGPU, &editor, std::placeholders::_1);
    graphic.bindTickCmdFunc(tickFunc);

    std::vector<std::tuple<DelegateManager::InputType, std::any>> funcs {};
    std::any func1 = std::function<void(TBE::Editor::DelegateManager::KeyType)>(
        std::bind(&TBE::Engine::Engine::captureKeyInput, this, std::placeholders::_1));
    funcs.push_back(std::make_tuple(DelegateManager::InputType::eKeyBoard, func1));

    std::unordered_map<DelegateManager::InputType, uint32_t> delegateExsist {};
    for(auto&[type, funcAny] : funcs){
        if (auto it = delegateExsist.find(type); it == delegateExsist.end()){
            delegateExsist[type] = editor.addDelegate(type);
        }
        switch(type) {
            case DelegateManager::InputType::eMouseMove:
                editor.bindFunc<uint32_t, uint32_t>(delegateExsist[type], funcAny);
                break;
            case DelegateManager::InputType::eMouseClick:
                editor.bindFunc<bool>(delegateExsist[type], funcAny);
                break;
            case DelegateManager::InputType::eKeyBoard:
                editor.bindFunc<DelegateManager::KeyType>(delegateExsist[type], funcAny);
                break;
            case DelegateManager::InputType::eUnknown:
                Utils::Log::logErrorMsg("bad InputType");
                break;
            default:
                Utils::Log::logErrorMsg("bad InputType");
                break;
        }
    }
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

void Engine::exit() {
    logger->trace("Exiting Engine.");
}

void Engine::tick() {
    winForm.tick();
    graphic.tick();
    editor.tickCPU();
}

} // namespace TBE::Engine
