#include <imgui.h>
#include <imgui_internal.h>
#include <backends/imgui_impl_vulkan.h>
#include <backends/imgui_impl_glfw.h>
#include "json11.hpp"

#include "editor/EditorUI.h"
#include "runtime/global/GlobalContext.h"
#include "runtime/code/base/macro.h"
#include "runtime/framework/level/Level.h"
#include "runtime/framework/serializer/HeSerializer.h"

#include "runtime/render/RenderSystem.h"
#include "runtime/platform/WindowSystem.h"
#include "runtime/platform/PlatformUtils.h"
#include "runtime/resource/asset/AssetManager.h"

#include "editor/global/GlobalContext.h"
#include "editor/managers/EditorInputManager.h"
#include "editor/managers/EditorSceneManager.h"

namespace DynastyEngine
{
    static bool pEditorMenu = true;
    static bool bShowViewport = true;
    static bool bShowEngineSettings = true;
	static bool bShowSceneSettings = true;
    static bool bShowContentBrowser = true;
	static bool bShowSceneHierachy = true;
    static bool bShowProperties = true;
    static bool bShowStats = false;

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
        // ImGui::StyleColorsDark();

        // set imgui color style
        setUIColorStyle();

        initInfo.renderSystem->initializeUIRenderBackend(this);
    }

    void EditorUI::showEditorUI()
    {
        showEditorMenu(&pEditorMenu);
        showEngineSettingsWindow(&bShowEngineSettings);
        showSceneSettingsWindow(&bShowSceneSettings);
        showEditorViewportWindow(&bShowViewport);
        showSceneHierarchyWindow(&bShowSceneHierachy, &bShowProperties);
        showContentBrowserWindow(&bShowContentBrowser);
    }

    void EditorUI::showEditorMenu(bool* pEditorMenu)
    {
        ImGuiDockNodeFlags dock_flags   = ImGuiDockNodeFlags_DockSpace;
        ImGuiWindowFlags   window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoTitleBar |
                                        ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
                                        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBackground |
                                        ImGuiConfigFlags_NoMouseCursorChange | ImGuiWindowFlags_NoBringToFrontOnFocus;

        const ImGuiViewport* main_viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(main_viewport->WorkPos, ImGuiCond_Always);
        std::array<int, 2> window_size = gRuntimeGlobalContext.mWindowSystem->getWindowSize();
        ImGui::SetNextWindowSize(ImVec2((float)window_size[0], (float)window_size[1]), ImGuiCond_Always);

        ImGui::SetNextWindowViewport(main_viewport->ID);

        ImGui::Begin("Editor menu", pEditorMenu, window_flags);

        ImGuiID main_docking_id = ImGui::GetID("Main Docking");
        if (ImGui::DockBuilderGetNode(main_docking_id) == nullptr)
        {
            ImGui::DockBuilderRemoveNode(main_docking_id);

            ImGui::DockBuilderAddNode(main_docking_id, dock_flags);
            ImGui::DockBuilderSetNodePos(main_docking_id,
                                         ImVec2(main_viewport->WorkPos.x, main_viewport->WorkPos.y + 18.0f));
            ImGui::DockBuilderSetNodeSize(main_docking_id,
                                          ImVec2((float)window_size[0], (float)window_size[1] - 18.0f));

            ImGuiID center = main_docking_id;
            ImGuiID left;
            ImGuiID right = ImGui::DockBuilderSplitNode(center, ImGuiDir_Right, 0.25f, nullptr, &left);

            ImGuiID left_other;
            ImGuiID left_file_content = ImGui::DockBuilderSplitNode(left, ImGuiDir_Down, 0.30f, nullptr, &left_other);

            ImGuiID left_game_engine;
            ImGuiID left_asset =
                ImGui::DockBuilderSplitNode(left_other, ImGuiDir_Left, 0.30f, nullptr, &left_game_engine);

            ImGui::DockBuilderDockWindow("World Objects", left_asset);
            ImGui::DockBuilderDockWindow("Components Details", right);
            ImGui::DockBuilderDockWindow("File Content", left_file_content);
            ImGui::DockBuilderDockWindow("Game Engine", left_game_engine);

            ImGui::DockBuilderFinish(main_docking_id);
        }

        ImGui::DockSpace(main_docking_id);

        // if (ImGui::BeginMenuBar())
        // {
        //     if (ImGui::BeginMenu("Menu"))
        //     {
        //         if (ImGui::MenuItem("Reload Current Level"))
        //         {
        //             // g_runtime_global_context.m_world_manager->reloadCurrentLevel();
        //             // g_runtime_global_context.m_render_system->clearForLevelReloading();
        //             // g_editor_global_context.m_scene_manager->onGObjectSelected(k_invalid_gobject_id);
        //         }
        //         if (ImGui::MenuItem("Save Current Level"))
        //         {
        //             // g_runtime_global_context.m_world_manager->saveCurrentLevel();
        //         }
        //         if (ImGui::BeginMenu("Debug"))
        //         {
        //             if (ImGui::BeginMenu("Animation"))
        //             {
        //                 // if (ImGui::MenuItem(g_runtime_global_context.m_render_debug_config->animation.show_skeleton ? "off skeleton" : "show skeleton"))
        //                 // {
        //                 //     g_runtime_global_context.m_render_debug_config->animation.show_skeleton = !g_runtime_global_context.m_render_debug_config->animation.show_skeleton;
        //                 // }
        //                 // if (ImGui::MenuItem(g_runtime_global_context.m_render_debug_config->animation.show_bone_name ? "off bone name" : "show bone name"))
        //                 // {
        //                 //     g_runtime_global_context.m_render_debug_config->animation.show_bone_name = !g_runtime_global_context.m_render_debug_config->animation.show_bone_name;
        //                 // }
        //                 ImGui::EndMenu();
        //             }
        //             if (ImGui::BeginMenu("Camera"))
        //             {
        //                 // if (ImGui::MenuItem(g_runtime_global_context.m_render_debug_config->camera.show_runtime_info ? "off runtime info" : "show runtime info"))
        //                 // {
        //                 //     g_runtime_global_context.m_render_debug_config->camera.show_runtime_info = !g_runtime_global_context.m_render_debug_config->camera.show_runtime_info;
        //                 // }
        //                 ImGui::EndMenu();
        //             }
        //             if (ImGui::BeginMenu("Game Object"))
        //             {
        //                 // if (ImGui::MenuItem(g_runtime_global_context.m_render_debug_config->gameObject.show_bounding_box ? "off bounding box" : "show bounding box"))
        //                 // {
        //                 //     g_runtime_global_context.m_render_debug_config->gameObject.show_bounding_box = !g_runtime_global_context.m_render_debug_config->gameObject.show_bounding_box;
        //                 // }
        //                 ImGui::EndMenu();
        //             }
        //             ImGui::EndMenu();
        //         }
        //         if (ImGui::MenuItem("Exit"))
        //         {
        //             // g_editor_global_context.m_engine_runtime->shutdownEngine();
        //             exit(0);
        //         }
        //         ImGui::EndMenu();
        //     }
        //     if (ImGui::BeginMenu("Window"))
        //     {
        //         // ImGui::MenuItem("World Objects", nullptr, &m_asset_window_open);
        //         // ImGui::MenuItem("Game", nullptr, &m_game_engine_window_open);
        //         // ImGui::MenuItem("File Content", nullptr, &m_file_content_window_open);
        //         // ImGui::MenuItem("Detail", nullptr, &m_detail_window_open);
        //         ImGui::EndMenu();
        //     }
        //     ImGui::EndMenuBar();
        // }

        if (ImGui::BeginMenuBar()) 
        {
            if (ImGui::BeginMenu("File"))
            {
                if (ImGui::MenuItem("New", "Ctrl+N"))
                    newScene();

                if (ImGui::MenuItem("Open...", "Ctrl+O"))
                    openScene();

                if (ImGui::MenuItem("Save As...", "Ctrl+Shift+S"))
                    saveSceneAs();

                if (ImGui::MenuItem("Exit", NULL, false)) 
					cleanup();

                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Window")) 
            {
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
				ImGui::MenuItem("Tutorial", NULL, &bShowTutorial);
				ImGui::MenuItem("About Me", NULL, &bShowAboutMe);
				ImGui::MenuItem("Demo ImGui", NULL, &bShowDemoImGui);

				ImGui::EndMenu();
			}

            ImGui::EndMenuBar();
        }

        ImGui::End();
    }

    void EditorUI::showContentBrowserWindow(bool* bShowContentBrowser) 
    {
        mContentBrowserPanel.onImGuiDraw(bShowContentBrowser);
    }

    void EditorUI::showEngineSettingsWindow(bool* bShowEngineSettings) 
    {
        ImGui::Begin("Engine Settings", bShowEngineSettings);

        ImGui::End();
    }

    void EditorUI::showSceneSettingsWindow(bool* bShowSceneSettings) 
    {
        mSceneSettingsPanel.onImGuiDraw(bShowSceneSettings);
    }

    void EditorUI::showSceneHierarchyWindow(bool* bShowSceneHierachy, bool* bShowProperties)
    {
        mSceneHierarchyPanel.onImGuiDraw(bShowSceneHierachy, bShowProperties);
    }

    void EditorUI::showEditorViewportWindow(bool* bShowViewport)
    {
        ImGuiIO&         io           = ImGui::GetIO();
        ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_MenuBar;

        const ImGuiViewport* main_viewport = ImGui::GetMainViewport();

        if (!*bShowViewport)
            return;

        if (!ImGui::Begin("Game Engine", bShowViewport, window_flags))
        {
            ImGui::End();
            return;
        }

        glm::vec2 render_target_window_pos = { 0.0f, 0.0f };
        glm::vec2  render_target_window_size = { 0.0f, 0.0f };

        auto menu_bar_rect = ImGui::GetCurrentWindow()->MenuBarRect();

        render_target_window_pos.x = ImGui::GetWindowPos().x;
        render_target_window_pos.y = menu_bar_rect.Max.y;
        render_target_window_size.x = ImGui::GetWindowSize().x;
        render_target_window_size.y = (ImGui::GetWindowSize().y + ImGui::GetWindowPos().y) - menu_bar_rect.Max.y; // coord of right bottom point of full window minus coord of right bottom point of menu bar window.

        // if (new_window_pos != m_engine_window_pos || new_window_size != m_engine_window_size)
        {
#if defined(__MACH__)
            // The dpi_scale is not reactive to DPI changes or monitor switching, it might be a bug from ImGui.
            // Return value from ImGui::GetMainViewport()->DpiScal is always the same as first frame.
            // glfwGetMonitorContentScale and glfwSetWindowContentScaleCallback are more adaptive.
            float dpi_scale = main_viewport->DpiScale;
            gRuntimeGlobalContext.mRenderSystem->updateEngineContentViewport(render_target_window_pos.x * dpi_scale,
                render_target_window_pos.y * dpi_scale,
                render_target_window_size.x * dpi_scale,
                render_target_window_size.y * dpi_scale);
#else
            gRuntimeGlobalContext.mRenderSystem->updateEngineContentViewport(
                render_target_window_pos.x, render_target_window_pos.y, render_target_window_size.x, render_target_window_size.y);
#endif
            gEditorGlobalContext.mInputManager->setEngineWindowPos(render_target_window_pos);
            gEditorGlobalContext.mInputManager->setEngineWindowSize(render_target_window_size);
        }

        ImGui::End();
    }

    void EditorUI::setUIColorStyle() 
    {
        ImGuiStyle* style  = &ImGui::GetStyle();
        ImVec4*     colors = style->Colors;

        colors[ImGuiCol_Text]                  = ImVec4(0.4745f, 0.4745f, 0.4745f, 1.00f);
        colors[ImGuiCol_TextDisabled]          = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
        colors[ImGuiCol_WindowBg]              = ImVec4(0.0078f, 0.0078f, 0.0078f, 1.00f);
        colors[ImGuiCol_ChildBg]               = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
        colors[ImGuiCol_PopupBg]               = ImVec4(0.08f, 0.08f, 0.08f, 0.94f);
        colors[ImGuiCol_Border]                = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
        colors[ImGuiCol_BorderShadow]          = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
        colors[ImGuiCol_FrameBg]               = ImVec4(0.047f, 0.047f, 0.047f, 0.5411f);
        colors[ImGuiCol_FrameBgHovered]        = ImVec4(0.196f, 0.196f, 0.196f, 0.40f);
        colors[ImGuiCol_FrameBgActive]         = ImVec4(0.294f, 0.294f, 0.294f, 0.67f);
        colors[ImGuiCol_TitleBg]               = ImVec4(0.0039f, 0.0039f, 0.0039f, 1.00f);
        colors[ImGuiCol_TitleBgActive]         = ImVec4(0.0039f, 0.0039f, 0.0039f, 1.00f);
        colors[ImGuiCol_TitleBgCollapsed]      = ImVec4(0.00f, 0.00f, 0.00f, 0.50f);
        colors[ImGuiCol_MenuBarBg]             = ImVec4(0.0f, 0.0f, 0.0f, 1.00f);
        colors[ImGuiCol_ScrollbarBg]           = ImVec4(0.02f, 0.02f, 0.02f, 0.53f);
        colors[ImGuiCol_ScrollbarGrab]         = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
        colors[ImGuiCol_ScrollbarGrabHovered]  = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
        colors[ImGuiCol_ScrollbarGrabActive]   = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
        colors[ImGuiCol_CheckMark]             = ImVec4(93.0f / 255.0f, 10.0f / 255.0f, 66.0f / 255.0f, 1.00f);
        colors[ImGuiCol_SliderGrab]            = colors[ImGuiCol_CheckMark];
        colors[ImGuiCol_SliderGrabActive]      = ImVec4(0.3647f, 0.0392f, 0.2588f, 0.50f);
        colors[ImGuiCol_Button]                = ImVec4(0.0117f, 0.0117f, 0.0117f, 1.00f);
        colors[ImGuiCol_ButtonHovered]         = ImVec4(0.0235f, 0.0235f, 0.0235f, 1.00f);
        colors[ImGuiCol_ButtonActive]          = ImVec4(0.0353f, 0.0196f, 0.0235f, 1.00f);
        colors[ImGuiCol_Header]                = ImVec4(0.1137f, 0.0235f, 0.0745f, 0.588f);
        colors[ImGuiCol_HeaderHovered]         = ImVec4(5.0f / 255.0f, 5.0f / 255.0f, 5.0f / 255.0f, 1.00f);
        colors[ImGuiCol_HeaderActive]          = ImVec4(0.0f, 0.0f, 0.0f, 1.00f);
        colors[ImGuiCol_Separator]             = ImVec4(0.0f, 0.0f, 0.0f, 0.50f);
        colors[ImGuiCol_SeparatorHovered]      = ImVec4(45.0f / 255.0f, 7.0f / 255.0f, 26.0f / 255.0f, 1.00f);
        colors[ImGuiCol_SeparatorActive]       = ImVec4(45.0f / 255.0f, 7.0f / 255.0f, 26.0f / 255.0f, 1.00f);
        colors[ImGuiCol_ResizeGrip]            = ImVec4(0.26f, 0.59f, 0.98f, 0.20f);
        colors[ImGuiCol_ResizeGripHovered]     = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
        colors[ImGuiCol_ResizeGripActive]      = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
        colors[ImGuiCol_Tab]                   = ImVec4(6.0f / 255.0f, 6.0f / 255.0f, 8.0f / 255.0f, 1.00f);
        colors[ImGuiCol_TabHovered]            = ImVec4(45.0f / 255.0f, 7.0f / 255.0f, 26.0f / 255.0f, 150.0f / 255.0f);
        colors[ImGuiCol_TabActive]             = ImVec4(47.0f / 255.0f, 6.0f / 255.0f, 29.0f / 255.0f, 1.0f);
        colors[ImGuiCol_TabUnfocused]          = ImVec4(45.0f / 255.0f, 7.0f / 255.0f, 26.0f / 255.0f, 25.0f / 255.0f);
        colors[ImGuiCol_TabUnfocusedActive]    = ImVec4(6.0f / 255.0f, 6.0f / 255.0f, 8.0f / 255.0f, 200.0f / 255.0f);
        colors[ImGuiCol_DockingPreview]        = ImVec4(47.0f / 255.0f, 6.0f / 255.0f, 29.0f / 255.0f, 0.7f);
        colors[ImGuiCol_DockingEmptyBg]        = ImVec4(0.20f, 0.20f, 0.20f, 0.00f);
        colors[ImGuiCol_PlotLines]             = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
        colors[ImGuiCol_PlotLinesHovered]      = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
        colors[ImGuiCol_PlotHistogram]         = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
        colors[ImGuiCol_PlotHistogramHovered]  = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
        colors[ImGuiCol_TableHeaderBg]         = ImVec4(0.0f, 0.0f, 0.0f, 1.00f);
        colors[ImGuiCol_TableBorderStrong]     = ImVec4(2.0f / 255.0f, 2.0f / 255.0f, 2.0f / 255.0f, 1.0f);
        colors[ImGuiCol_TableBorderLight]      = ImVec4(0.23f, 0.23f, 0.25f, 1.00f);
        colors[ImGuiCol_TableRowBg]            = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
        colors[ImGuiCol_TableRowBgAlt]         = ImVec4(1.00f, 1.00f, 1.00f, 0.06f);
        colors[ImGuiCol_TextSelectedBg]        = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
        colors[ImGuiCol_DragDropTarget]        = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
        colors[ImGuiCol_NavHighlight]          = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
        colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
        colors[ImGuiCol_NavWindowingDimBg]     = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
        colors[ImGuiCol_ModalWindowDimBg]      = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);
    }





    void EditorUI::init() {
        // TODO 应该放到别处
        // load config
        // loadConfig();
    }   

    void EditorUI::renderUI() 
    {
        showEditorUI();
        return;

        LOG_INFO("----- renderUI -----");
        static bool bChangeDim = false;
        
        // ----DockSpace Begin----
        static bool dockspaceOpen = true;
        static bool opt_fullscreen = true;
        static bool opt_padding = false;
        static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

        // We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
        // because it would be confusing to have two docking targets within each others.
        ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
        if (opt_fullscreen)
        {
            LOG_INFO("----- onDraw 1 -----");
            const ImGuiViewport* viewport = ImGui::GetMainViewport();
            LOG_INFO("View Size: {} {}", viewport->Size.x, viewport->Size.y);
            ImGui::SetNextWindowPos(viewport->WorkPos);
            ImGui::SetNextWindowSize(viewport->WorkSize);
            ImGui::SetNextWindowViewport(viewport->ID);
            // 设置圆角 
            ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
            // 设置圆角 
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
        style.WindowMinSize.x = 110.0f;
        if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
        {
            ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
            ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
        } 
        style.WindowMinSize.x = minWinSizeX;

        // ----MenuBar Begin----
        if (ImGui::BeginMenuBar()) 
        {
            if (ImGui::BeginMenu("File"))
            {
                if (ImGui::MenuItem("New", "Ctrl+N"))
                    newScene();

                if (ImGui::MenuItem("Open...", "Ctrl+O"))
                    openScene();

                if (ImGui::MenuItem("Save As...", "Ctrl+Shift+S"))
                    saveSceneAs();

                if (ImGui::MenuItem("Exit", NULL, false)) 
					cleanup();

                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Window")) 
            {
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
				ImGui::MenuItem("Tutorial", NULL, &bShowTutorial);
				ImGui::MenuItem("About Me", NULL, &bShowAboutMe);
				ImGui::MenuItem("Demo ImGui", NULL, &bShowDemoImGui);

				ImGui::EndMenu();
			}

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
        if (bShowEngineSettings)
        {
            ImGui::Begin("Engine Settings", &bShowEngineSettings);

            ImGui::End();
        }
        if (bShowViewport) {
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0, 0 });
			ImGui::Begin("Viewport");

            auto viewportMinRegion = ImGui::GetWindowContentRegionMin();
			auto viewportMaxRegion = ImGui::GetWindowContentRegionMax();
			auto viewportOffset = ImGui::GetWindowPos();
			mViewportBounds[0] = { viewportMinRegion.x + viewportOffset.x, viewportMinRegion.y + viewportOffset.y };
			mViewportBounds[1] = { viewportMaxRegion.x + viewportOffset.x, viewportMaxRegion.y + viewportOffset.y };

            mViewportFocused = ImGui::IsWindowFocused();
			mViewportHovered = ImGui::IsWindowHovered();

            if (ImGui::BeginDragDropTarget())
			{
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
				{
					const wchar_t* path = (const wchar_t*)payload->Data;
					openScene();
				}
				ImGui::EndDragDropTarget();
			}

            ImGui::End();
			ImGui::PopStyleVar();
        }
        if (bShowSceneSettings)
		{
			mSceneSettingsPanel.onImGuiDraw(&bShowSceneSettings);
		}
        // LOG_INFO("----- onDraw 7 -----");
        // if (bShowDemoImGui)
		// {
		// 	ImGui::ShowDemoWindow(&bShowDemoImGui);
		// }
        // LOG_INFO("----- onDraw 7 -----");
        // uiToolbar();

        ImGui::End(); 
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
        std::string filepath = FileDialogs::openFile("HEngine Level (*.he)\0*.he\0");
		if (!filepath.empty())
			openScene(filepath);
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
        if (!mEditorScenePath.empty())
        {
            // SerializeScene(mActiveScene, mEditorScenePath);
        }
        else
        {
            saveSceneAs();
        }
    }
        
    void EditorUI::saveSceneAs() 
    {   
        std::string filepath = FileDialogs::saveFile("HEngine Level (*.he)\0*.he\0");

    }

    void EditorUI::loadDefaultEditorConfig() 
    {
        const std::filesystem::path currentEditorConfigPath{ gRuntimeGlobalContext.mAssetManager->getFullPath("imgui.ini") };
		const std::filesystem::path defaultEditorConfigPath{ gRuntimeGlobalContext.mAssetManager->getFullPath("engine/config/imgui.ini") };
		
        // HE_CORE_ASSERT(std::filesystem::exists(DefaultEditorConfigPath));

		if (std::filesystem::exists(currentEditorConfigPath))
			std::filesystem::remove(currentEditorConfigPath);
		std::filesystem::copy(defaultEditorConfigPath, std::filesystem::current_path());

        // Window
		bShowViewport = true;
		bShowContentBrowser = true;
		bShowSceneHierachy = true;
		bShowProperties = true;
		bShowStats = false;
		bShowEngineSettings = true;
		bShowSceneSettings = true;
		// bShowSRT = true;

		// Help
		bShowTutorial = false;
		bShowAboutMe = false;
		bShowDemoImGui = false;
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

