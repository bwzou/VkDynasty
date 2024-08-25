#include <imgui.h>
#include <imgui_internal.h>
#include <backends/imgui_impl_vulkan.h>
#include <backends/imgui_impl_glfw.h>
#include "json11.hpp"

#include "editor/EditorUI.h"
#include "runtime/global/GlobalContext.h"
#include "runtime/platform/WindowSystem.h"
#include "runtime/code/base/macro.h"
#include "runtime/framework/level/Level.h"
#include "runtime/framework/serializer/HeSerializer.h"

#include "runtime/render/RenderSystem.h"
#include "runtime/platform/WindowSystem.h"

namespace DynastyEngine
{
    static bool bShowViewport = true;
    static bool bShowEngineSettings = true;
	static bool bShowSceneSettings = true;
    static bool bShowContentBrowser = true;
	static bool bShowSceneHierachy = true;
    static bool bShowProperties = true;
    static bool bShowStats = true;

    // Help
	static bool bShowTutorial = false;
	static bool bShowAboutMe = false;
	static bool bShowDemoImGui = true;


    void checkVkResult(VkResult err) 
    {
        if (err == 0) 
        {
            return;
        }
        spdlog::error("[vulkan] Error: VkResult = {}", static_cast<long>(err));
        if (err < 0) 
        {
            abort();
        }
    }

    
    void EditorUI::initialize(WindowUIInitInfo initInfo) 
    {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();

        ImGuiIO& io = ImGui::GetIO();
        // io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
        // io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;   // Enable Gamepad Controls
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking
        // io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;    // Enable Multi-Viewport / Platfor
        io.DisplayFramebufferScale = ImVec2(1.0, 1.0);
        io.FontGlobalScale = 1.0;
        io.ConfigViewportsNoAutoMerge = true;
        io.ConfigDockingAlwaysTabBar         = true;
        io.ConfigWindowsMoveFromTitleBarOnly = true;
        
        float fontSize = 18.0f;
        io.Fonts->AddFontFromFileTTF("/Users/bowenzou/Workspace/Documents/dynasty/engine/asset/fonts/opensans/OpenSans-Bold.ttf", fontSize);
        io.FontDefault = io.Fonts->AddFontFromFileTTF("/Users/bowenzou/Workspace/Documents/dynasty/engine/asset/fonts/opensans/OpenSans-Regular.ttf", fontSize);
        io.Fonts->Build();

        ImGuiStyle& style     = ImGui::GetStyle();
        style.WindowPadding   = ImVec2(1.0, 0);
        style.FramePadding    = ImVec2(14.0, 2.0f);
        style.ChildBorderSize = 0.0f;
        style.FrameRounding   = 5.0f;
        style.FrameBorderSize = 1.5f;

        // Setup Dear ImGui style
        ImGui::StyleColorsDark();

        initInfo.renderSystem->initializeUIRenderBackend(this);
    }

    void EditorUI::showEditorUI()
    {

    }


    void EditorUI::init() {
        // TODO 应该放到别处
        // load config
        // loadConfig();
    }   


