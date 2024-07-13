#pragma once

#include <unordered_map>
#include <mutex>
#include <assimp/scene.h>
#include <iostream>
#include <set>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/GltfMaterial.h>


#include "Model.h"
#include "Config.h"
#include "Cube.h"
#include "../code/base/Buffer.h"
#include "../code/base/ThreadPool.h"
#include "../code/util/ImageUtils.h"
#include "../code/util/StringUtils.h"
#include "../code/log/LogSystem.h"


namespace DynastyEngine
{
    namespace Utils 
    {
        // static glm::mat4 ConvertMatrixToGLMFormat(const aiMatrix4x4& from)
        // 	{
        // 		glm::mat4 to;
        // 		//the a,b,c,d in assimp is the row ; the 1,2,3,4 is the column
        // 		to[0][0] = from.a1; to[1][0] = from.a2; to[2][0] = from.a3; to[3][0] = from.a4;
        // 		to[0][1] = from.b1; to[1][1] = from.b2; to[2][1] = from.b3; to[3][1] = from.b4;
        // 		to[0][2] = from.c1; to[1][2] = from.c2; to[2][2] = from.c3; to[3][2] = from.c4;
        // 		to[0][3] = from.d1; to[1][3] = from.d2; to[2][3] = from.d3; to[3][3] = from.d4;
        // 		return to;
        // 	}

        static void SetVertexBoneDataToDefault(Vertex& vertex) 
        {
            for (int i = 0; i < MAX_BONES_PER_VERTEX; i++) 
            {
                vertex.a_boneIDs[i] = -1;
                vertex.a_boneWeights[i] = 0.0f;
            }
        }
    }

    class ModelLoader 
    {
    public:
        // explicit的主要用法就是放在单参数的构造函数中, 防止隐式转换, 导致函数的入口参数
        explicit ModelLoader(Config &config);

        bool loadModel(const std::string &filepath);
        bool loadSkybox(const std::string &filepath);

        inline DemoScene &getScene() { return scene_; }

        inline size_t getModelPrimitiveCnt() const 
        {
            if (scene_.model) {
                return scene_.model->primitiveCnt;
            }
            return 0;
        }

        inline void resetAllModelStates() 
        {
            for (auto &kv : modelCache_) 
            {
                kv.second->resetStates();
            }

            for (auto &kv : skyboxMaterialCache_) 
            {
                kv.second->resetStates();
            }
        }

        inline glm::mat4 aiMatrix4x4ToGlm(const aiMatrix4x4* from) 
        {
            glm::mat4 to;
            
            to[0][0] = (GLfloat)from->a1; to[0][1] = (GLfloat)from->b1;  to[0][2] = (GLfloat)from->c1; to[0][3] = (GLfloat)from->d1;
            to[1][0] = (GLfloat)from->a2; to[1][1] = (GLfloat)from->b2;  to[1][2] = (GLfloat)from->c2; to[1][3] = (GLfloat)from->d2;
            to[2][0] = (GLfloat)from->a3; to[2][1] = (GLfloat)from->b3;  to[2][2] = (GLfloat)from->c3; to[2][3] = (GLfloat)from->d3;
            to[3][0] = (GLfloat)from->a4; to[3][1] = (GLfloat)from->b4;  to[3][2] = (GLfloat)from->c4; to[3][3] = (GLfloat)from->d4;
            
            return to;
        }

        static void loadCubeMesh(ModelVertexes &mesh);

    private:
        void loadWorldAxis();
        void loadLights();
        void loadFloor();

        bool processNode(const aiNode *ai_node, const aiScene *ai_scene, ModelNode &outNode, glm::mat4 &transform);
        bool processMesh(const aiMesh *ai_mesh, const aiScene *ai_scene, ModelMesh &outMesh);
        void processMaterial(const aiMaterial *ai_material, aiTextureType textureType, Material &material);

        static glm::mat4 convertMatrix(const aiMatrix4x4 &m);
        static BoundingBox convertBoundingBox(const aiAABB &aabb);
        static WrapMode convertTexWrapMode(const aiTextureMapMode &mode);
        static glm::mat4 adjustModelCenter(BoundingBox &bounds);

        void preloadTextureFiles(const aiScene *scene, const std::string &resDir);
        std::shared_ptr<Buffer<RGBA>> loadTextureFile(const std::string &path);

    public: 
        DemoScene scene_;

    private:
        Config &config_;
        
        std::unordered_map<std::string, std::shared_ptr<Model>> modelCache_;
        std::unordered_map<std::string, std::shared_ptr<Buffer<RGBA>>> textureDataCache_;
        std::unordered_map<std::string, std::shared_ptr<SkyboxMaterial>> skyboxMaterialCache_;

        std::mutex modelLoadMutex_;
        std::mutex texCacheMutex_;
    };
}