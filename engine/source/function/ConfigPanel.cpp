#include "ConfigPanel.h"
#include <imgui.h>
#include <imgui_internal.h>
#include <backends/imgui_impl_vulkan.h>
#include <backends/imgui_impl_glfw.h>
#include "json11.hpp"

struct ImguiDebugInfo {
        float speed;
        float look_x;
        float look_y;
        float look_z;

        float center_x;
        float center_y;
        float center_z;

        float up_x;
        float up_y;
        float up_z;

        float rotate_x;
        float rotate_y;
        float rotate_z;
        float radians;

        float z_far;
        float z_near;
};

void checkVkResult(VkResult err) {
    if (err == 0) {
        return;
    }
    spdlog::error("[vulkan] Error: VkResult = {}", static_cast<long>(err));
    if (err < 0) {
        abort();
    }
}

bool ConfigPanel::init(void *window, int width, int height) {
    frameWidth_ = width;
    frameHeight_ = height;

    // Setup ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    io.IniFilename = nullptr;
    io.Fonts->AddFontDefault();
    io.Fonts->Build();

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    ImGuiStyle *style = &ImGui::GetStyle();
    style->Alpha = 0.8f;

    // load config
    return loadConfig();
}


bool ConfigPanel::initImgui(void *window, std::shared_ptr<Renderer> renderer) {
    if (renderer) {
        initialize_ = true;
        renderer_ = renderer.get();
        VkAttachmentDescription colorAttachment{};
        colorAttachment.format = VK_FORMAT_B8G8R8A8_UNORM;
        colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        VkAttachmentReference colorReference{};
        colorReference.attachment = 0;
        colorReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpassDescription{};
        subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpassDescription.colorAttachmentCount = 1;
        subpassDescription.pColorAttachments = &colorReference;

        VkSubpassDependency supassDependency{};
        supassDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        supassDependency.dstSubpass = 0;
        supassDependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        supassDependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        supassDependency.srcAccessMask = 0;
        supassDependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

        VkRenderPassCreateInfo renderPassCreateInfo{};
        renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassCreateInfo.attachmentCount = 1;
        renderPassCreateInfo.pAttachments = &colorAttachment;
        renderPassCreateInfo.subpassCount = 1;
        renderPassCreateInfo.pSubpasses = &subpassDescription;
        renderPassCreateInfo.dependencyCount = 1;
        renderPassCreateInfo.pDependencies = &supassDependency;

        vkCreateRenderPass(renderer_->getVkCtx().device(), &renderPassCreateInfo, nullptr, &imguiRenderPass_);

        // IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        (void)io;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;   // Enable Gamepad Controls
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;      // Enable Docking
        io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;    // Enable Multi-Viewport / Platfor
        io.DisplayFramebufferScale = ImVec2(1.0, 1.0);
        io.FontGlobalScale = 1.0;
        io.ConfigViewportsNoAutoMerge = true;
        // Setup Dear ImGui style
        // ImGui::StyleColorsDark();
        // ImGui::StyleColorsLight();

        ImGui_ImplGlfw_InitForVulkan((GLFWwindow *) window, true);
        ImGui_ImplVulkan_InitInfo iniInfo = {};
        iniInfo.Instance                  = renderer_->getVkCtx().instance();
        iniInfo.PhysicalDevice            = renderer_->getVkCtx().physicalDevice();
        iniInfo.Device                    = renderer_->getVkCtx().device();
        iniInfo.QueueFamily               = renderer_->getVkCtx().findQueueFamilies(renderer_->getVkCtx().physicalDevice()).graphicsFamily.value();
        iniInfo.Queue                     = renderer_->getVkCtx().graphicsQueue();
        iniInfo.DescriptorPool            = renderer_->getShaderProgram()->getDescriptorPool();
        iniInfo.Subpass                   = 0;
            
        // may be different from the real swapchain image count
        // see ImGui_ImplVulkanH_GetMinImageCountFromPresentMode
        iniInfo.MinImageCount = 3;
        iniInfo.ImageCount    = 3;
        iniInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
        iniInfo.Allocator = VK_NULL_HANDLE;
        iniInfo.CheckVkResultFn = checkVkResult;
        ImGui_ImplVulkan_Init(&iniInfo, imguiRenderPass_);

        // 将字体上传到 GPU Uploading fonts to the GPU
        auto commandBuffer = renderer_->getVkCtx().beginCommands();
        VkCommandBuffer cmdBuffer = commandBuffer -> cmdBuffer;
        ImGui_ImplVulkan_CreateFontsTexture(cmdBuffer);
        renderer_->getVkCtx().endCommands(commandBuffer);
    }
}


