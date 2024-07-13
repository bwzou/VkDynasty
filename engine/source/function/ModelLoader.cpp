#include "ModelLoader.h"

namespace DynastyEngine
{
    ModelLoader::ModelLoader(Config &config) : config_(config) 
    {
        loadWorldAxis();
        loadLights();
        loadFloor();
    }


    void ModelLoader::loadCubeMesh(ModelVertexes &mesh) 
    {
        const float *cubeVertexes = Cube::getCubeVertexes();

        mesh.primitiveType = Primitive_TRIANGLE;
        mesh.primitiveCnt = 12;

        for (int i = 0; i < 12; i++) 
        {
            for (int j = 0; j < 3; j++) 
            {
                Vertex vertex{};
                vertex.a_position.x = cubeVertexes[i * 9 + j * 3 + 0];
                vertex.a_position.y = cubeVertexes[i * 9 + j * 3 + 1];
                vertex.a_position.z = cubeVertexes[i * 9 + j * 3 + 2];
                mesh.vertexes.push_back(vertex);
                mesh.indices.push_back(i * 3 + j);
            }
        }
        mesh.InitVertexes();
    }


    void ModelLoader::loadWorldAxis() 
    {
        float axisY = -0.01f;
        int idx = 0;
        for (int i = -32; i<= 32; i++) 
        {
            scene_.worldAxis.vertexes.push_back({glm::vec3(-6.4, axisY, 0.2f * (float) i)});
            scene_.worldAxis.vertexes.push_back({glm::vec3(6.4, axisY, 0.2f * (float) i)});
            scene_.worldAxis.indices.push_back(idx++);
            scene_.worldAxis.indices.push_back(idx++);

            scene_.worldAxis.vertexes.push_back({glm::vec3(0.2f * (float) i, axisY, -6.4)});
            scene_.worldAxis.vertexes.push_back({glm::vec3(0.2f * (float) i, axisY, 6.4)});
            scene_.worldAxis.indices.push_back(idx++);
            scene_.worldAxis.indices.push_back(idx++);
        }
        scene_.worldAxis.InitVertexes();

        scene_.worldAxis.primitiveType = Primitive_LINE;
        scene_.worldAxis.primitiveCnt = scene_.worldAxis.indices.size() / 2;
        scene_.worldAxis.material = std::make_shared<Material>();
        scene_.worldAxis.material->shadingModel = Shading_BaseColor;
        scene_.worldAxis.material->baseColor = glm::vec4(0.25f, 0.25f, 0.25f, 1.f);
        scene_.worldAxis.material->lineWidth = 1.f;
    }


    void ModelLoader::loadLights()
    {
        scene_.pointLight.primitiveType = Primitive_POINT;
        scene_.pointLight.primitiveCnt = 1;
        scene_.pointLight.vertexes.resize(scene_.pointLight.primitiveCnt);
        scene_.pointLight.indices.resize(scene_.pointLight.primitiveCnt);

        scene_.pointLight.vertexes[0] =  {config_.pointLightPosition};
        scene_.pointLight.indices[0] = 0;
        scene_.pointLight.material = std::make_shared<Material>();
        scene_.pointLight.material->shadingModel = Shading_BaseColor;
        scene_.pointLight.material->baseColor = glm::vec4(config_.pointLightColor, 1.f);
        scene_.pointLight.material->pointSize = 10.f;
        scene_.pointLight.InitVertexes();
    }


    void ModelLoader::loadFloor() 
    {
        float floorY = 0.01f;
        float floorSize = 4.0f;

        scene_.floor.vertexes.push_back({glm::vec3(-floorSize, floorY, floorSize), glm::vec2(0.f, 1.f),
                                    glm::vec3(0.f, 1.f, 0.f)});
        scene_.floor.vertexes.push_back({glm::vec3(-floorSize, floorY, -floorSize), glm::vec2(0.f, 0.f),
                                        glm::vec3(0.f, 1.f, 0.f)});
        scene_.floor.vertexes.push_back({glm::vec3(floorSize, floorY, -floorSize), glm::vec2(1.f, 0.f),
                                        glm::vec3(0.f, 1.f, 0.f)});
        scene_.floor.vertexes.push_back({glm::vec3(floorSize, floorY, floorSize), glm::vec2(1.f, 1.f),
                                        glm::vec3(0.f, 1.f, 0.f)});
        scene_.floor.indices.push_back(0);
        scene_.floor.indices.push_back(2);
        scene_.floor.indices.push_back(1);
        scene_.floor.indices.push_back(0);
        scene_.floor.indices.push_back(3);
        scene_.floor.indices.push_back(2);    

        scene_.floor.primitiveType = Primitive_TRIANGLE;
        scene_.floor.primitiveCnt = 2;                             

        scene_.floor.material = std::make_shared<Material>();
        scene_.floor.material->shadingModel = Shading_BlinnPhong;
        scene_.floor.material->baseColor = glm::vec4(1.0f);
        scene_.floor.material->doubleSided = true;

        scene_.floor.aabb = BoundingBox(glm::vec3(-4, 0, -4), glm::vec3(4, 0, 4));
        scene_.floor.InitVertexes();
    }


