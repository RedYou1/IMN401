
#include <chrono>
#include <cmath>
#include <glad/glad.h>
#include <iostream>
#include <glm/gtc/constants.hpp>

#include "Application.h"
#include "Frame.h"
#include "imgui_impl_glfw_gl3.h"

//Exportation d'image
#include <filesystem>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb/stb_image_write.h>
//End Exportation d'image
//Primitive Vectoriel
#include "Scene.h"
//End Primitive Vectoriel
//Importation d'image
#include <windows.h>     // For file dialog (Windows)
#include <commdlg.h>     // For OPENFILENAME
#ifdef _WIN32
    #define GLFW_EXPOSE_NATIVE_WIN32
    #include <glfw3native.h>
    #include <windows.h>
    #include <commdlg.h>
#endif
#include <stb_image.h>
#include "Materials/BaseMaterial/BaseMaterial.h"   
#include "Texture2D.h"
#include "Plane.h"
//End Importation d'image

bool goForward = false;
bool goBackward = false;
bool goLeft = false;
bool goRight = false;

namespace {
    bool isRaytracerSceneActive(const EngineGL* engine) {
        return engine && engine->getRenderScene() == EngineGL::RenderScene::Raytracer;
    }
}

static void error_callback(int error, const char* description) {
    LOG_TRACE << "-------------------------------------------------------------" << std::endl;
    LOG_TRACE << "Error :" << error << "\nDescription: " << description << std::endl;
}

Application::Application(int width, int height, std::string name)
    : m_width(width), m_height(height), m_title(name) {
    srand((unsigned int)time(0));

    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        exit(-1);
    }

    glfwSetErrorCallback(error_callback);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    m_window = glfwCreateWindow(m_width, m_height, m_title.c_str(), NULL, NULL);

    glfwMakeContextCurrent(m_window);
    glfwSetWindowUserPointer(m_window, this);

    // glfwSwapInterval(0);

    // Load all OpenGL functions using the glfw loader function
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize OpenGL context" << std::endl;
        glfwTerminate();
        exit(-1);
    }

    try {
        ImGui_ImplGlfwGL3_Init(m_window, false);

        glfwSetMouseButtonCallback(m_window, mouse_callback_glfw);
        glfwSetScrollCallback(m_window, mouse_wheel_callback_glfw);
        glfwSetKeyCallback(m_window, keyboard_callback_glfw);
        glfwSetCharCallback(m_window, ImGui_ImplGlfwGL3_CharCallback);
        glfwSetCursorPosCallback(m_window, mousepos_callback_glfw);
        glfwSetFramebufferSizeCallback(m_window, window_resize_callback_glfw);

    }
    catch (const std::exception& e) { std::cerr << "Error GLFW Initalization: " << e.what() << std::endl; }

    try {
        // Direct engine
        m_engine = new EngineGL(m_width, m_height);

        m_scene = Scene::getInstance();

        m_engine->init();
    }
    catch (const std::exception& e) {
        LOG_ERROR << "Error Engine Initalization: " << e.what() << std::endl;
        Logger::getInstance()->show_interface = true;
    }

    m_clear_color = ImColor(120, 120, 120);
    m_display_interface = true;

    m_rotate = m_zoom = m_anim_object = false; // Trackball
    m_trackball = true;
    m_speed_factor = 0.025f;
    m_camera_speed = 1.f;
    m_wheel = 0.f;

    middle = glm::vec2(m_width / 2, m_height / 2);
    nMouse = middle;
    m_translate = false;

    m_startTime = std::chrono::steady_clock::now();
    m_lastTime = m_startTime;
    m_secondsSinceStart = 0.0f;
    m_elapsedSeconds = 0.0f;
    m_elapsedMillis = 0.0f;
}

Application::~Application() {
    ImGui_ImplGlfwGL3_Shutdown();
    glfwTerminate();
    delete m_engine;
    m_scene->kill();
}

