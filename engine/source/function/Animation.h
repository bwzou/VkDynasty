#pragma once

#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/GltfMaterial.h>
#include <vector>
#include "./Animdata.h"
#include "./Bone.h"
#include "Model.h"
#include "../code/base/GLMInc.h"


// 每个网格的最大骨骼数量
// 在皮肤着色器中不能高于相同的常量
#define MAX_BONES 100
// 每个顶点关联的最大骨骼
#define MAX_BONES_PER_VERTEX 4

namespace Utils
{
	static glm::mat4 ConvertMatrixToGLMFormat(const aiMatrix4x4& from)
	{
		glm::mat4 to;
		//the a,b,c,d in assimp is the row ; the 1,2,3,4 is the column
		to[0][0] = from.a1; to[1][0] = from.a2; to[2][0] = from.a3; to[3][0] = from.a4;
		to[0][1] = from.b1; to[1][1] = from.b2; to[2][1] = from.b3; to[3][1] = from.b4;
		to[0][2] = from.c1; to[1][2] = from.c2; to[2][2] = from.c3; to[3][2] = from.c4;
		to[0][3] = from.d1; to[1][3] = from.d2; to[2][3] = from.d3; to[3][3] = from.d4;
		return to;
	}
};


struct AssimpNodeData {
	glm::mat4 transformation;
	std::string name;
	int childrenCount;
	std::vector<AssimpNodeData> children;
};


class Animation	{
public:
	Animation() = default;
	Animation(const Animation&) = default;
	Animation(const aiScene* scene) {
		// Assimp::Importer importer;
				
		// from https://github.com/assimp/assimp/issues/2544
		//unsigned int ppsteps = aiProcess_LimitBoneWeights | aiProcess_Triangulate | aiProcess_GenSmoothNormals;
		//aiPropertyStore* props = aiCreatePropertyStore();
		//aiSetImportPropertyInteger(props, AI_CONFIG_PP_SBP_REMOVE, aiPrimitiveType_LINE | aiPrimitiveType_POINT);
		//aiSetImportPropertyInteger(props, AI_CONFIG_IMPORT_FBX_PRESERVE_PIVOTS, 0);
		//aiSetImportPropertyInteger(props, AI_CONFIG_IMPORT_REMOVE_EMPTY_BONES, 0);
		////auto scene = (aiScene*)aiImportFileExWithProperties(assetPath, ppsteps, NULL, props);
		//const aiScene* scene = aiImportFileExWithProperties(animationPath.c_str(), ppsteps, NULL, props);
		//aiReleasePropertyStore(props);

		// static const uint32_t s_MeshImportFlags =
		// 		aiProcess_CalcTangentSpace |        // Create binormals/tangents just in case
		// 		aiProcess_Triangulate |             // Make sure we're triangles
		// 		aiProcess_SortByPType |             // Split meshes by primitive type
		// 		aiProcess_GenNormals |              // Make sure we have legit normals
		// 		aiProcess_GenUVCoords |             // Convert UVs if required 
		// 		//aiProcess_OptimizeMeshes |          // Batch draws where possible
		// 		aiProcess_ValidateDataStructure;    // Validation

		// const aiScene* scene = importer.ReadFile(animationPath, aiProcess_Triangulate | aiProcess_CalcTangentSpace | aiProcess_FlipUVs | aiProcess_GenBoundingBoxes);
		assert(scene && scene->mRootNode);
		animation = scene->mAnimations[0];
		mDuration = animation->mDuration;
		mTicksPerSecond = animation->mTicksPerSecond;
		aiMatrix4x4 tempGlobalTransformation = scene->mRootNode->mTransformation;
		tempGlobalTransformation = tempGlobalTransformation.Inverse();
		globalInverseTransformation = Utils::ConvertMatrixToGLMFormat(tempGlobalTransformation);
		ReadHeirarchyData(mRootNode, scene->mRootNode);
		// ReadMissingBones(animation, *model);
	}

	~Animation() {

	}

	Bone* FindBone(const std::string& name) {
		auto iter = std::find_if(mBones.begin(), mBones.end(),
			[&](const Bone& Bone) {
				return Bone.GetBoneName() == name;
			}
		);
		if (iter == mBones.end()) return nullptr;
		else return &(*iter);
	}

	inline float GetTicksPerSecond() { return mTicksPerSecond; }
	inline float GetDuration() { return mDuration; }
	inline const AssimpNodeData& GetRootNode() { return mRootNode; }
	inline const std::map<std::string, BoneInfo>& GetBoneIDMap() {
		return mBoneInfoMap;
	}

public:
	// important
	// void ReadMissingBones(const aiAnimation* animation, Model& model);
	void ReadHeirarchyData(AssimpNodeData& dest, const aiNode* src) {
		assert(src);

		dest.name = src->mName.data;
		std::cout << " dest.name " << dest.name << std::endl;
		dest.transformation = Utils::ConvertMatrixToGLMFormat(src->mTransformation);
		dest.childrenCount = src->mNumChildren;

		for (int i = 0; i < src->mNumChildren; i++) {
			AssimpNodeData newData;
			ReadHeirarchyData(newData, src->mChildren[i]);
			dest.children.push_back(newData);
		}
	}

	float mDuration;
	int mTicksPerSecond;
	aiAnimation* animation;
	std::vector<Bone> mBones;
	AssimpNodeData mRootNode;
	glm::mat4 globalInverseTransformation;
	std::map<std::string, BoneInfo> mBoneInfoMap;
};
