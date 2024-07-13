#pragma once


#include <vulkan/vulkan.h>
#include <memory>
#include <string>
#include <array>
#include <unordered_map>

#include "Geometry.h"
#include "Vertex.h"
#include "Material.h"
// #include "Skinned.h"
#include "./Animdata.h"
#include "./Animation.h"
#include "./Animator.h"


namespace DynastyEngine
{

    static VkFormat vertexAttributeFormat(size_t size) 
    {
        switch (size) 
        {
            case 1: return VK_FORMAT_R32_SFLOAT;
            case 2: return VK_FORMAT_R32G32_SFLOAT;
            case 3: return VK_FORMAT_R32G32B32_SFLOAT;
            case 4: return VK_FORMAT_R32G32B32A32_SFLOAT;
            case 5: return VK_FORMAT_R32G32B32A32_SINT;
            default:
                break;
        }
        return VK_FORMAT_UNDEFINED;
    }

    struct Vertex 
    {
        glm::vec3 a_position;
        glm::vec2 a_texCoord;
        glm::vec3 a_normal;
        glm::vec3 a_tangent;
        glm::vec4 a_boneWeights;
        glm::ivec4 a_boneIDs;
    };

    struct ModelVertexes : VertexArray 
    {
        PrimitiveType primitiveType;
        size_t primitiveCnt = 0;
        std::vector<Vertex> vertexes;
        std::vector<int32_t> indices;

        std::shared_ptr<VertexArrayObject> vao = nullptr;

        void UpdateVertexes() const 
        {
            if (vao) 
            {
                vao->updateVertexData(vertexesBuffer, vertexesBufferLength);
            }
        };

        void InitVertexes() 
        {
            vertexSize = sizeof(Vertex);

            vertexesDesc.resize(6);
            vertexesDesc[0] = {3, sizeof(Vertex), offsetof(Vertex, a_position)};
            vertexesDesc[1] = {2, sizeof(Vertex), offsetof(Vertex, a_texCoord)};
            vertexesDesc[2] = {3, sizeof(Vertex), offsetof(Vertex, a_normal)};
            vertexesDesc[3] = {3, sizeof(Vertex), offsetof(Vertex, a_tangent)};
            vertexesDesc[4] = {4, sizeof(Vertex), offsetof(Vertex, a_boneWeights)};
            vertexesDesc[5] = {5, sizeof(Vertex), offsetof(Vertex, a_boneIDs)}; 

            vertexesBuffer = vertexes.empty() ? nullptr : (uint8_t *) &vertexes[0];
            vertexesBufferLength = vertexes.size() * sizeof(Vertex);

            indexBuffer = indices.empty() ? nullptr : &indices[0];
            indexBufferLength = indices.size() * sizeof(int32_t);
        }
    };

    struct ModelBase : ModelVertexes 
    {
        BoundingBox aabb{};
        std::shared_ptr<Material> material = nullptr;
        
        virtual void resetStates() 
        {
            vao = nullptr;
            if (material) 
            {
                material->resetStates();
            }
        }
    };

    struct ModelPoints : ModelBase {};

    struct ModelLines : ModelBase {};

    struct ModelMesh : ModelBase {};

    struct ModelNode 
    {
        glm::mat4 transform = glm::mat4(1.f);

        std::vector<ModelMesh> meshes;
        std::vector<ModelNode> children;
        // Skinned *skinned = nullptr;
    };

    class Model 
    {
    public:
        std::string resourcePath;

        ModelNode rootNode;
        BoundingBox rootAABB;

        size_t meshCnt = 0;
        size_t primitiveCnt = 0;
        size_t vertexCnt = 0;

        glm::mat4 centeredTransform;

        // Animation
        bool bAnimated = false;
        bool bPlayAnim = false;
        bool bStopAnim = false;

        Animation mAnimation;
        Animator mAnimator;
        
        float mAnimPlaySpeed = 1.0f;

        int mBoneCounter = 0;
        std::map<std::string, BoneInfo> mBoneInfoMap;

        auto& GetBoneInfoMap() { return mBoneInfoMap; }
        int& GetBoneCount() { return mBoneCounter; }

        void ReadMissingBones(const aiAnimation* animation) 
        {
            int size = animation->mNumChannels;

            // reading channels(bones engaged in an animation and their keyframes)
            for (int i = 0; i < size; i++) 
            {
                auto channel = animation->mChannels[i];
                std::string boneName = std::string(channel->mNodeName.data);
                
                if (mBoneInfoMap.find(boneName) == mBoneInfoMap.end()) 
                {
                    mBoneInfoMap[boneName].id = mBoneCounter;
                    mBoneCounter++;
                }
                mAnimation.mBones.push_back(Bone(channel->mNodeName.data,
                mBoneInfoMap[channel->mNodeName.data].id, channel));
            }
            
            mAnimation.mBoneInfoMap = mBoneInfoMap;
        }


        void resetStates() 
        {
            resetNodeStates(rootNode);
        }

        void resetNodeStates(ModelNode &node) 
        {
            for (auto &mesh : node.meshes) 
            {
                mesh.resetStates();
            }
            for (auto &childNode : node.children) 
            {
                resetNodeStates(childNode);
            }
        }
    };

    struct DemoScene 
    {
        std::shared_ptr<Model> model;
        ModelLines worldAxis;
        ModelPoints pointLight;
        ModelMesh floor;
        ModelMesh skybox;

        void resetStates() 
        {
            if (model) { model->resetStates(); }
            worldAxis.resetStates();
            pointLight.resetStates();
            floor.resetStates();
            skybox.resetStates();
        }
    };
}