void Application::displayOverlay(bool display) {

    ImGuiStyle& style = ImGui::GetStyle();
    style.ChildWindowRounding = 8.0f;
    style.WindowRounding = 8.0f;
    style.FrameRounding = 8.0f;
    style.WindowTitleAlign = ImVec2(0.5f, 0.5f);
    ImGui::SetNextWindowPos(ImVec2(10, 20));

    if (!ImGui::Begin("Stats", &display, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings)) {
        ImGui::End();
        return;
    }
    ImGui::Text("Mouse Position: (%.1f,%.1f)", ImGui::GetIO().MousePos.x, ImGui::GetIO().MousePos.y);
    ImGui::Separator();
    ImGui::Separator();

    static ImVec4 color = ImColor(0.5f, 0.5f, 0.5f, 1.0f);
    ImGui::Text("Clear Color :");
    ImGui::SameLine();
    if (ImGui::ColorButton(color))
        ImGui::OpenPopup("Clear Color");
    if (ImGui::BeginPopup("Clear Color")) {
        ImGui::ColorPicker4("", (float*)&color);
        m_engine->setClearColor(glm::vec4(color.x, color.y, color.z, color.w));
        ImGui::EndPopup();
    }
    // OUTILS DE DESSIN
    if (ImGui::Begin("Outils de Dessin Vectoriel"))
    {
        ImGui::Text("Outils de dessin");

        ImGui::Separator();

        ImGui::ColorEdit4("Couleur de remplissage", &m_currentFillColor[0]);
        ImGui::ColorEdit4("Couleur du contour", &m_currentStrokeColor[0]);
        ImGui::SliderFloat("Épaisseur du contour", &m_currentThickness, 1.0f, 30.0f);

        ImGui::Separator();

        if (ImGui::ColorEdit4("Couleur d'arrière-plan", &m_backgroundColor[0]))
        {
            m_engine->setClearColor(m_backgroundColor);
        }

        ImGui::Separator();

    }
    //End outils de dessin


    ImGui::Separator();
    // Transformation Interactives
    if (ImGui::Begin("Transformations Interactives"))
    {
        ImGui::Text("Mode de Transformation");

        if (ImGui::RadioButton("Aucun", m_transformMode == TransformMode::None)) {
            m_transformMode = TransformMode::None;
            m_selectedNode = nullptr;
        }
        ImGui::SameLine();
        if (ImGui::RadioButton("Translation", m_transformMode == TransformMode::Translate)) {
            m_transformMode = TransformMode::Translate;
        }
        ImGui::SameLine();
        if (ImGui::RadioButton("Rotation", m_transformMode == TransformMode::Rotate)) {
            m_transformMode = TransformMode::Rotate;
        }
        ImGui::SameLine();
        if (ImGui::RadioButton("Scale", m_transformMode == TransformMode::Scale)) {
            m_transformMode = TransformMode::Scale;
        }

        ImGui::Separator();

        Node* selected = m_scene->getManipulatedNode();

        ImGui::Text("Objet sélectionné : %s \n", 
        selected ? selected->getName().c_str() : "Aucun\n");

        ImGui::Separator();
        
        ImGui::SliderFloat(": Metalic", &selected->materialProperties.shininess, 0.f, 10.f, "%.3f", 2.f);

        ImGui::Separator();
    }
        //End Transformation Interactives
    ImGui::Text("Camera parameters:");

    ImGui::Checkbox(" : Manipulate Object Trackball", &m_trackball);

    ImGui::SliderFloat(": Speed Factor", &m_speed_factor, 0.001f, 0.050f);
    ImGui::SliderFloat(": Camera Speed", &m_camera_speed, 0.001f, 1.f);
    ImGui::Separator();
    ImGui::End();

    if (ImGui::BeginMainMenuBar()) {
// Importation d'image
        if (ImGui::BeginMenu("File")) {
                    if (ImGui::MenuItem("Importer Image...")) {
            importImage();
        }
        //End Importation d'image
        ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Console")) {
            ImGui::MenuItem("Show Console", NULL, &Logger::getInstance()->show_interface);
            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }

    m_scene->displayInterface();
    m_engine->displayInterface();

    if (Logger::getInstance()->show_interface)
        Logger::getInstance()->Draw("Console");
}

void Application::mousepos_callback_glfw(GLFWwindow* window, double mouseX, double mouseY) {
    Application* g = static_cast<Application*>(glfwGetWindowUserPointer(window));
    g->mousepos_event((int)mouseX, (int)mouseY);
}

void Application::keyboard_callback_glfw(GLFWwindow* window, int key, int scancode, int action, int mods) {
    Application* g = static_cast<Application*>(glfwGetWindowUserPointer(window));
    g->keyboard_event(key, scancode, action, mods);
}

void Application::mouse_callback_glfw(GLFWwindow* window, int button, int action, int mods) {
    Application* g = static_cast<Application*>(glfwGetWindowUserPointer(window));
    g->mouse_event(button, action, mods);
}

void Application::mouse_wheel_callback_glfw(GLFWwindow* window, double xoffset, double yoffset) {
    ImGui_ImplGlfwGL3_ScrollCallback(window, xoffset, yoffset);
    Application* g = static_cast<Application*>(glfwGetWindowUserPointer(window));
    g->mouse_wheel_event(yoffset);
}

void Application::window_resize_callback_glfw(GLFWwindow* window, int width, int height) {
    Application* g = static_cast<Application*>(glfwGetWindowUserPointer(window));

    if (width > 0 && height > 0)
        g->windowSize_event(width, height);
}

void Application::windowSize_event(int width, int height) {
    m_width = width;
    m_height = height;

    if (m_engine != NULL)
        m_engine->onWindowResize(width, height);
}

void Application::keyboard_event(int key, int scancode, int action, int mods) {

    ImGuiIO& io = ImGui::GetIO();
    if (action == GLFW_PRESS && key == GLFW_KEY_ESCAPE)
        glfwSetWindowShouldClose(m_window, GL_TRUE); // Quit Application

    if (action == GLFW_PRESS) {
        io.KeysDown[key] = true;
        switch (key) {
        case GLFW_KEY_TAB:
            m_display_interface = !m_display_interface;
            break;
        case GLFW_KEY_SPACE:
            m_trackball = !m_trackball;

            break;
            //Exportation d'image
        case GLFW_KEY_R:
            m_isRecording = !m_isRecording;
            if (m_isRecording) {
                m_frameCounter = 0;
                std::cout << "Enregistrement en cours - Appuyer sur R pour arreter\n";
            }
            else {
                std::cout << "Enregistrement arretter " << m_frameCounter << " frames exporter vers "
                    << m_exportFolder << std::endl;
            }
            break;
            //End Exportation d'image
        //Primitive Vectoriel
        case GLFW_KEY_G:  // Toggle creation mode
            m_creationMode = !m_creationMode;
            std::cout << "Vector Primitive Creation Mode: " << (m_creationMode ? "ENABLED" : "DISABLED") << std::endl;
            if (!m_creationMode && m_currentCreatingPrimitive) {
                m_currentCreatingPrimitive->finishCreation();
                m_currentCreatingPrimitive = nullptr;
            }
            break;

        case GLFW_KEY_1: m_currentPrimType = VectorPrimitive::Type::POINT;      std::cout << "Mode: Point\n"; break;
        case GLFW_KEY_2: m_currentPrimType = VectorPrimitive::Type::LINE;       std::cout << "Mode: Line\n"; break;
        case GLFW_KEY_3: m_currentPrimType = VectorPrimitive::Type::RECTANGLE;  std::cout << "Mode: Rectangle\n"; break;
        case GLFW_KEY_4: m_currentPrimType = VectorPrimitive::Type::TRIANGLE;   std::cout << "Mode: Triangle\n"; break;
        case GLFW_KEY_5: m_currentPrimType = VectorPrimitive::Type::CIRCLE;     std::cout << "Mode: Circle\n"; break;
            //End Primitive Vectoriel

        default: break;

        }
    }
    else if (action == GLFW_RELEASE) {
        io.KeysDown[key] = false;
    }
    if (!ImGui::GetIO().WantCaptureKeyboard) {
        if (action == GLFW_PRESS) {
            io.KeysDown[key] = true;
            switch (key) {
            case GLFW_KEY_W:
                goForward = true;
                break;
            case GLFW_KEY_S:
                goBackward = true;
                break;
            case GLFW_KEY_A:
                goLeft = true;
                break;
            case GLFW_KEY_D:
                goRight = true;
                break;

            default: break;
            }
        }
        else if (action == GLFW_RELEASE) {
            io.KeysDown[key] = false;
            switch (key) {
            case GLFW_KEY_W:
                goForward = false;
                break;
            case GLFW_KEY_S:
                goBackward = false;
                break;
            case GLFW_KEY_A:
                goLeft = false;
                break;
            case GLFW_KEY_D:
                goRight = false;
                break;

            default: break;
            }
        }
    }
}

void Application::mousepos_event(int x, int y)
{
    if (isRaytracerSceneActive(m_engine)) return;

    nMouse.x = (float)x;
    nMouse.y = (float)y;

    if (m_creationMode && m_currentCreatingPrimitive)
    {
        glm::vec2 worldPos = screenToWorld(nMouse);
        m_currentCreatingPrimitive->updateCreation(worldPos);
        return;
    }

    Node* selected = m_scene->getManipulatedNode();
    if (m_transformMode != TransformMode::None && selected)
    {
        if (m_rotate || m_translate || m_scaling)
        {
            trackballFrame();   // This will now use the selected node
        }
        return;
    }

    // Normal camera trackball
    if (m_trackball) {
        trackballFrame();
    }
}
void Application::mouse_event(int button, int action, int mods)
{
    if (isRaytracerSceneActive(m_engine)) {
        m_rotate = false;
        m_translate = false;
        return;
    }

    if (!ImGui::GetIO().WantCaptureMouse) {
        nMouse.x = ImGui::GetIO().MousePos.x;
        nMouse.y = ImGui::GetIO().MousePos.y;
        glm::vec2 worldPos = screenToWorld(nMouse);

        if (m_creationMode)
        {
            if (button == 0 && action == GLFW_PRESS)
            {
                m_currentCreatingPrimitive = new VectorPrimitive(m_currentPrimType, "Prim");
                if (m_currentCreatingPrimitive)
                {
                    m_currentCreatingPrimitive->setFillColor(m_currentFillColor);
                    m_currentCreatingPrimitive->setStrokeColor(m_currentStrokeColor);
                    m_currentCreatingPrimitive->setStrokeThickness(m_currentThickness);
                }

                m_currentCreatingPrimitive->startCreation(glm::vec2(0.0f));

                glm::vec3 centerPos(0.0f, 8.0f, 0.0f);
                m_currentCreatingPrimitive->frame()->translate(centerPos);

                m_scene->getSceneNode()->adopt(m_currentCreatingPrimitive);

                if (m_engine) m_engine->refreshNodeCollector();

                std::cout << "Created " << m_currentCreatingPrimitive->getName() << std::endl;

                if (m_currentPrimType != VectorPrimitive::Type::LINE)
                {
                    m_currentCreatingPrimitive->finishCreation();
                    m_currentCreatingPrimitive = nullptr;
                }
            }
            else if (button == 0 && action == GLFW_RELEASE)
            {
                if (m_currentCreatingPrimitive && m_currentPrimType == VectorPrimitive::Type::LINE)
                {
                    m_currentCreatingPrimitive->updateCreation(screenToWorld(nMouse));
                    m_currentCreatingPrimitive->finishCreation();
                    m_currentCreatingPrimitive = nullptr;
                }
            }
            return;   
        }
//Transformations interactives
if (m_transformMode != TransformMode::None) 
{
    if (button == 0 && action == GLFW_PRESS) 
    {
        if (m_selectedNode == nullptr) {
            m_selectedNode = m_scene->getManipulatedNode();
        }

        oMouse = nMouse;   

        m_rotate    = (m_transformMode == TransformMode::Rotate);
        m_translate = (m_transformMode == TransformMode::Translate);
        m_scaling   = (m_transformMode == TransformMode::Scale);

        std::cout << "Transforming node: "
                  << (m_selectedNode ? m_selectedNode->getName() : "None")
                  << std::endl;
    } 
    else if (button == 0 && action == GLFW_RELEASE) 
    {
        m_rotate = false;
        m_translate = false;
        m_scaling = false;
    }
    return;
}
    }

    //End Transformations interactives
    if (m_trackball) {
        oMouse = nMouse;
        if (action == GLFW_PRESS && button == 0) m_rotate = true;
        else if (action == GLFW_RELEASE && button == 0) m_rotate = false;

        if (action == GLFW_PRESS && button == 1) m_translate = true;
        else if (action == GLFW_RELEASE && button == 1) m_translate = false;
    }
}

void Application::mouse_wheel_event(double yoffset) {
    if (isRaytracerSceneActive(m_engine))
        return;

    if (!ImGui::GetIO().WantCaptureMouse) {
        if (m_trackball)
            m_wheel -= (float)yoffset;

        float modif = (yoffset) ? 1.0f : -1.0f;
        glm::vec3 dep = (float)(yoffset) * 0.5f * m_scene->camera()->frame()->convertDirTo(glm::vec3(0.0, 0.0, -1.0), m_scene->getManipulatedNode()->frame());
        m_scene->getManipulatedNode()->frame()->translate(dep);
    }
}
glm::vec2 Application::getNormalizedMouseCoord(glm::vec2 m) {
    glm::vec2 v;
    v.x = (float)m.x / (float)m_width;
    v.y = 1.0f - (float)m.y / (float)m_height;
    v = glm::vec2(2.0f) * v - glm::vec2(1.0f);
    return v;
}

glm::vec3 Application::projectOnSphere(glm::vec2 pos) {
    glm::vec3 v;
    v[0] = pos[0];
    v[1] = pos[1];

    if (v.x * v.x + v.y * v.y <= 0.5f)
        v.z = sqrt(1.0f - (v.x * v.x + v.y * v.y));
    else
        v.z = (0.5f) / sqrt(v.x * v.x + v.y * v.y);
    return v;
}

void Application::trackballFrame() 
{
    glm::vec2 v, o;
    nMouse.x = ImGui::GetIO().MousePos.x;
    nMouse.y = ImGui::GetIO().MousePos.y;

    v = getNormalizedMouseCoord(nMouse);
    o = getNormalizedMouseCoord(oMouse);

    if (glm::length(v - o) < 0.001f) {
        oMouse = nMouse;
        return;
    }
    //Transformations interactives
    if (m_rotate) {
        rotateFrameTrackBall(v, o);
    }
    else if (m_translate) {
        translateFrameTrackball(v, o);
    }
    else if (m_scaling) {
        scaleFrameTrackball(v, o);
    }
    //End Transformations interactives
    oMouse = nMouse;   
}
//Transformations interactives
void Application::translateFrameTrackball(glm::vec2 v, glm::vec2 o)
{
    if (!m_selectedNode) return;

    glm::vec2 dis = v - o;

    Node* selected = m_scene->getManipulatedNode();
    if (!selected) return;

    Frame* f = selected->frame();

    
    glm::vec3 right = m_scene->camera()->frame()->convertDirTo(glm::vec3(1.0f, 0.0f, 0.0f), f); 
    glm::vec3 up    = m_scene->camera()->frame()->convertDirTo(glm::vec3(0.0f, 1.0f, 0.0f), f);

    
    glm::vec3 movement = right * dis.x * 8.0f + up * dis.y * 8.0f;

    f->translate(movement);
}

void Application::rotateFrameTrackBall(glm::vec2 v, glm::vec2 o)
{
    if (!m_selectedNode) return;

    glm::vec3 v1 = projectOnSphere(v);
    glm::vec3 v2 = projectOnSphere(o);

    Node* selected = m_scene->getManipulatedNode();
    if (!selected) return;

    Frame* f = selected->frame();
    v1 = glm::normalize(v1);
    v2 = glm::normalize(v2);

    glm::vec3 rax = glm::normalize(glm::cross(v2, v1));
    if (!std::isnan(rax.x)) {
        float angle = acos(glm::dot(v1, v2));
        rax = glm::normalize(m_scene->camera()->frame()->convertDirTo(rax, f));
        f->rotate(rax, (float)(glm::pi<float>() * angle));
    }
}

void Application::scaleFrameTrackball(glm::vec2 v, glm::vec2 o)
{
    if (!m_selectedNode) return;

    glm::vec2 dis = v - o;

    float scaleFactor = std::exp(-dis.y * 2.0f); // smooth & stable

    Node* selected = m_scene->getManipulatedNode();
    if (!selected) return;

    Frame* f = selected->frame();
    f->scale(glm::vec3(scaleFactor));
}
//End Transformations interactives
void Application::animate(const float elapsedTime) {
    m_engine->animate(elapsedTime);

    if (goForward)
        m_scene->camera()->frame()->translate(glm::vec3(0.0, 0.0, -m_camera_speed));
    else if (goBackward)
        m_scene->camera()->frame()->translate(glm::vec3(0.0, 0.0, m_camera_speed));
    if (goRight)
        m_scene->camera()->frame()->translate(glm::vec3(m_camera_speed, 0.0, 0.0));
    else if (goLeft)
        m_scene->camera()->frame()->translate(glm::vec3(-m_camera_speed, 0.0, 0.0));
}

void Application::updateTime() {
    const auto now = std::chrono::steady_clock::now();
    m_elapsedSeconds = std::chrono::duration<float>(now - m_lastTime).count();
    m_elapsedMillis = std::chrono::duration<float, std::milli>(now - m_lastTime).count();
    m_secondsSinceStart = std::chrono::duration<float>(now - m_startTime).count();

    constexpr float maxSeconds = 3600.0f;
    if (m_secondsSinceStart > maxSeconds) {
        m_startTime = now;
        m_lastTime = now;
        m_secondsSinceStart = 0.0f;
        m_elapsedSeconds = 0.0f;
        m_elapsedMillis = 0.0f;
        return;
    }

    m_lastTime = now;
}

//Exportation d'image
void Application::saveCurrentFrame()
{
    int width = m_width;
    int height = m_height;

    std::vector<unsigned char> pixels(width * height * 4);

    glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());

    stbi_flip_vertically_on_write(true);

    static bool folderCreated = false;
    if (!folderCreated) {
        if (!std::filesystem::exists(m_exportFolder))
            std::filesystem::create_directories(m_exportFolder);
        folderCreated = true;
    }

    char filename[256];
    snprintf(filename, sizeof(filename), "%sframe_%04d.png", m_exportFolder.c_str(), m_frameCounter++);

    if (stbi_write_png(filename, width, height, 4, pixels.data(), width * 4)) {
        std::cout << "Exported: " << filename << std::endl;
    }
    else {
        std::cerr << "Failed to save " << filename << std::endl;
    }
}
//End Exportation d'image

    //Primitive Vectoriel
