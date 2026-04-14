#ifndef _APPLICATION
#define _APPLICATION

#include "EngineGL.h"
#include <glfw3.h>
#include <glm/glm.hpp>
#include <chrono>
//Primitive Vectoriel
#include "VectorPrimitive.h"
//End Primitive Vectoriel

class Application {
public:
    Application(int width = 1024, int height = 1024, std::string name = "My OpenGL Engine");

    ~Application();

    void mainLoop();

    // Display Imgui Overlay
    void displayOverlay(bool display);

    
    // Draw & Animation
    void animate(const float elapsedTime);
    
    // KeyBoard Event
    static void keyboard_callback_glfw(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void mouse_callback_glfw(GLFWwindow* window, int button, int action, int mods);
    static void mouse_wheel_callback_glfw(GLFWwindow* window, double xoffset, double yoffset);
    static void mousepos_callback_glfw(GLFWwindow* win, double mouseX, double mouseY);
    static void window_resize_callback_glfw(GLFWwindow* win, int width, int height);
    
    void mousepos_event(int x, int y);
    void keyboard_event(int key, int scancode, int action, int mods);
    void mouse_event(int button, int action, int mods);
    void mouse_wheel_event(double yoffset);
    void windowSize_event(int width, int height);
    
    private:
    void updateTime();
    
    
    glm::vec3 projectOnSphere(glm::vec2 pos);
    
    glm::vec2 getNormalizedMouseCoord(glm::vec2 m);
    
    void trackballFrame();
    void translateFrameTrackball(glm::vec2 v, glm::vec2 o);
    void rotateFrameTrackBall(glm::vec2 v, glm::vec2 o);
    void scaleFrameTrackball(glm::vec2 v, glm::vec2 o);
    
    GLFWwindow* m_window;
    int m_height, m_width;
    std::string m_title;
    
    ImVec4 m_clear_color;
    bool m_show_overlay;
    bool m_display_interface;
    
    // SampleEngine
    EngineGL* m_engine;
    Scene* m_scene;
    bool m_anim_object;

    // Camera Attributes
    bool m_trackball, m_firstperson, m_translate; // Mode
    bool mode_on, m_rotate, m_zoom;               // Trackball operations
    bool m_scaling = false;
    float m_speed_factor, m_camera_speed, m_wheel;

    float angleYaw, anglePitch;
    glm::vec3 camPos, camDir;
    glm::vec2 oMouse, nMouse;
    glm::vec2 middle;

    std::chrono::steady_clock::time_point m_startTime;
    std::chrono::steady_clock::time_point m_lastTime;
    float m_secondsSinceStart;
    float m_elapsedSeconds;
    float m_elapsedMillis;

    //Exportation d'image
    bool m_isRecording = false;
    int m_frameCounter = 0;
    std::string m_exportFolder = "Enregistrements/";
    void saveCurrentFrame();
    //End Exportation d'image
    //Primitive Vectoriel
    bool m_creationMode = false;
    VectorPrimitive::Type m_currentPrimType = VectorPrimitive::Type::RECTANGLE;
    VectorPrimitive* m_currentCreatingPrimitive = nullptr;
    glm::vec2 m_creationStartPos = glm::vec2(0.0f);
    glm::vec2 screenToWorld(glm::vec2 screenPos);
    //End Primitive Vectoriel
    //Outils de dessin
    glm::vec4 m_currentFillColor   = glm::vec4(0.8f, 0.1f, 0.95f, 0.9f);   // Default purple fill
    glm::vec4 m_currentStrokeColor = glm::vec4(1.0f, 0.0f, 1.0f, 1.0f);    // Magenta stroke
    float     m_currentThickness   = 5.0f;                                 // Default thickness
    glm::vec4 m_backgroundColor    = glm::vec4(0.1f, 0.1f, 0.15f, 1.0f);   // Dark background
    //End Outils de dessin 
        
    //Importation d'image
        void importImage();
        
    //End Importation d'image
    //Transformations interactives
    Node* m_selectedNode = nullptr;
    Node* m_lastImportedImage = nullptr;
    enum class TransformMode { None, Translate, Rotate, Scale };
    TransformMode m_transformMode = TransformMode::None;
    // End Transformation interactives
};

#endif