    bool ModelLoader::loadSkybox(const std::string &filepath) 
    {
        if (filepath.empty()) 
        {
            return false;
        }

        if (scene_.skybox.primitiveCnt <= 0) 
        {
            loadCubeMesh(scene_.skybox);
        }

        auto it = skyboxMaterialCache_.find(filepath);
        if (it != skyboxMaterialCache_.end()) 
        {
            scene_.skybox.material = it->second;
            return true;
        }

        std::cout << "load skybox, path: %s" << filepath.c_str() << std::endl;
        auto material = std::make_shared<SkyboxMaterial>();
        material->shadingModel = Shading_Skybox;

        std::vector<std::shared_ptr<Buffer<RGBA>>> skyboxTex;
        if (StringUtils::endsWith(filepath, "/")) 
        {
            skyboxTex.resize(6);

            ThreadPool pool(6);
            pool.pushTask([&](int thread_id) {skyboxTex[0] = loadTextureFile(filepath + "right.jpg"); });
            pool.pushTask([&](int thread_id) {skyboxTex[1] = loadTextureFile(filepath + "left.jpg"); });
            pool.pushTask([&](int thread_id) {skyboxTex[2] = loadTextureFile(filepath + "top.jpg"); });
            pool.pushTask([&](int thread_id) {skyboxTex[3] = loadTextureFile(filepath + "bottom.jpg"); });
            pool.pushTask([&](int thread_id) {skyboxTex[4] = loadTextureFile(filepath + "front.jpg"); });
            pool.pushTask([&](int thread_id) {skyboxTex[5] = loadTextureFile(filepath + "back.jpg"); });
            pool.waitTasksFinish();

            auto &texData = material->textureData[MaterialTexType_CUBE];
            texData.tag = filepath;
            texData.width = skyboxTex[0]->getWidth();
            texData.height = skyboxTex[0]->getHeight();
            texData.data = std::move(skyboxTex);
            texData.wrapModeU = Wrap_CLAMP_TO_EDGE;
            texData.wrapModeV = Wrap_CLAMP_TO_EDGE;
            texData.wrapModeW = Wrap_CLAMP_TO_EDGE;
        } 
        else 
        {
            skyboxTex.resize(1);
            skyboxTex[0] = loadTextureFile(filepath);

            auto &texData = material->textureData[MaterialTexType_EQUIRECTANGULAR];
            texData.tag = filepath;
            texData.width = skyboxTex[0]->getWidth();
            texData.height = skyboxTex[0]->getHeight();
            texData.data = std::move(skyboxTex);
            texData.wrapModeU = Wrap_CLAMP_TO_EDGE;
            texData.wrapModeV = Wrap_CLAMP_TO_EDGE;
            texData.wrapModeW = Wrap_CLAMP_TO_EDGE;
        }

        skyboxMaterialCache_[filepath] = material;
        scene_.skybox.material = material;
        return true;
    }