void ConfigPanel::onDraw() {
    // // Start the ImGui frame
    // ImGui_ImplVulkan_NewFrame();
    // ImGui_ImplGlfw_NewFrame();
    // ImGui::NewFrame();
    // ImGuiIO& io = ImGui::GetIO();
    // (void)io;

    // // Settings window
    // ImGui::Begin("Settings", nullptr, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize);
    // drawSettings();
    // ImGui::SetWindowPos(ImVec2(frameWidth_ - ImGui::GetWindowWidth(), 0));
    // ImGui::End();

    // // auto commandBuffer = renderer_->getVkCtx().beginCommands();
    // // ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer->cmdBuffer);
    // // renderer_->getVkCtx().endCommands(commandBuffer);
    // ImGui::ShowDemoWindow();
    
    // ImGui::Render();

    ImguiDebugInfo debugInfo{};
    debugInfo.speed = 90.0F;
    debugInfo.look_x = 2.0f;
    debugInfo.look_y = 2.0f;
    debugInfo.look_z = 2.0F;
    debugInfo.up_z = 1.f;
    debugInfo.rotate_z = 2.0;
    debugInfo.radians = 45.f;
    debugInfo.z_far = .1f;
    debugInfo.z_near = 10.f;

    auto* mainviewport = ImGui::GetMainViewport();
    mainviewport->Flags |= ImGuiViewportFlags_NoRendererClear;
    ImGuiIO& io = ImGui::GetIO();
    (void)io;
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    static bool show_demo_window = false;
    static bool show_another_window = false;
    const ImGuiViewport* main_viewport = ImGui::GetMainViewport();
    // ImGui::SetNextWindowPos(ImVec2(main_viewport->WorkPos.x + 661, main_viewport->WorkPos.y),
    //                         ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowPos(ImVec2(main_viewport->WorkPos), ImGuiCond_FirstUseEver);  
    {
        if (show_demo_window) {
            ImGui::ShowDemoWindow(&show_demo_window);
        }
        {
            static int counter = 0;
            ImGui::Begin("debug window");  // Create a window called "Hello, world!" and append into it.

            ImGui::Text("This is some useful text.");           // Display some text (you can use a format strings too)
            ImGui::Checkbox("Demo Window", &show_demo_window);  // Edit bools storing our window open/close state
            ImGui::Checkbox("Another Window", &show_another_window);
            float cenetr_x = debugInfo.look_x + 0.3f;
            float cenetr_y = debugInfo.look_y + 0.3f;
            float cenetr_z = debugInfo.look_z + 0.3f;
            ImGui::SliderFloat("speed", &debugInfo.speed, .0f, 180.0f);
            ImGui::SliderFloat("look at x", &debugInfo.look_x, .0f, 8.f);
            ImGui::SliderFloat("look at y", &debugInfo.look_y, .0f, 8.f);
            ImGui::SliderFloat("look at z", &debugInfo.look_z, .0f, 8.f);

            ImGui::SliderFloat("center x", &debugInfo.center_x, .0f, cenetr_x);
            ImGui::SliderFloat("center y", &debugInfo.center_y, .0f, cenetr_y);
            ImGui::SliderFloat("center z", &debugInfo.center_z, .0f, cenetr_z);

            ImGui::SliderFloat("up x", &debugInfo.up_x, .0f, 2.f);
            ImGui::SliderFloat("up y", &debugInfo.up_y, .0f, 2.f);
            ImGui::SliderFloat("up z", &debugInfo.up_z, .0f, 2.f);

            ImGui::SliderFloat("rotate x", &debugInfo.rotate_x, .0f, 10.f);
            ImGui::SliderFloat("rotate y", &debugInfo.rotate_y, .0f, 10.f);
            ImGui::SliderFloat("rotate z", &debugInfo.rotate_z, .1f, 10.f);

            ImGui::SliderFloat("radians z", &debugInfo.radians, 10.f, 180.f);

            ImGui::SliderFloat("z_near", &debugInfo.z_near, .1f, 10.f);
            ImGui::SliderFloat("z_far", &debugInfo.z_far, .1f, 10.f);

            // world axis
            ImGui::Separator();
            ImGui::Checkbox("world axis", &config_.worldAxis);

            ImGui::Separator();
            ImGui::Checkbox("point light", &config_.showLight);
            if (config_.showLight) {
                ImGui::Text("light color");
                ImGui::ColorEdit3("light color", (float *) &config_.pointLightColor, ImGuiColorEditFlags_NoLabel);

                ImGui::Text("light position");
                // ImGui::SliderFloat("position x", &config_.pointLightPosition[0], -50.0f, 50.f);
                // ImGui::SliderFloat("position y", &config_.pointLightPosition[1], -50.0f, 50.f);
                // ImGui::SliderFloat("position z", &config_.pointLightPosition[2], -50.0f, 50.f);
                ImGui::SliderAngle("##light position", &lightPositionAngle_, 0, 360.f);
            }

            // shadow floor
            ImGui::Separator();
            ImGui::Checkbox("shadow floor", &config_.showFloor);
            config_.shadowMap = config_.showFloor;

            if (ImGui::Button(
                    "Button")) {  // Buttons return true when clicked (most widgets return true when edited/activated)
                counter++;
            }
            ImGui::SameLine();
            ImGui::Text("counter = %d", counter);

            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
            ImGui::End();
        }

        if (show_another_window) {
            ImGui::Begin("Another Window");  // Pass a pointer to our bool variable (the window will have a closing
                                             // button that will clear the bool when clicked)
            ImGui::Text("Hello from another window!");
            if (ImGui::Button("Close Me")) {
                show_another_window = false;
            }
            ImGui::End();
        }
    }
    ImGui::Render();

    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
    }
}


