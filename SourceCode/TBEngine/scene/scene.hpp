#pragma once

#include "shader/shader.hpp"
#include "model/model.hpp"
#include "camera/camera.hpp"
#include "TBEngine/enums.hpp"

#include <vector>
#include <string_view>
#include <tuple>
#include <any>

namespace TBE::Scene {

class Scene {
public:
    std::vector<std::tuple<InputType, std::any>> getBindFuncs();

public:
    void tickCPU();

public: // model related
    // call addModel(...) for all the models needed to read before calling read();
    void   read();
    size_t addModel(std::string_view modelPath, std::string_view texturePath);

public: // shader related
    void addShader(std::string filePath, ShaderType type) {
        shaderManager.addShader(filePath, type);
    }

public:
    auto getIdxSize(uint32_t idx) { return modelManager.getIdxSize(idx); }

private:
    Camera                  camera{};
    Resource::ShaderManager shaderManager{};
    Model::ModelManager     modelManager{};
    uint32_t                currentFrame = 0;

private:
    void updateUniformBuffer();
};

} // namespace TBE::Scene
