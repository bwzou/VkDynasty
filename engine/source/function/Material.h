#pragma once

#include <unordered_map>
#include <functional>

#include "../render/PipelineStates.h"
#include "Texture.h"
#include "Uniform.h"
#include "../render/ShaderProgram.h"


enum AlphaMode {
    Alpha_Opaque,
    Alpha_Blend,
};

enum ShadingModel {
    Shading_Unknown = 0,
    Shading_BaseColor,
    Shading_BlinnPhong,
    Shading_PBR,
    Shading_Skybox,
    Shading_IBL_Irradiance,
    Shading_IBL_Prefilter,
    Shading_FXAA,
};

enum MaterialTexType {
    MaterialTexType_NONE = 0,

    MaterialTexType_ALBEDO,
    MaterialTexType_NORMAL,
    MaterialTexType_EMISSIVE,
    MaterialTexType_AMBIENT_OCCLUSION,
    MaterialTexType_METAL_ROUGHNESS,

    MaterialTexType_CUBE,
    MaterialTexType_EQUIRECTANGULAR,

    MaterialTexType_IBL_IRRADIANCE,
    MaterialTexType_IBL_PREFILTER,

    MaterialTexType_QUAD_FILTER,

    MaterialTexType_SHADOWMAP,
};

enum UniformBlockType {
    UniformBlock_Scene,
    UniformBlock_Model,
    UniformBlock_Material,
    UniformBlock_QuadFilter,
    UniformBlock_IBLPrefilter,
};

struct UniformsScene {
    glm::vec3 u_ambientColor;
    glm::vec3 u_cameraPosition;
    glm::vec3 u_pointLightPosition;
    glm::vec3 u_pointLightColor;
};

struct UniformsModel {
    glm::uint32_t u_reverseZ;
    glm::mat4 u_modelMatrix;
    glm::mat4 u_modelViewProjectionMatrix;
    glm::mat3 u_inverseTransposeModelMatrix;
    glm::mat4 u_shadowMVPMatrix;
};

struct UniformsMaterial {
    glm::uint32_t u_enableLight;
    glm::uint32_t u_enableIBL;
    glm::uint32_t u_enableShadow;

    glm::float32_t u_pointSize;
    glm::float32_t u_kSpecular;
    glm::vec4 u_baseColor;
};

struct UniformsQuadFilter {
    glm::vec2 u_screenSize;
};

struct UniformsIBLPrefilter {
    glm::float32_t u_srcResolution;
    glm::float32_t u_roughness;
};

struct TextureData {
    std::string tag;
    size_t width = 0;
    size_t height = 0;
    std::vector<std::shared_ptr<Buffer<RGBA>>> data;
    WrapMode wrapModeU = Wrap_REPEAT;
    WrapMode wrapModeV = Wrap_REPEAT;
    WrapMode wrapModeW = Wrap_REPEAT;
};

class MaterialObject {
public:
    ShadingModel shadingModel = Shading_Unknown;
    std::shared_ptr<PipelineStates> pipelineStates;
    std::shared_ptr<ShaderProgram> shaderProgram;
    std::shared_ptr<ShaderResources> shaderResources;
};


