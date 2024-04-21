/*
 * SoftGLRender
 * @author 	: keith@robot9.me
 *
 */

#pragma once


#define GLM_FORCE_ALIGNED
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES 
#define GLM_FORCE_INLINE
#define GLM_FORCE_AVX2
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
// #include <glm/gtc/type_aligned.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>

using RGBA = glm::u8vec4;