    bool ModelLoader::loadModel(const std::string &filepath) 
    {
        // using a local lock_guard to lock modelLoadMutex_ guarantees unlocking on destruction / exception:
        std::lock_guard<std::mutex> lk(modelLoadMutex_);
        if (filepath.empty()) 
        {
            return false;
        }

        auto it = modelCache_.find(filepath);
        if (it != modelCache_.end()) 
        {
            scene_.model = it -> second;
            return true;
        }

        modelCache_[filepath] = std::make_shared<Model>();
        scene_.model  = modelCache_[filepath];

        std::cout << "load model, path: %s" << filepath.c_str() << std::endl;

        // load model
        Assimp::Importer importer;
        // 添加下一个代码，您将得到一个独立节点， 修复用ASSIMP导入程序加载文件时，就会添加原始fbx文件中不存在的奇怪节点的问题。
        importer.SetPropertyBool(AI_CONFIG_IMPORT_FBX_PRESERVE_PIVOTS, false);
        if (filepath.empty()) 
        {
            std::cout << "ModelLoader::loadModel, empty model file path." << std::endl;
            return false;
        }

        // aiScene 这是Assimp数据接口的根对象。一旦我们有了这个场景对象，我们就能访问到加载后的模型中所有所需的数据了。
        // aiProcess_Triangulate，我们告诉Assimp，如果模型不是（全部）由三角形组成，它需要将模型所有的图元形状变换为三角形。
        // aiProcess_FlipUVs将在处理的时候翻转y轴的纹理坐标。
        // aiProcess_GenNormals：如果模型不包含法向量的话，就为每个顶点创建法线。
        // aiProcess_SplitLargeMeshes：将比较大的网格分割成更小的子网格，如果你的渲染有最大顶点数限制，只能渲染较小的网格，那么它会非常有用。
        // aiProcess_OptimizeMeshes：和上个选项相反，它会将多个小网格拼接为一个大的网格，减少绘制调用从而进行优化
        const aiScene *scene = importer.ReadFile(filepath, 
        aiProcess_Triangulate 
        | aiProcess_CalcTangentSpace 
        | aiProcess_SortByPType 
        | aiProcess_GenNormals 
        | aiProcess_GenUVCoords 
        | aiProcess_ValidateDataStructure 
        | aiProcess_FlipUVs 
        | aiProcess_GenBoundingBoxes);
        // 检查场景和其根节点不为null，并且检查了它的一个标记(Flag)，来查看返回的数据是不是不完整的。
        if (!scene || scene->mFlags == AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) 
        {
            std::cout << "ModelLoader::loadModel, description: %s" << importer.GetErrorString() << std::endl;
            return false;
        }

        scene_.model->resourcePath = filepath.substr(0, filepath.find_last_of('/'));

        // preload textures
        preloadTextureFiles(scene, scene_.model->resourcePath);

        auto currTransform = glm::mat4(1.f);
        if (scene->HasAnimations()) 
        {
            scene_.model->bAnimated = true;
            if (!processNode(scene->mRootNode, scene, scene_.model->rootNode, currTransform)) 
            {
                std::cout << "ModelLoader::loadModel, process node failed." << std::endl;
                return false;
            }
            scene_.model->mAnimation = Animation(scene);
            // 读取骨骼数据
            scene_.model->ReadMissingBones(scene_.model->mAnimation.mAnimation);
            scene_.model->mAnimator = Animator(&scene_.model->mAnimation);

        } 
        else 
        {
            if (!processNode(scene->mRootNode, scene, scene_.model->rootNode, currTransform)) 
            {
                std::cout << "ModelLoader::loadModel, process node failed." << std::endl;
                return false;
            }
        }

        // model center transform
        scene_.model->centeredTransform = adjustModelCenter(scene_.model->rootAABB);
        return true;
    }


    // processNode递归处理结点载入mesh类
    bool ModelLoader::processNode(const aiNode *ai_node, const aiScene *ai_scene, ModelNode &outNode, glm::mat4 &transform) 
    {
        if (!ai_node) 
        {
            return false;
        }

        outNode.transform = Utils::ConvertMatrixToGLMFormat(ai_node->mTransformation);
        auto currTransform = transform * outNode.transform;

        for (size_t i = 0; i < ai_node->mNumMeshes; i++) 
        {
            const aiMesh *meshPtr = ai_scene->mMeshes[ai_node->mMeshes[i]];
            if (meshPtr) 
            {
                ModelMesh mesh;
                if (processMesh(meshPtr, ai_scene, mesh)) 
                {
                    scene_.model->meshCnt++;
                    scene_.model->primitiveCnt += mesh.primitiveCnt;
                    scene_.model->vertexCnt += mesh.vertexes.size();

                    // bounding box
                    auto bounds = mesh.aabb.transform(currTransform);
                    scene_.model->rootAABB.merge(bounds);
                    
                    outNode.meshes.push_back(std::move(mesh));
                }
            }
        }

        for(size_t i = 0; i < ai_node->mNumChildren; i++) 
        {
            ModelNode childNode;
            if (processNode(ai_node->mChildren[i], ai_scene, childNode, currTransform)) 
            {
                outNode.children.push_back(std::move(childNode));
            }
        }
        return true;
    }


