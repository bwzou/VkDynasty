#define _CRT_SECURE_NO_WARNINGS

#pragma once

#include "../../code/base/macro.h"
#include "../../imgui/ImGuiLayer.h"
#include "../../DynastyEngine.h"

namespace DynastyEngine 
{
    class VulkanImGuiLayer : public ImGuiLayer
	{
	public:
		VulkanImGuiLayer() = default;
        // VulkanImGuiLayer(GLFWwindow* window) : ImGuiLayer(window) {}
		VulkanImGuiLayer(const std::string& name) : ImGuiLayer(name) {}
		virtual ~VulkanImGuiLayer();

		virtual void begin() override;
		virtual void end() override;

		virtual void onAttach() override;
		virtual void onDetach() override;

	private:
		float m_Time = 0.0f;

	};
}