    void EditorUI::initializeImgui(std::shared_ptr<Renderer> renderer) 
    {   
        LOG_INFO("initImgui");
        if (renderer) 
        {   
            pInitializeImgui = true;
            mRenderer = renderer.get();
            
            IMGUI_CHECKVERSION();
            ImGui::CreateContext();
            ImGuiIO& io = ImGui::GetIO();
            io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
            // io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;   // Enable Gamepad Controls
            io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;      // Enable Docking
            io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;    // Enable Multi-Viewport / Platfor
            io.DisplayFramebufferScale = ImVec2(1.0, 1.0);
            io.FontGlobalScale = 1.0;
            io.ConfigViewportsNoAutoMerge = true;

            float fontSize = 18.0f;
            io.Fonts->AddFontFromFileTTF("/Users/bowenzou/Workspace/Projects/dynasty/engine/asset/fonts/opensans/OpenSans-Bold.ttf", fontSize);
            io.FontDefault = io.Fonts->AddFontFromFileTTF("/Users/bowenzou/Workspace/Projects/dynasty/engine/asset/fonts/opensans/OpenSans-Regular.ttf", fontSize);

            // Setup Dear ImGui style
            ImGui::StyleColorsDark();
            //ImGui::StyleColorsClassic();

            // When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
            ImGuiStyle& style = ImGui::GetStyle();
            if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
            {
                style.WindowRounding = 0.0f;
                style.Colors[ImGuiCol_WindowBg].w = 1.0f;
            }

            std::shared_ptr<WindowSystem> windowSystem = gRuntimeGlobalContext.mWindowSystem;
            GLFWwindow* window = windowSystem->getWindow();
            ImGui_ImplGlfw_InitForVulkan((GLFWwindow *) window, true);
            ImGui_ImplVulkan_InitInfo iniInfo = {};
            iniInfo.Instance                  = mRenderer->getVkCtx().instance();
            iniInfo.PhysicalDevice            = mRenderer->getVkCtx().physicalDevice();
            iniInfo.Device                    = mRenderer->getVkCtx().device();
            iniInfo.QueueFamily               = mRenderer->getVkCtx().findQueueFamilies(mRenderer->getVkCtx().physicalDevice()).graphicsFamily.value();
            iniInfo.Queue                     = mRenderer->getVkCtx().graphicsQueue();
            iniInfo.DescriptorPool            = mRenderer->getShaderProgram()->getDescriptorPool();
            iniInfo.Subpass                   = 0;
            // may be different from the real swapchain image count
            // see ImGui_ImplVulkanH_GetMinImageCountFromPresentMode
            iniInfo.MinImageCount = 3;
            iniInfo.ImageCount    = 3;
            // iniInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
            // iniInfo.Allocator = VK_NULL_HANDLE;
            // iniInfo.CheckVkResultFn = checkVkResult;
            ImGui_ImplVulkan_Init(&iniInfo, mRenderer->getFbo()->getRenderPass());

            // 将字体上传到 GPU Uploading fonts to the GPU
            auto commandBuffer = mRenderer->getVkCtx().beginCommands();
            VkCommandBuffer cmdBuffer = commandBuffer -> cmdBuffer;
            ImGui_ImplVulkan_CreateFontsTexture(cmdBuffer);
            mRenderer->getVkCtx().endCommands(commandBuffer);
            
            vkDeviceWaitIdle(mRenderer->getVkCtx().device());
            ImGui_ImplVulkan_DestroyFontUploadObjects();
        }
    }


    void EditorUI::uploadFonts() 
    {

    }