void ConfigPanel::drawSettings() {

    // clear Color
    ImGui::Separator();
    ImGui::Text("clear color");
    ImGui::ColorEdit4("clear color", (float *) &config_.clearColor, ImGuiColorEditFlags_NoLabel);

    // wireframe
    ImGui::Separator();
    ImGui::Checkbox("wireframe", &config_.wireframe);

    // world axis
    ImGui::Separator();
    ImGui::Checkbox("world axis", &config_.worldAxis);

    // shadow floor
    ImGui::Separator();
    ImGui::Checkbox("shadow floor", &config_.showFloor);
    config_.shadowMap = config_.showFloor;

}


void ConfigPanel::update() {
    config_.pointLightPosition = 2.f * glm::vec3(glm::sin(lightPositionAngle_),
                                               1.2f,
                                               glm::cos(lightPositionAngle_));
    if (updateLightFunc_) {
        updateLightFunc_(config_.pointLightPosition, config_.pointLightColor);
    }        
    
}
    

void ConfigPanel::updateSize(int width, int height) {
    frameWidth_ = width;
    frameHeight_ = height;
}


bool ConfigPanel::wantCaptureKeyboard() {
  ImGuiIO &io = ImGui::GetIO();
  return io.WantCaptureKeyboard;
}


bool ConfigPanel::wantCaptureMouse() {
  ImGuiIO &io = ImGui::GetIO();
  return io.WantCaptureMouse;
}


bool ConfigPanel::loadConfig() {
    auto configPath = ASSETS_DIR + "assets.json";
    auto configStr = FileUtils::readText(configPath);
    
    if (configStr.empty()) {
        throw std::runtime_error("configPath is error!");
        return false;
    }

    std::string err;
    const auto json = json11::Json::parse(configStr, err);
    for (auto &kv : json["model"].object_items()) {
        modelPaths_[kv.first] = ASSETS_DIR + kv.second["path"].string_value();
    }
    for (auto &kv :  json["skybox"].object_items()) {
      skyboxPaths_[kv.first] = ASSETS_DIR + kv.second["path"].string_value();
    }
    
    if (modelPaths_.empty()) { 
       std::cout << "load models failed: %s" << err.c_str() << std::endl;
       return false;
    }

    for (const auto &kv : modelPaths_) {
        modelNames_.emplace_back(kv.first.c_str());
    }
    for (const auto &kv : skyboxPaths_) {
        skyboxNames_.emplace_back(kv.first.c_str());
    }
    return reloadModel(modelPaths_.begin()->first);
}


bool ConfigPanel::reloadModel(const std::string &name) {
    if (name != config_.modelName) {
        config_.modelName = name;
        config_.modelPath = modelPaths_[config_.modelName];

        if (reloadModelFunc_) {
            return reloadModelFunc_(config_.modelPath);
        }
    }
    return true;
}


bool ConfigPanel::reloadSkybox(const std::string &name) {

}


void ConfigPanel::destroy() {
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}





