#pragma once

#include "TBEngine/resource/shader/shader.hpp"
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
    void destroy();

public:
    std::vector<std::tuple<InputType, std::any>> getBindFuncs();

public:
    void tickCPU();

public: // model related
    // call addModel(...) for all the models needed to read before call read();
    void read();
    void addModel(std::string_view modelPath, std::string_view texturePath);

public: // shader related
    void addShader(std::string filePath, ShaderType type) { shader.addShader(filePath, type); }

    void destroyShaderCache();

public:
    auto getIdxSize(uint32_t idx) { return modelManager.getIdxSize(idx); }

public:
    Resource::ShaderManager shader{};

private:
    Camera              camera{};
    Model::ModelManager modelManager{};
    uint32_t            currentFrame = 0;

private:
    void updateUniformBuffer();
};

} // namespace TBE::Scene
