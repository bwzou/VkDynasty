// #pragma once

// #include <unordered_map>
// #include <mutex>
// #include <assimp/scene.h>

// class ModerLoader {
// public:
//     // explict ModelLoader(Config &config)
//     explicit ModelLoader(Config &config);

//     bool loadModel(const std::string &filepath);
//     bool loadSkybox(const std::string &filepath);

//     inline DemoScene &getScene() { return scene_; }

//     inline size_t getModelPrimitiveCnt() const {
//         if (scene_.model) {
//         return scene_.model->primitiveCnt;
//         }
//         return 0;
//     }

//     inline void resetAllModelStates() {
//         for (auto &kv : modelCache_) {
//             kv.second->resetStates();
//         }

//         for (auto &kv : skyboxMaterialCache_) {
//             kv.second->resetStates();
//         }
//     }

//     static void loadCubeMesh(ModelVertexes &mesh);


// };