    // aiMesh是Mesh对象, Mesh中包含顶点位置数据、法向量、纹理数据，每个Mesh可以包含一个或者多个Face
    bool ModelLoader::processMesh(const aiMesh *ai_mesh, const aiScene * ai_scene, ModelMesh &outMesh) 
    {
        std::vector<Vertex> vertexes;
        std::vector<int> indices;

        for (size_t i = 0; i < ai_mesh->mNumVertices; i++) 
        {
            Vertex vertex{};

            if (scene_.model->bAnimated) 
            {
                Utils::SetVertexBoneDataToDefault(vertex);
            }

            if (ai_mesh->HasPositions()) 
            {
                vertex.a_position.x = ai_mesh->mVertices[i].x;
                vertex.a_position.y = ai_mesh->mVertices[i].y;
                vertex.a_position.z = ai_mesh->mVertices[i].z;
            }
            if (ai_mesh->HasTextureCoords(0)) 
            {
                vertex.a_texCoord.x = ai_mesh->mTextureCoords[0][i].x;
                vertex.a_texCoord.y = ai_mesh->mTextureCoords[0][i].y;
            } else 
            {
                vertex.a_texCoord = glm::vec2(0.0f, 0.0f);
            }
            if (ai_mesh->HasNormals()) 
            {
                vertex.a_normal.x = ai_mesh->mNormals[i].x;
                vertex.a_normal.y = ai_mesh->mNormals[i].y;
                vertex.a_normal.z = ai_mesh->mNormals[i].z;
            }
            if (ai_mesh->HasTangentsAndBitangents())
            {
                vertex.a_tangent.x = ai_mesh->mTangents[i].x;
                vertex.a_tangent.y = ai_mesh->mTangents[i].y;
                vertex.a_tangent.z = ai_mesh->mTangents[i].z;
            }
            vertexes.push_back(vertex);
        }

        // 现在遍历网格的每个面(面是网格的三角形)并检索相应的顶点索引
        for (size_t i = 0; i < ai_mesh->mNumFaces; i++) 
        {
            aiFace face = ai_mesh->mFaces[i];
            if (face.mNumIndices != 3) 
            {
                std::cout << "ModelLoader::processMesh, mesh not transformed to triangle mesh." << std::endl;
            }
            for (size_t j = 0; j < face.mNumIndices; ++j)  // 每个面都有顶点索引
            { 
                indices.push_back((int) (face.mIndices[j])); // 将索引存储在容器中，类似于VEO
            }
        }

        // Bones
        if (scene_.model->bAnimated) 
        {
            for (size_t boneIndex = 0; boneIndex < ai_mesh->mNumBones; ++boneIndex) 
            {
                int boneID = -1;
                std::string boneName = ai_mesh->mBones[boneIndex]->mName.C_Str();
                if (scene_.model->mBoneInfoMap.find(boneName) == scene_.model->mBoneInfoMap.end()) 
                {
                    BoneInfo newBoneInfo;
                    newBoneInfo.id = scene_.model->mBoneCounter;
                    newBoneInfo.offset = convertMatrix(ai_mesh->mBones[boneIndex]->mOffsetMatrix);
                    scene_.model->mBoneInfoMap[boneName] = newBoneInfo;
                    boneID = scene_.model->mBoneCounter;
                    scene_.model->mBoneCounter++;
                } 
                else 
                {
                    boneID = scene_.model->mBoneInfoMap[boneName].id;
                }

                if (boneID == -1) 
                {
                    break;
                }
                
                auto weights = ai_mesh->mBones[boneIndex]->mWeights;
                int numWeights = ai_mesh->mBones[boneIndex]->mNumWeights;
                for (int weightIndex = 0; weightIndex < numWeights; ++weightIndex) 
                {
                    int vertexId = weights[weightIndex].mVertexId;
                    float weight = weights[weightIndex].mWeight;
                    if (vertexId > vertexes.size()) 
                    {
                        break;
                    }
                    for (int i = 0; i < MAX_BONES_PER_VERTEX; ++i) 
                    {
                        if (vertexes[vertexId].a_boneIDs[i] < 0) 
                        {
                            vertexes[vertexId].a_boneWeights[i] = weight;
                            vertexes[vertexId].a_boneIDs[i] = boneID;
                            break;
                        }
                    }
                }
            }
        }

        outMesh.material = std::make_shared<Material>();
        outMesh.material->baseColor = glm::vec4(1.f);
        if (ai_mesh->mMaterialIndex >= 0) 
        {
            // 处理材质
            const aiMaterial *material = ai_scene->mMaterials[ai_mesh->mMaterialIndex];
            // alpha mode
            outMesh.material->alphaMode = Alpha_Opaque;
            aiString alphaMode;
            if (material->Get(AI_MATKEY_GLTF_ALPHAMODE, alphaMode) == aiReturn_SUCCESS)
            {
                // "MASK" not support
                if (aiString("BLEND") == alphaMode) 
                {
                    outMesh.material->alphaMode = Alpha_Blend;
                }
            }

            // double side
            outMesh.material->doubleSided = false;
            bool doubleSide;
            if (material->Get(AI_MATKEY_TWOSIDED, doubleSide) == aiReturn_SUCCESS) 
            {
                outMesh.material->doubleSided = doubleSide;
            }

            // shading mode
            outMesh.material->shadingModel = Shading_BlinnPhong;  // default
            aiShadingMode shading_mode;
            if (material->Get(AI_MATKEY_SHADING_MODEL, shading_mode) == aiReturn_SUCCESS) 
            {
                if (aiShadingMode_PBR_BRDF == shading_mode) 
                {
                    outMesh.material->shadingModel = Shading_PBR;
                }
            }

            for (int i = 0; i <= AI_TEXTURE_TYPE_MAX; i++) 
            {
                processMaterial(material, static_cast<aiTextureType>(i), *outMesh.material);
            }
        }

        outMesh.primitiveType = Primitive_TRIANGLE;
        outMesh.primitiveCnt = ai_mesh->mNumFaces;
        outMesh.vertexes = std::move(vertexes); // std::move作用主要可以将一个左值转换成右值引用，从而可以调用C++11右值引用的拷贝构造函数
        outMesh.indices = std::move(indices);
        outMesh.aabb = convertBoundingBox(ai_mesh->mAABB);
        outMesh.InitVertexes();

        return true;
    }