    void EditorUI::renderUI() 
    {
        LOG_INFO("----- renderUI -----");
        static bool bChangeDim = false;

        // ImGui_ImplVulkan_NewFrame();
        // ImGui_ImplGlfw_NewFrame();
        // ImGui::NewFrame();
        
        // ----DockSpace Begin----
        static bool dockspaceOpen = true;
        static bool opt_fullscreen = true;
        static bool opt_padding = false;
        static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

        ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
        // ImGui::Begin("DockSpace Demo", &dockspaceOpen, window_flags);
        // if (bShowDemoImGui)
		// {
		// 	ImGui::ShowDemoWindow(&bShowDemoImGui);
		// }
        // LOG_INFO("----- onDraw 7 -----");

        // ImGui::End(); 
        // return;

        // We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
        // because it would be confusing to have two docking targets within each others.
        window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
        if (opt_fullscreen)
        {
            LOG_INFO("----- onDraw 1 -----");
            const ImGuiViewport* viewport = ImGui::GetMainViewport();
            ImGui::SetNextWindowPos(viewport->WorkPos);
            ImGui::SetNextWindowSize(ImVec2(1000, 20));
            ImGui::SetNextWindowViewport(viewport->ID);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
            window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
            window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
        }
        else
        {
            dockspace_flags &= ~ImGuiDockNodeFlags_PassthruCentralNode;
        }

        // When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background
        // and handle the pass-thru hole, so we ask Begin() to not render a background.
        if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
            window_flags |= ImGuiWindowFlags_NoBackground;

        // Important: note that we proceed even if Begin() returns false (aka window is collapsed).
        // This is because we want to keep our DockSpace() active. If a DockSpace() is inactive,
        // all active windows docked into it will lose their parent and become undocked.
        // We cannot preserve the docking relationship between an active window and an inactive docking, otherwise
        // any change of dockspace/settings would lead to windows being stuck in limbo and never being visible.
        if (!opt_padding)
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::Begin("DockSpace Demo", &dockspaceOpen, window_flags);
        if (!opt_padding)
            ImGui::PopStyleVar();

        if (opt_fullscreen)
            ImGui::PopStyleVar(2);
        
        // Submit the DockSpace
        ImGuiIO& io = ImGui::GetIO();
        ImGuiStyle& style = ImGui::GetStyle();
        float minWinSizeX = style.WindowMinSize.x;
        // LOG_INFO("----- WindowMinSize {} {} -----", style.WindowMinSize.x, style.WindowMinSize.y);
        style.WindowMinSize.x = 10.0f;
        if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
        {
            ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
            ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
        } 
        else 
        {
            ImGuiIO& io = ImGui::GetIO();
            ImGui::Text("ERROR: Docking is not enabled! See Demo > Configuration.");
            ImGui::Text("Set io.ConfigFlags |= ImGuiConfigFlags_DockingEnable in your code, or ");
            ImGui::SameLine(0.0f, 0.0f);
            if (ImGui::SmallButton("click here"))
                io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
        }
        style.WindowMinSize.x = minWinSizeX;

        LOG_INFO("----- onDraw 2 -----");
        // ----MenuBar Begin----
        if (ImGui::BeginMenuBar()) 
        {
            LOG_INFO("----- onDraw 3 -----");
            if (ImGui::BeginMenu("File"))
            {
                if (ImGui::MenuItem("New", "Ctrl+N"))
                    newScene();

                if (ImGui::MenuItem("Open...", "Ctrl+O"))
                    openScene();

                if (ImGui::MenuItem("Save As...", "Ctrl+Shift+S"))
                    saveSceneAs();

                // if (ImGui::MenuItem("Exit", NULL, false)) 
				// 	Application::GetInstance().Close();

                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Window")) 
            {
                LOG_INFO("----- onDraw 5 -----");
                ImGui::MenuItem("Viewport", NULL, &bShowViewport);
                ImGui::MenuItem("Content Browser", NULL, &bShowContentBrowser);
                ImGui::MenuItem("Level Hierachy", NULL, &bShowSceneHierachy);
                ImGui::MenuItem("Properties", NULL, &bShowProperties);
                ImGui::MenuItem("Stats", NULL, &bShowStats);
				ImGui::MenuItem("Engine Settings", NULL, &bShowEngineSettings);
				ImGui::MenuItem("Environment Settings", NULL, &bShowSceneSettings);

                if (ImGui::MenuItem("Load Default Layout"))
					loadDefaultEditorConfig();

				ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Help"))
			{
                LOG_INFO("----- onDraw 6 -----");
				ImGui::MenuItem("Tutorial", NULL, &bShowTutorial);
				ImGui::MenuItem("About Me", NULL, &bShowAboutMe);
				ImGui::MenuItem("Demo ImGui", NULL, &bShowDemoImGui);

				ImGui::EndMenu();
			}

            LOG_INFO("----- onDraw 7 -----");
            ImGui::EndMenuBar();
        }
        // ----MenuBar End----

        // ----Windows Begin----
		if (bShowContentBrowser)
		{
			mContentBrowserPanel.onImGuiDraw(&bShowContentBrowser);
		}
        if (bShowSceneHierachy || bShowProperties)
		{
			mSceneHierarchyPanel.onImGuiDraw(&bShowSceneHierachy, &bShowProperties);
		}
        LOG_INFO("----- onDraw 7 -----");
        if (bShowStats)
		{
			// ImGui::Begin("Stats", &bShowStats);

			// std::string name = "None";
			// if (mHoveredEntity)
			// 	name = mHoveredEntity.GetComponent<TagComponent>().Tag;
			// ImGui::Text("Hovered Entity: %s", name.c_str());

			// auto stats = Renderer2D::GetStats();
			// ImGui::Text("Renderer2D Stats:");
			// ImGui::Text("Draw Calls: %d", stats.DrawCalls);
			// ImGui::Text("Quads: %d", stats.QuadCount);
			// ImGui::Text("Vertices: %d", stats.GetTotalVertexCount());
			// ImGui::Text("Indices: %d", stats.GetTotalIndexCount());

			// ImGui::End();
		}
        if (bShowSceneSettings)
		{
			mSceneSettingsPanel.onImGuiDraw(&bShowSceneSettings);
		}
        LOG_INFO("----- onDraw 7 -----");
        if (bShowDemoImGui)
		{
			ImGui::ShowDemoWindow(&bShowDemoImGui);
		}
        LOG_INFO("----- onDraw 7 -----");
        // uiToolbar();

        ImGui::End(); 






        // ImGui::Render();
        // ImDrawData* main_draw_data = ImGui::GetDrawData();
        // ImGui_ImplVulkan_RenderDrawData(main_draw_data, mRenderer->getDrawCmd());

        // if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        // {
        //     ImGui::UpdatePlatformWindows();
        //     ImGui::RenderPlatformWindowsDefault();
        // }

        // ImGui::Begin("Another Window");  // Pass a pointer to our bool variable (the window will have a closing
        //                                         // button that will clear the bool when clicked)
        // ImGui::Text("Hello from another window!");
        // if (ImGui::Button("Close Me")) {
        //     // show_another_window = false;
        // }
        // ImGui::End();
        
        // io.DisplaySize = ImVec2((float)1000, (float)800);

        // ImGui::EndFrame();

        // LOG_INFO("----- onDraw end -1-----");

        // // ImGui::Render();

        // LOG_INFO("----- onDraw end -1-----");
        // if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        // {
        //     LOG_INFO("----- onDraw end -2-----");
        //     GLFWwindow* backup_current_context = glfwGetCurrentContext();
        //     ImGui::UpdatePlatformWindows();
        //     LOG_INFO("----- onDraw end -3-----");
        //     // ImGui::RenderPlatformWindowsDefault();
        //     LOG_INFO("----- onDraw end -4-----");
        //     glfwMakeContextCurrent(backup_current_context);
        // }



        // static bool dockspaceOpen = true;
        // static bool opt_fullscreen = true;
        // static bool opt_padding = false;
        // static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

        // // We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
        // // because it would be confusing to have two docking targets within each others.
        // ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
        // if (opt_fullscreen)
        // {
        //     const ImGuiViewport* viewport = ImGui::GetMainViewport();
        //     ImGui::SetNextWindowPos(viewport->WorkPos);
        //     ImGui::SetNextWindowSize(viewport->WorkSize);
        //     ImGui::SetNextWindowViewport(viewport->ID);
        //     ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        //     ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        //     window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
        //     window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
        // }
        // else
        // {
        //     dockspace_flags &= ~ImGuiDockNodeFlags_PassthruCentralNode;
        // }

        // // When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background
        // // and handle the pass-thru hole, so we ask Begin() to not render a background.
        // if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
        //     window_flags |= ImGuiWindowFlags_NoBackground;

        // // Important: note that we proceed even if Begin() returns false (aka window is collapsed).
        // // This is because we want to keep our DockSpace() active. If a DockSpace() is inactive,
        // // all active windows docked into it will lose their parent and become undocked.
        // // We cannot preserve the docking relationship between an active window and an inactive docking, otherwise
        // // any change of dockspace/settings would lead to windows being stuck in limbo and never being visible.
        // if (!opt_padding)
        //     ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        // ImGui::Begin("DockSpace Demo", &dockspaceOpen, window_flags);
        // if (!opt_padding)
        //     ImGui::PopStyleVar();

        // if (opt_fullscreen)
        //     ImGui::PopStyleVar(2);

        // // Submit the DockSpace
        // ImGuiIO& io = ImGui::GetIO();
        // // ImGuiStyle& style = ImGui::GetStyle();
        // // float minWinSizeX = style.WindowMinSize.x;
        // // style.WindowMinSize.x = 110.0f;
        // if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
        // {
        //     LOG_INFO("----- onDraw end 2-----");
        //     ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
        //     ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
        // }
        // else
        // {
        //     // ShowDockingDisabledMessage();
        // }
        // // style.WindowMinSize.x = minWinSizeX;

        // if (ImGui::BeginMenuBar())
        // {
        //     LOG_INFO("----- onDraw end 3 -----");
        //     if (ImGui::BeginMenu("Options"))
        //     {
        //         // Disabling fullscreen would allow the window to be moved to the front of other windows,
        //         // which we can't undo at the moment without finer window depth/z control.
        //         ImGui::MenuItem("Fullscreen", NULL, &opt_fullscreen);
        //         ImGui::MenuItem("Padding", NULL, &opt_padding);
        //         ImGui::Separator();

        //         if (ImGui::MenuItem("Flag: NoSplit",                "", (dockspace_flags & ImGuiDockNodeFlags_NoSplit) != 0))                 { dockspace_flags ^= ImGuiDockNodeFlags_NoSplit; }
        //         if (ImGui::MenuItem("Flag: NoResize",               "", (dockspace_flags & ImGuiDockNodeFlags_NoResize) != 0))                { dockspace_flags ^= ImGuiDockNodeFlags_NoResize; }
        //         if (ImGui::MenuItem("Flag: NoDockingInCentralNode", "", (dockspace_flags & ImGuiDockNodeFlags_NoDockingInCentralNode) != 0))  { dockspace_flags ^= ImGuiDockNodeFlags_NoDockingInCentralNode; }
        //         if (ImGui::MenuItem("Flag: AutoHideTabBar",         "", (dockspace_flags & ImGuiDockNodeFlags_AutoHideTabBar) != 0))          { dockspace_flags ^= ImGuiDockNodeFlags_AutoHideTabBar; }
        //         if (ImGui::MenuItem("Flag: PassthruCentralNode",    "", (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode) != 0, opt_fullscreen)) { dockspace_flags ^= ImGuiDockNodeFlags_PassthruCentralNode; }
        //         ImGui::Separator();

        //         // if (ImGui::MenuItem("Close", NULL, false, dockspaceOpen != NULL))
        //         //     dockspaceOpen = false;
        //         ImGui::EndMenu();
        //     }
        //     // HelpMarker(
        //     //     "When docking is enabled, you can ALWAYS dock MOST window into another! Try it now!" "\n"
        //     //     "- Drag from window title bar or their tab to dock/undock." "\n"
        //     //     "- Drag from window menu button (upper-left button) to undock an entire node (all windows)." "\n"
        //     //     "- Hold SHIFT to disable docking (if io.ConfigDockingWithShift == false, default)" "\n"
        //     //     "- Hold SHIFT to enable docking (if io.ConfigDockingWithShift == true)" "\n"
        //     //     "This demo app has nothing to do with enabling docking!" "\n\n"
        //     //     "This demo app only demonstrate the use of ImGui::DockSpace() which allows you to manually create a docking node _within_ another window." "\n\n"
        //     //     "Read comments in ShowExampleAppDockSpace() for more details.");

        //     ImGui::EndMenuBar();
        // }

        // if (bShowEngineSettings)
		// {
		// 	ImGui::Begin("Engine Settings", &bShowEngineSettings);

		// 	const char* modes[] = { "2D", "3D" };
		// 	int lastMode = 1;
		// 	ImGui::Columns(2, nullptr, false);
		// 	ImGui::SetColumnWidth(0, 100.0f);
		// 	ImGui::Text("Mode");
		// 	ImGui::NextColumn();
		// 	if (ImGui::Combo("##Mode", &lastMode, modes, IM_ARRAYSIZE(modes)))
		// 	{
		// 		if (lastMode != lastMode)
		// 		{
		// 			bChangeDim = true;
		// 		}
		// 	}
		// 	ImGui::EndColumns();

		// 	ImGui::Columns(2, nullptr, false);
		// 	ImGui::SetColumnWidth(0, 100.0f);
		// 	ImGui::Text("Camera Speed");
		// 	ImGui::NextColumn();
		// 	// ImGui::SliderFloat("##Camera Speed", &, 0.1f, 10.0f);
		// 	ImGui::EndColumns();
			
		// 	ImGui::End();
		// }

        // ImGui::Begin("Another Window", &dockspaceOpen);  // Pass a pointer to our bool variable (the window will have a closing
        //                                         // button that will clear the bool when clicked)
        // ImGui::Text("Hello from another window!");
        // if (ImGui::Button("Close Me")) {
        //     // show_another_window = false;
        // }
        // ImGui::End();

        // ImGui::End();

        // ImGui::EndFrame();

        // // ImGui::Render();
        // // ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), YOUR_COMMAND_BUFFER);

        // if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        // {
        // //     LOG_INFO("----- onDraw end -2-----");
        //     GLFWwindow* backup_current_context = glfwGetCurrentContext();
        //     ImGui::UpdatePlatformWindows();
        // //     LOG_INFO("----- onDraw end -3-----");
        //     ImGui::RenderPlatformWindowsDefault();
        // //     LOG_INFO("----- onDraw end -4-----");
        //     glfwMakeContextCurrent(backup_current_context);
        // }

        // LOG_INFO("----- onDraw end-----");


        // ImGui::ShowDemoWindow();
        // ImGui::EndFrame();
        // ImGui::Render();
        // // ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), YOUR_COMMAND_BUFFER);
        
        // ImGuiIO& io = ImGui::GetIO();
        // if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        // {
        //     GLFWwindow* backup_current_context = glfwGetCurrentContext();
        //     ImGui::UpdatePlatformWindows();
        //     ImGui::RenderPlatformWindowsDefault();
        //     glfwMakeContextCurrent(backup_current_context);
        // }
    }


    void EditorUI::uiToolbar()
	{
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 2));
		ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, ImVec2(0, 0));
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
		auto& colors = ImGui::GetStyle().Colors;
		const auto& buttonHovered = colors[ImGuiCol_ButtonHovered];
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(buttonHovered.x, buttonHovered.y, buttonHovered.z, 0.5f));
		const auto& buttonActive = colors[ImGuiCol_ButtonActive];
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(buttonActive.x, buttonActive.y, buttonActive.z, 0.5f));

		ImGui::Begin("##toolbar", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
		ImGuiDockNode* node = ImGui::GetWindowDockNode();
		node->LocalFlags |= ImGuiDockNodeFlags_NoTabBar;

		float size = ImGui::GetWindowHeight() - 4.0f;
		// Ref<Texture2D> icon = ModeManager::IsEditState() ? IconManager::GetInstance().GetPlayIcon() : IconManager::GetInstance().GetStopIcon();
		ImGui::SetCursorPosX((ImGui::GetWindowContentRegionMax().x * 0.5f) - (size * 0.5f));
		// if (ImGui::ImageButton((ImTextureID)icon->GetRendererID(), ImVec2(size, size), ImVec2(0, 0), ImVec2(1, 1), 0))
		// {
		// 	if (ModeManager::IsEditState())
		// 		OnScenePlay();
		// 	else
		// 		OnSceneStop();
		// }
		ImGui::PopStyleVar(2);
		ImGui::PopStyleColor(3);
		ImGui::End();
	}


    void EditorUI::newScene() 
    {
        mEditorScene = std::make_shared<Level>();

        mSceneHierarchyPanel.setContent(mEditorScene);
        mEditorScenePath = std::filesystem::path();
    }
    
    void EditorUI::openScene()
    {

    }

    void EditorUI::openScene(const std::filesystem::path& path) 
    {
        if (path.extension().string() != ".he" || path.extension().string() != ".json") 
        {
            LOG_WARN("Could not load {} - not a scene file. A scene file format must be .he or .json", path.filename().string());
            return;
        }

        std::shared_ptr<Level> newScene = std::make_shared<Level>();
        if (path.extension().string() == ".he") 
        {
            HeSerializer serializer(newScene);
            mEditorScene = newScene;

            
        }
        // TODO 加载方式可能会不一样
        else if (path.extension().string() == ".json") 
        {

        }

        // 设置显示内容
        mSceneHierarchyPanel.setContent(mEditorScene);

        mEditorScenePath = path;
    }

    void EditorUI::saveScene(const std::filesystem::path& format) 
    {

    }
        

    void EditorUI::saveSceneAs() 
    {   

    }


    void EditorUI::loadDefaultEditorConfig() {

    }


    


    // TODO 应该放到别处
    // bool EditorUI::loadConfig() 
    // {
    //     auto configPath = ASSETS_DIR + "assets.json";
    //     auto configStr = FileUtils::readText(configPath);

    //     LOG_INFO("loadConfig: {}", configStr);
        
    //     if (configStr.empty()) 
    //     {
    //         throw std::runtime_error("configPath is error!");
    //         return false;
    //     }

    //     std::string err;
    //     const auto json = json11::Json::parse(configStr, err);
    //     for (auto &kv : json["model"].object_items()) 
    //     {
    //         mModelPaths[kv.first] = ASSETS_DIR + kv.second["path"].string_value();
    //     }
    //     for (auto &kv :  json["skybox"].object_items()) 
    //     {
    //         mSkyboxPaths[kv.first] = ASSETS_DIR + kv.second["path"].string_value();
    //     }
        
    //     if (mModelPaths.empty()) 
    //     { 
    //         std::cout << "load models failed: %s" << err.c_str() << std::endl;
    //         return false;
    //     }

    //     for (const auto &kv : mModelPaths)
    //     {
    //         mModelNames.emplace_back(kv.first.c_str());
    //     }
    //     for (const auto &kv : mSkyboxPaths) 
    //     {
    //         mSkyboxNames.emplace_back(kv.first.c_str());
    //     }
    //     return reloadModel(mModelPaths.begin()->first);
    // }


    // TODO 应该放到别处
    // bool EditorUI::reloadModel(const std::string &name) 
    // {
    //     if (name != mConfig.modelName) 
    //     {
    //         mConfig.modelName = name;
    //         mConfig.modelPath = mModelPaths[mConfig.modelName];

    //         if (mReloadModelFunc) 
    //         {
    //             return mReloadModelFunc(mConfig.modelPath);
    //         }
    //     }
    //     return true;
    // }


    void EditorUI::cleanup() 
    {

    }
}