glm::vec2 Application::screenToWorld(glm::vec2 screenPos)
{
    float aspect = (float)m_width / (float)m_height;
    glm::vec2 ndc = glm::vec2(
        (screenPos.x / m_width) * 2.0f - 1.0f,
        1.0f - (screenPos.y / m_height) * 2.0f
    );

    return ndc * 100.0f + glm::vec2(0.0f, 5.0f); 
}
//End primitive 
//Importation d'image
void Application::importImage()
{
#ifdef _WIN32
    char filename[MAX_PATH] = {0};

    OPENFILENAMEA ofn = {0};
    ofn.lStructSize = sizeof(OPENFILENAMEA);
    ofn.hwndOwner = glfwGetWin32Window(m_window);
    ofn.lpstrFilter = "Images (*.png;*.jpg;*.jpeg;*.bmp;*.tga)\0*.png;*.jpg;*.jpeg;*.bmp;*.tga\0All Files\0*.*\0\0";
    ofn.lpstrFile = filename;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR;

    if (GetOpenFileNameA(&ofn))
    {
        int width, height, channels;
        unsigned char* data = stbi_load(filename, &width, &height, &channels, 4);

        if (data)
        {
            std::cout << "Image loaded: " << filename << " (" << width << "x" << height << ")" << std::endl;

            std::filesystem::path path(filename);

            std::string nodeName = path.stem().string();
            int counter = 1;
            std::string baseName = nodeName;

            while (m_scene->findNode(nodeName) != nullptr) {
                nodeName = baseName + "_" + std::to_string(counter++);
            }

            Node* imageNode = m_scene->getNode(nodeName);

            
            Texture2D* texture = new Texture2D(width, height, GL_RGBA8);
            glTextureSubImage2D(texture->getId(), 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, data);

            BaseMaterial* material = new BaseMaterial("ImageMaterial");
            material->setTexture(texture);

            imageNode->setMaterial(material);
            imageNode->setModel(m_scene->getModel<ModelGL>(ObjPath + "Quad.obj"));

            float aspect = static_cast<float>(width) / height;
            imageNode->frame()->scale(glm::vec3(aspect * 25.0f, 25.0f, 1.0f));
            imageNode->frame()->translate(glm::vec3(0.0f, 8.0f, 0.0f));  
            m_scene->getSceneNode()->adopt(imageNode);

            m_lastImportedImage = imageNode;

            if (m_engine) {
                m_engine->refreshNodeCollector();
            }

            stbi_image_free(data);
            std::cout << "Image imported successfully!" << std::endl;
        }
        else
        {
            std::cerr << "Failed to load image: " << filename << std::endl;
        }
    }
    else
    {
        std::cout << "Image import cancelled." << std::endl;
    }
#else
    std::cout << "Image import is currently only supported on Windows." << std::endl;
#endif
}
//End Importation d'image
void Application::mainLoop() {
    LOG_INFO << "Beginning Main Loop" << std::endl;

    while (!glfwWindowShouldClose(m_window)) {
        updateTime();

        // Evenment handling and interface rendering
        glfwPollEvents();
        ImGui_ImplGlfwGL3_NewFrame();
        displayOverlay(m_display_interface);

        // Rendering
        m_engine->render();

        // update
        animate(m_secondsSinceStart);

        if (m_display_interface)
            ImGui::Render();
        //Exportation d'image
        if (m_isRecording) {
            saveCurrentFrame();
        }
        //End Exportation d'image
        glfwSwapBuffers(m_window);
    }
    LOG_INFO << "Ending Main Loop" << std::endl;
}