    void ModelLoader::processMaterial(const aiMaterial *ai_material, aiTextureType textureType, Material &material) 
    {
        if (ai_material->GetTextureCount(textureType) <= 0) 
        {
            return; 
        }

        for (size_t i = 0; i < ai_material->GetTextureCount(textureType); i++) // 检查储存在材质中(该类型)纹理的数量
        { 
            aiTextureMapMode texMapMode[2];  // [u, v]
            aiString texPath;
            aiReturn retStatus = ai_material->GetTexture(textureType, i, &texPath, nullptr, nullptr, nullptr, nullptr, &texMapMode[0]);

            if (retStatus != aiReturn_SUCCESS || texPath.length == 0) {
                std::cout << "load texture type=%d, index=%d failed with return value=%d" <<  textureType <<  i <<retStatus << std::endl;
                continue;
            }
            std::string absolutePath = scene_.model->resourcePath + "/" + texPath.C_Str();
            MaterialTexType texType = MaterialTexType_NONE;
            // 处理不同的纹理类型
            switch (textureType) 
            {
                case aiTextureType_BASE_COLOR:
                case aiTextureType_DIFFUSE: // diffuse maps
                    texType = MaterialTexType_ALBEDO;
                    break;
                case aiTextureType_NORMALS: // normal maps
                    texType = MaterialTexType_NORMAL;
                    break;
                case aiTextureType_EMISSIVE:
                    texType = MaterialTexType_EMISSIVE;
                    break;
                case aiTextureType_LIGHTMAP:
                    texType = MaterialTexType_AMBIENT_OCCLUSION;
                    break;
                case aiTextureType_UNKNOWN:
                    texType = MaterialTexType_METAL_ROUGHNESS;
                    break;
                default:
                    continue;  // not support
            }
            auto buffer = loadTextureFile(absolutePath);
            if (buffer) 
            {
                auto &texData = material.textureData[texType];
                texData.tag = absolutePath;
                texData.width = buffer->getWidth();
                texData.height = buffer->getHeight();
                texData.data = {buffer};
                texData.wrapModeU = convertTexWrapMode(texMapMode[0]);
                texData.wrapModeV = convertTexWrapMode(texMapMode[1]);
            } 
            else 
            {
                // std::cout << "load texture failed: %s, path: %s" << Material::materialTexTypeStr(texType) << absolutePath.c_str() << std::endl;
                std::cout << "load texture failed: %s, path: %s" << absolutePath.c_str() << std::endl;
            }
        }
    }


    glm::mat4 ModelLoader::convertMatrix(const aiMatrix4x4 &m) 
    {
        glm::mat4 ret;
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                ret[j][i] = m[i][j];
            }
        }
        return ret;
    }


    BoundingBox ModelLoader::convertBoundingBox(const aiAABB &aabb) 
    {
        BoundingBox ret{};
        ret.min = glm::vec3(aabb.mMin.x, aabb.mMin.y, aabb.mMin.z);
        ret.max = glm::vec3(aabb.mMax.x, aabb.mMax.y, aabb.mMax.z);
        return ret;
    }


    WrapMode ModelLoader::convertTexWrapMode(const aiTextureMapMode &mode) 
    {
        WrapMode retWrapMode;
        switch (mode) 
        {
            case aiTextureMapMode_Wrap:
                retWrapMode = Wrap_REPEAT;
                break;
            case aiTextureMapMode_Clamp:
                retWrapMode = Wrap_CLAMP_TO_EDGE;
                break;
            case aiTextureMapMode_Mirror:
                retWrapMode = Wrap_MIRRORED_REPEAT;
                break;
            default:
                retWrapMode = Wrap_REPEAT;
                break;
        }

        return retWrapMode;
    }


    glm::mat4 ModelLoader::adjustModelCenter(BoundingBox &bounds) 
    {
        glm::mat4 modelTransform(1.0f);
        glm::vec3 trans = (bounds.max + bounds.min) / -2.f;
        trans.y = -bounds.min.y;
        float bounds_len = glm::length(bounds.max - bounds.min);
        modelTransform = glm::scale(modelTransform, glm::vec3(3.f / bounds_len));
        modelTransform = glm::translate(modelTransform, trans);
        return modelTransform;
    }


    void ModelLoader::preloadTextureFiles(const aiScene *scene, const std::string &resDir) 
    {
        std::set<std::string> texPaths;
        for (int materialIdx = 0; materialIdx < scene->mNumMaterials; materialIdx++) 
        {
            aiMaterial *material = scene->mMaterials[materialIdx];
            for (int texType = aiTextureType_NONE; texType <= AI_TEXTURE_TYPE_MAX; texType++) 
            {
                auto textureType = static_cast<aiTextureType>(texType);
                size_t texCnt = material->GetTextureCount(textureType);
                for (size_t i = 0; i < texCnt; i++) 
                {
                    aiString textPath;
                    aiReturn retStatus = material->GetTexture(textureType, i, &textPath);
                    if (retStatus != aiReturn_SUCCESS || textPath.length == 0) {
                        continue;
                    }
                    texPaths.insert(resDir + "/" + textPath.C_Str());
                }
            }
        }
        if (texPaths.empty()) 
        {
            return;
        }

        ThreadPool pool(std::min(texPaths.size(), (size_t) std::thread::hardware_concurrency()));
        for (auto &path : texPaths) 
        {
            pool.pushTask([&](int thread_id)
            {
                loadTextureFile(path);
            });
        }
    }


    std::shared_ptr<Buffer<RGBA>> ModelLoader::loadTextureFile(const std::string &path) 
    {
        texCacheMutex_.lock();
        if (textureDataCache_.find(path) != textureDataCache_.end()) 
        {
            auto &buffer = textureDataCache_[path];
            texCacheMutex_.unlock();
            return buffer;
        }
        texCacheMutex_.unlock();

        std::cout << "load texture, path: %s" << path.c_str() << std::endl;

        auto buffer = ImageUtils::readImageRGBA(path);
        if (buffer == nullptr) 
        {
            std::cout << "load texture failed, path: %s" << path.c_str() << std::endl;
            return nullptr;
        }

        texCacheMutex_.lock();
        textureDataCache_[path] = buffer;
        texCacheMutex_.unlock();

        return buffer;
    }

}

