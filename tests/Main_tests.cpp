#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <cctype>
#include <filesystem>
#include <stdexcept>
#include <vector>

#include <glad/glad.h>
#include <glfw3.h>
#include <imgui.h>

// files to test
#include "EngineGL.h"
#include "Sphere.h"
#include "Plane.h"
#include "MaterialGL.h"
#include "utils.hpp"

// forward declaration of floating functions in EngineGL.cpp
void calculerVecteurLumiere(const Light* light, const glm::vec3& intersectionPoint, glm::vec3& lightVec);
void calculerEchantillonsGrilleRayons(std::tuple<float, float> offset, float &NDCx, float &NDCy, float width, float height, bool vibration);
float genererLesEchantillonsSSAA(int samples, std::vector<std::tuple<float, float>>& offsets);
std::vector<unsigned char> appliquerFlouMoyen(const std::vector<unsigned char>& imageBuffer, int width, int height, int kernelSize);

// math functions to compare results
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp> // for glm::translate/rotate/scale (for expectations)
#include <glm/gtc/quaternion.hpp>



#define SHADOW_EPSILON 1e-5f
glm::vec3 calculerDeplacementDeCollision(const IntersectionData &intersection, const Ray &ray);

int scanFramebuffer(int width, int height, unsigned char bgR, unsigned char bgG, unsigned char bgB, unsigned char bgA) {
    std::vector<unsigned char> frameBuffer(width * height * 4);
    glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, frameBuffer.data());

    int foundCount = 0;

    std::cout << "Scanning framebuffer for non-background pixels...\n";
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            int index = (y * width + x) * 4;
            unsigned char r = frameBuffer[index];
            unsigned char g = frameBuffer[index+1];
            unsigned char b = frameBuffer[index+2];
            unsigned char a = frameBuffer[index+3];

            if (r != bgR || g != bgG || b != bgB || a != bgA) {
                std::cout << "Hit at OpenGL (X:" << x << ", Y:" << y << ") "
                          << "- RGBA: (" << (int)r << ", " << (int)g << ", " 
                          << (int)b << ", " << (int)a << ")\n";
                foundCount++;
            }
        }
    }
    std::cout << "Total non-background pixels found: " << foundCount << "\n";
    
    return foundCount;
}

#if defined(INF1_TESTS_USE_EGL)
#include <cstdlib>
struct HeadlessEnvSetup {
    HeadlessEnvSetup() {
        setenv("MESA_GL_VERSION_OVERRIDE", "4.6FC", 1);
        setenv("MESA_GLSL_VERSION_OVERRIDE", "460", 1);
        setenv("MESA_LOADER_DRIVER_OVERRIDE", "llvmpipe", 1);
        setenv("EGL_PLATFORM", "surfaceless", 1);
        setenv("LIBGL_ALWAYS_SOFTWARE", "1", 1);

        glfwInitHint(GLFW_PLATFORM, GLFW_PLATFORM_NULL);
    }
};
static HeadlessEnvSetup setup_env;
#endif

#define ROOT_IDENTIFIER ".root"

static void applyCommonOpenGLContextHints() {
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
}

static GLFWwindow* createHiddenTestWindowPreferEGL(int width, int height, const char* title) {
    applyCommonOpenGLContextHints();

#if defined(INF1_TESTS_USE_EGL)
    // Try EGL first (useful for headless/CI setups). If it fails, fall back to the default/native backend.
    glfwWindowHint(GLFW_CONTEXT_CREATION_API, GLFW_EGL_CONTEXT_API);
    GLFWwindow* window = glfwCreateWindow(width, height, title, nullptr, nullptr);
    if (window != nullptr) {
        return window;
    }

    applyCommonOpenGLContextHints();
    glfwWindowHint(GLFW_CONTEXT_CREATION_API, GLFW_NATIVE_CONTEXT_API);
    return glfwCreateWindow(width, height, title, nullptr, nullptr);
#else
    return glfwCreateWindow(width, height, title, nullptr, nullptr);
#endif
}

static void setWorkingDirectoryToProjectRoot() {
    std::filesystem::path exePath = std::filesystem::current_path();
    unsigned int maxDepth = 10;
    while (exePath.has_parent_path() && !std::filesystem::exists(exePath / ROOT_IDENTIFIER) && maxDepth > 0) {
        exePath = exePath.parent_path();
        maxDepth--;
    }

    if (!std::filesystem::exists(exePath / ROOT_IDENTIFIER)) {
        throw std::runtime_error("Project root not found. Are you running tests from the project folder?");
    }

    std::filesystem::current_path(exePath);
}

static bool nearFloat(float a, float b, float eps = 1e-5f) {
    return std::abs(a - b) <= eps;
}

static void checkNearVec3(const glm::vec3 &a, const glm::vec3 &b, float eps = 1e-5f) {
    CHECK(nearFloat(a.x, b.x, eps));
    CHECK(nearFloat(a.y, b.y, eps));
    CHECK(nearFloat(a.z, b.z, eps));
}

static glm::vec3 transformPoint(const glm::mat4 &m, const glm::vec3 &p) {
    glm::vec4 r = m * glm::vec4(p, 1.0f);
    return glm::vec3(r);
}

static glm::vec3 transformDir(const glm::mat4 &m, const glm::vec3 &d) {
    glm::vec4 r = m * glm::vec4(d, 0.0f);
    return glm::vec3(r);
}

static bool nearByte(unsigned char a, unsigned char b, unsigned char eps = 3) {
    int da = static_cast<int>(a);
    int db = static_cast<int>(b);
    return std::abs(da - db) <= eps;
}

static void checkNearColor(const unsigned char *pixel, unsigned char r, unsigned char g, unsigned char b, unsigned char a) {
    CHECK(nearByte(pixel[0], r));
    CHECK(nearByte(pixel[1], g));
    CHECK(nearByte(pixel[2], b));
    CHECK(nearByte(pixel[3], a));
}

static GLFWwindow* setupGLContext(int width = 64, int height = 64) {
    setWorkingDirectoryToProjectRoot();
    if (glfwInit() != GLFW_TRUE) return nullptr;
    GLFWwindow* window = createHiddenTestWindowPreferEGL(width, height, "test");
    if (!window) {
        glfwTerminate();
        return nullptr;
    }
    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
    return window;
}

static void teardownGLContext(GLFWwindow* window) {
    Scene::kill();

    if (window) {
        glfwMakeContextCurrent(nullptr);
        glfwDestroyWindow(window);
    }
    glfwTerminate();
}

class TestEngineGL : public EngineGL {
public:
    using EngineGL::EngineGL;
    
    std::vector<unsigned char> publicRenderRaytracerImage(
        const glm::mat4 &cameraFrame, const glm::mat4 &inverseViewProjection, int width, int height, const glm::vec4 &clearColor) const {
        return EngineGL::renderRaytracerImage(cameraFrame, inverseViewProjection, width, height, clearColor);
    }

    void addSphereToScene(const std::string& name, float r, glm::vec3 center) {
        Node *n = new Node(name);
        Sphere *s = new Sphere(r, center);
        n->setSphere(s);
        this->allNodes->nodes.push_back(n);
        this->scene->getSceneNode()->adopt(n);
    }
    
    void clearNodes() {
        this->allNodes->nodes.clear();
    }
    
    Node* getLastNode() {
        return this->allNodes->nodes.back();
    }
};

static Light* createDefaultLight(){ // mirror engineGL.init()'s
    Light* light = new Light();
    light->type = Light::PONCTUELLE;
    light->position = glm::vec3(3.0f, 3.0f, 3.0f);
    light->puissance = 1.0f;
    light->attenuation = 0.2f;
    light->couleurAmbiante = glm::vec3(0.1f, 0.1f, 0.1f);
    light->couleurDiffuse = glm::vec3(1.0f, 1.0f, 1.0f);
    light->couleurSpeculaire = glm::vec3(1.0f, 1.0f, 1.0f);
    return light;
}

TEST_CASE("Init (point gratuit pour valider l'environnement de test)") {
    CHECK(1 + 1 == 2);
}

TEST_CASE("phaseLargeRayonIntersections - sphere intersection") {
    GLFWwindow* window = setupGLContext(64, 64);
    REQUIRE(window != nullptr);

    {
        TestEngineGL engine(64, 64);
        REQUIRE(engine.init());

        engine.clearNodes();
        engine.addSphereToScene("TestSphere", 2.0f, glm::vec3(0.0f, 0.0f, -5.0f));

        Ray ray(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f));
        IntersectionData hit;

        engine.phaseLargeRayonIntersections(ray, hit);

        CHECK(hit.t != FLT_MAX);
        CHECK(nearFloat(hit.t, 3.0f));
    }

    teardownGLContext(window);
}

TEST_CASE("Lambert raytracing - sphere diffuse color") {
    GLFWwindow* window = setupGLContext(64, 64);
    REQUIRE(window != nullptr);

    {
        TestEngineGL engine(16, 16);
        engine.init();
        engine.clearNodes();

        engine.addSphereToScene("TestSphere", 2.0f, glm::vec3(0.0f, 0.0f, -5.0f));
        Node* sphereNode = engine.getLastNode();
        sphereNode->materialProperties.albedo = glm::vec3(1.0f, 0.0f, 0.0f); // Red
        sphereNode->materialProperties.diffuse = 1.0f;
        sphereNode->materialProperties.specular = 0.0f; // No specular

        Scene::getInstance()->lights = Resource_mgr<Light>();
        Light* light = createDefaultLight();
        light->type = Light::DIRECTIONNELLE;
        light->direction = glm::vec3(0.0f, 0.0f, -1.0f); // Light shines into screen, along view direction
        light->couleurAmbiante = glm::vec3(0.0f);
        light->couleurSpeculaire = glm::vec3(0.0f);
        Scene::getInstance()->lights.insert("Light1", light);

        glm::mat4 cameraFrame = glm::mat4(1.0f); // Camera at origin looking -Z
        glm::mat4 view = glm::mat4(1.0f);
        glm::mat4 proj = glm::perspective(glm::radians(45.0f), 1.0f, 1.0f, 100.0f);
        glm::mat4 invVP = glm::inverse(proj * view);

        auto img = engine.publicRenderRaytracerImage(cameraFrame, invVP, 16, 16, glm::vec4(0.0f));

        // Center pixel should hit the sphere (which is at -5 Z) directly in the middle
        int cx = 8, cy = 8;
        int idx = (cy * 16 + cx) * 4;

        // Normal is (0,0,1), light dir is (0,0,-1). Light vector is (0,0,1).
        // Lambert: max(0, dot(N, L)) = max(0, dot((0,0,1), (0,0,1))) = 1.0.
        // Red color should be 255.
        CHECK(img[idx] > 200); // Red
        CHECK(img[idx+1] == 0); // Green
        CHECK(img[idx+2] == 0); // Blue
    }

    teardownGLContext(window);
}

TEST_CASE("Blinn-Phong raytracing - specular highlight") {
    GLFWwindow* window = setupGLContext(16, 16);
    REQUIRE(window != nullptr);

    {
        TestEngineGL engine(16, 16);
        engine.init();
        engine.clearNodes();

        engine.addSphereToScene("TestSphere", 2.0f, glm::vec3(0.0f, 0.0f, -5.0f));
        Node* sphereNode = engine.getLastNode();
        sphereNode->materialProperties.albedo = glm::vec3(1.0f, 0.0f, 0.0f); // Red
        sphereNode->materialProperties.diffuse = 0.0f; // No diffuse
        sphereNode->materialProperties.specular = 1.0f;
        sphereNode->materialProperties.hardness = 32.0f;

        Scene::getInstance()->lights = Resource_mgr<Light>();
        Light* light = createDefaultLight();
        light->type = Light::DIRECTIONNELLE;
        light->direction = glm::vec3(0.0f, 0.0f, -1.0f); // Light shines into screen, along view direction
        light->couleurAmbiante = glm::vec3(0.0f);
        light->couleurSpeculaire = glm::vec3(0.0f, 1.0f, 0.0f); // Green specular light
        Scene::getInstance()->lights.insert("Light1", light);

        glm::mat4 cameraFrame = glm::mat4(1.0f);
        glm::mat4 view = glm::mat4(1.0f);
        glm::mat4 proj = glm::perspective(glm::radians(45.0f), 1.0f, 1.0f, 100.0f);
        glm::mat4 invVP = glm::inverse(proj * view);

        auto img = engine.publicRenderRaytracerImage(cameraFrame, invVP, 16, 16, glm::vec4(0.0f));

        int cx = 8, cy = 8;
        int idx = (cy * 16 + cx) * 4;

        // Should see green specular highlight in the center
        CHECK(img[idx] == 0);
        CHECK(img[idx+1] > 150);
        CHECK(img[idx+2] == 0);
    }

    teardownGLContext(window);
}

TEST_CASE("Sphere intersection") {
    NodeMaterialProperties mat;

    // check hit
    {
        Sphere s(2.0f, glm::vec3(0.0f, 0.0f, -5.0f));
        Ray ray(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f));
        IntersectionData hit;
        
        s.intersect(ray, hit, mat);
        
        REQUIRE(hit.t != FLT_MAX); 
        CHECK(nearFloat(hit.t, 3.f)); // 5 - 2 = 3
        checkNearVec3(hit.p, glm::vec3(0.0f, 0.0f, -3.0f));
        checkNearVec3(hit.n, glm::vec3(0.0f, 0.0f, 1.0f));
    }

    // Then check miss
    {
        Sphere s(1.0f, glm::vec3(2.0f, 0.0f, -5.0f));
        Ray ray(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f));
        IntersectionData hit;
        
        s.intersect(ray, hit, mat);
        
        CHECK(hit.t == FLT_MAX);
    }
}

TEST_CASE("Plane intersection") {
    NodeMaterialProperties mat;

    // 1. Test the hit
    {
        Plane p(glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, -2.0f, 0.0f));
        Ray ray(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f));
        IntersectionData hit;
        
        p.intersect(ray, hit, mat);
        
        REQUIRE(hit.t != FLT_MAX);
        CHECK(nearFloat(hit.t, 2.0f));
        checkNearVec3(hit.p, glm::vec3(0.0f, -2.0f, 0.0f));
        checkNearVec3(hit.n, glm::vec3(0.0f, 1.0f, 0.0f));
    }

    // 2. Test the miss
    {
        Plane p(glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, -2.0f, 0.0f));
        Ray ray(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f)); // Parallel ray
        IntersectionData hit;
        
        p.intersect(ray, hit, mat);
        
        CHECK(hit.t == FLT_MAX);
    }
}

TEST_CASE("Triangle intersection") {
    class TestModelGL : public ModelGL{
    public:
        using ModelGL::ModelGL;
        void publicIntersectionTriangleRayon( const Ray& ray, const glm::vec3 &vertex1, const glm::vec3 &vertex2, const glm::vec3 &vertex3, IntersectionData& intersection, const NodeMaterialProperties &materialProperties){
            return intersectionTriangleRayon(ray, vertex1, vertex2, vertex3, intersection, materialProperties);
        }
    };
    
    TestModelGL* model = new TestModelGL("TestModel", false);
    NodeMaterialProperties mat;
    Ray ray(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f));

    // 1. Test the hit
    {
        glm::vec3 vertex0 = glm::vec3(-1.0f, -2.0f,  1.0f);
        glm::vec3 vertex1 = glm::vec3( 1.0f, -2.0f,  1.0f);
        glm::vec3 vertex2 = glm::vec3( 0.0f, -2.0f, -1.0f);
        IntersectionData hit;
        
        model->publicIntersectionTriangleRayon(ray, vertex0, vertex1, vertex2, hit, mat);
        
        REQUIRE(hit.t != FLT_MAX);
        CHECK(nearFloat(hit.t, 2.0f));
        checkNearVec3(hit.p, glm::vec3(0.0f, -2.0f, 0.0f));
        checkNearVec3(hit.n, glm::vec3(0.0f, 1.0f, 0.0f));
    }

    // 2. Test the miss
    {
        glm::vec3 vertex0 = glm::vec3(10.0f, -2.0f, 10.0f);
        glm::vec3 vertex1 = glm::vec3(12.0f, -2.0f, 10.0f);
        glm::vec3 vertex2 = glm::vec3(11.0f, -2.0f,  8.0f);
        IntersectionData hit;
        
        model->publicIntersectionTriangleRayon(ray, vertex0, vertex1, vertex2, hit, mat);
        
        CHECK(hit.t == FLT_MAX);
    }
    
    delete model;
}

TEST_CASE("calculerVecteurLumiere - DIRECTIONNELLE") {
    Light l;
    l.type = Light::DIRECTIONNELLE;
    l.direction = glm::vec3(0.0f, -1.0f, 0.0f);
    
    glm::vec3 point(0.0f);
    glm::vec3 lVec;
    calculerVecteurLumiere(&l, point, lVec);
    
    checkNearVec3(lVec, glm::vec3(0.0f, 1.0f, 0.0f));
}

TEST_CASE("calculerVecteurLumiere - PONCTUELLE/PROJECTEUR") {
    Light l;
    l.type = Light::PONCTUELLE;
    l.position = glm::vec3(0.0f, 5.0f, 0.0f);
    
    glm::vec3 point(0.0f, 1.0f, 0.0f);
    glm::vec3 lVec;
    calculerVecteurLumiere(&l, point, lVec);
    
    checkNearVec3(lVec, glm::vec3(0.0f, 1.0f, 0.0f));
}

TEST_CASE("calculerEchantillonsGrilleRayons - sans vibration") {
    float ndcx = 0.0f, ndcy = 0.0f;
    calculerEchantillonsGrilleRayons(std::make_tuple(0.5f, 0.5f), ndcx, ndcy, 100.0f, 100.0f, false);
    
    CHECK(nearFloat(ndcx, .5f/100.0f));
    CHECK(nearFloat(ndcy, .5f/100.0f));
}

TEST_CASE("calculerEchantillonsGrilleRayons - avec vibration") {
    float ndcx = 0.0f, ndcy = 0.0f;
    calculerEchantillonsGrilleRayons(std::make_tuple(0.5f, 0.5f), ndcx, ndcy, 100.0f, 100.0f, true);
    
    // Bounds check
    float minExpected = (0.5f - 0.25f) / 100.0f;
    float maxExpected = (0.5f + 0.25f) / 100.0f;
    
    CHECK(ndcx >= minExpected);
    CHECK(ndcx <= maxExpected);
    CHECK(ndcy >= minExpected);
    CHECK(ndcy <= maxExpected);
}

TEST_CASE("genererLesEchantillonsSSAA - 1 sample") {
    std::vector<std::tuple<float, float>> offsets;
    float scale = genererLesEchantillonsSSAA(1, offsets);
    
    REQUIRE(offsets.size() == 1);
    CHECK(nearFloat(std::get<0>(offsets[0]), 0.5f));
    CHECK(nearFloat(std::get<1>(offsets[0]), 0.5f));
    CHECK(nearFloat(scale, 1.f));
}

TEST_CASE("genererLesEchantillonsSSAA - 2 samples") {
    std::vector<std::tuple<float, float>> offsets;
    float scale = genererLesEchantillonsSSAA(2, offsets);
    
    CHECK(offsets.size() == 2);
    CHECK(nearFloat(scale, .5f));
}

TEST_CASE("appliquerFlouMoyen (Blur 3x3) - borders remain 0 rgb, 255 a") {
    int w = 5, h = 5;
    std::vector<unsigned char> img(w * h * 4, 255);
    auto blurred = appliquerFlouMoyen(img, w, h, 3);
    
    // Check border
    CHECK(blurred[0] == 0);
    CHECK(blurred[1] == 0);
    CHECK(blurred[2] == 0);
    CHECK(blurred[3] == 255);
    
    // Check interior
    int centerIdx = (2 * w + 2) * 4;
    CHECK(blurred[centerIdx] == 255);
    CHECK(blurred[centerIdx + 1] == 255);
    CHECK(blurred[centerIdx + 2] == 255);
    CHECK(blurred[centerIdx + 3] == 255);
}

TEST_CASE("appliquerFlouMoyen (Blur 5x5) - borders remain 0 rgb, 255 a") {
    int w = 5, h = 5;
    std::vector<unsigned char> img(w * h * 4, 255);
    auto blurred = appliquerFlouMoyen(img, w, h, 5);

    // Check border
    CHECK(blurred[0] == 0);
    CHECK(blurred[1] == 0);
    CHECK(blurred[2] == 0);
    CHECK(blurred[3] == 255);

    // Check interior (only center pixel for 5x5 kernel on a 5x5 image)
    int centerIdx = (2 * w + 2) * 4;
    CHECK(blurred[centerIdx] == 255);
    CHECK(blurred[centerIdx + 1] == 255);
    CHECK(blurred[centerIdx + 2] == 255);
    CHECK(blurred[centerIdx + 3] == 255);
}

TEST_CASE("calculerAttenuationQuadratique") {
    GLFWwindow* window = setupGLContext(64, 64);
    REQUIRE(window != nullptr);

    {
        TestEngineGL engine(64, 64);
        REQUIRE(engine.init());
    
        Scene *scene = Scene::getInstance();
        scene->lights = Resource_mgr<Light>();
        Light* light = createDefaultLight();
        light->type = Light::PONCTUELLE;
        light->position = glm::vec3(3.0f, 3.0f, 3.0f);
        scene->lights.insert("Light1", light);
    
        engine.onWindowResize(64, 64);
        engine.animate(0);
        engine.render();
        glFinish();
    
        unsigned char centerPixel[4] = {0, 0, 0, 0};
        glReadPixels(32, 32, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, centerPixel);
        checkNearColor(centerPixel, 92, 86, 81, 255);
    }
    teardownGLContext(window);
}

TEST_CASE("calculerAngleProjecteurIncident") {
    GLFWwindow* window = setupGLContext(64, 64);
    REQUIRE(window != nullptr);

    {
        TestEngineGL engine(64, 64);
        REQUIRE(engine.init());
    
        Scene *scene = Scene::getInstance();
        scene->lights = Resource_mgr<Light>();
        Light* light = createDefaultLight();
        light->type = Light::PROJECTEUR;
        light->position = glm::vec3(0.0f, 3.0f, 3.0f);
        light->direction = glm::vec3(0.0f, -1.0f, -1.0f);
        light->puissance = 1.0f;
        scene->lights.insert("Light1", light);
    
        engine.onWindowResize(64, 64);

        // Pass 1: Test with degrees
        light->angleCone = 30.0f;
        engine.animate(0);
        engine.render();
        glFinish();
        unsigned char pixelDegrees[4] = {0, 0, 0, 0};
        glReadPixels(32, 32, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, pixelDegrees);

        // Pass 2: Test with radians
        light->angleCone = glm::radians(30.0f);
        engine.animate(0);
        engine.render();
        glFinish();
        unsigned char pixelRadians[4] = {0, 0, 0, 0};
        glReadPixels(32, 32, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, pixelRadians);

        // Helper to find which pixel is closest to the expected color
        auto getDiff = [](unsigned char p[4]) {
            return std::abs(p[0] - 127) + std::abs(p[1] - 119) + 
                   std::abs(p[2] - 112) + std::abs(p[3] - 255);
        };

        // Assert on the result that is closest to the expected values
        if (getDiff(pixelDegrees) <= getDiff(pixelRadians)) {
            checkNearColor(pixelDegrees, 127, 119, 112, 255);
        } else {
            checkNearColor(pixelRadians, 127, 119, 112, 255);
        }
    }
    teardownGLContext(window);
}

TEST_CASE("Lambert - Raster") {
    GLFWwindow* window = setupGLContext(64, 64);
    REQUIRE(window != nullptr);
    
    {
        TestEngineGL engine(64, 64);
        REQUIRE(engine.init());
    
        Scene *scene = Scene::getInstance();
        scene->lights = Resource_mgr<Light>();
        Light* light = createDefaultLight();
        light->type = Light::DIRECTIONNELLE;
        light->direction = glm::vec3(0.0f, 0.0f, -1.0f);
        light->puissance = 1.0f;
        light->couleurAmbiante = glm::vec3(0.0f);
        light->couleurSpeculaire = glm::vec3(0.0f);
        scene->lights.insert("Light1", light);
    
        engine.onWindowResize(64, 64);
        engine.animate(0);
        engine.render();
        glFinish();
    
        unsigned char centerPixel[4] = {0, 0, 0, 0};
        glReadPixels(32, 32, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, centerPixel);
        checkNearColor(centerPixel, 113, 99, 88, 255);
    }
    teardownGLContext(window);
}

TEST_CASE("Phong - Raster") {
    GLFWwindow* window = setupGLContext(64, 64);
    REQUIRE(window != nullptr);
    
    {
        TestEngineGL engine(64, 64);
        REQUIRE(engine.init());
    
        Scene *scene = Scene::getInstance();
        scene->lights = Resource_mgr<Light>();
        Light *light = createDefaultLight();
        light->type = Light::DIRECTIONNELLE;
        light->direction = glm::vec3(0.0f, 0.0f, -1.0f);
        light->puissance = 1.0f;
        light->couleurSpeculaire = glm::vec3(1.0f);
        scene->lights.insert("Light1", light);
    
        Node* bunny = scene->getNode("Bunny");
        if (bunny) {
            bunny->materialProperties.useGouraud = false;
            bunny->materialProperties.useBlinnPhong = false;
        }
    
        engine.onWindowResize(64, 64);
        engine.animate(0);
        engine.render();
        glFinish();
    
    
        unsigned char highlightPixel[4] = {0, 0, 0, 0};
        glReadPixels(38, 32, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, highlightPixel);
        checkNearColor(highlightPixel, 255, 235, 218, 255);
    }
    teardownGLContext(window);
}

TEST_CASE("BlinnPhong - Raster") {
    GLFWwindow* window = setupGLContext(64, 64);
    REQUIRE(window != nullptr);

    {
        TestEngineGL engine(64, 64);
        REQUIRE(engine.init());
    
        Scene *scene = Scene::getInstance();
        scene->lights = Resource_mgr<Light>();
        Light *light = createDefaultLight();
        light->type = Light::DIRECTIONNELLE;
        light->direction = glm::vec3(0.0f, 0.0f, -1.0f);
        light->puissance = 1.0f;
        light->couleurSpeculaire = glm::vec3(1.0f);
        scene->lights.insert("Light1", light);

        Node* bunny = scene->getNode("Bunny");
        if (bunny) {
            bunny->materialProperties.useGouraud = false;
            bunny->materialProperties.useBlinnPhong = true;
        }
    
        engine.onWindowResize(64, 64);
        engine.animate(0);
        engine.render();
        glFinish();
    

        unsigned char highlightPixel[4] = {0, 0, 0, 0};
        glReadPixels(31, 43, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, highlightPixel);
        checkNearColor(highlightPixel, 201, 183, 174, 255);
    }
    teardownGLContext(window);
}

TEST_CASE("Texture coords") {
    GLFWwindow* window = setupGLContext(64, 64);
    REQUIRE(window != nullptr);
    
    std::string vsSource = "#version 460\n" + readFile("./src/Materials/BaseMaterial/Main-VS.glsl");
    const char* vsSrc = vsSource.c_str();
    
    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &vsSrc, nullptr);
    glCompileShader(vs);
    
    GLint vsOk = GL_FALSE;
    glGetShaderiv(vs, GL_COMPILE_STATUS, &vsOk);
    REQUIRE(vsOk == GL_TRUE);

    GLuint prog = glCreateProgram();
    glAttachShader(prog, vs);

    const char* tfVaryings[] = { "textureCoords" };
    glTransformFeedbackVaryings(prog, 1, tfVaryings, GL_INTERLEAVED_ATTRIBS);

    glLinkProgram(prog);
    GLint linked = GL_FALSE;
    glGetProgramiv(prog, GL_LINK_STATUS, &linked);
    REQUIRE(linked == GL_TRUE);

    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    float positions[] = { 0.f,0.f,0.f,  1.f,0.f,0.f,  0.f,1.f,0.f };
    GLuint posVbo;
    glGenBuffers(1, &posVbo);
    glBindBuffer(GL_ARRAY_BUFFER, posVbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(positions), positions, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

    float normals[] = { 0.f,0.f,1.f,  0.f,0.f,1.f,  0.f,0.f,1.f };
    GLuint normVbo;
    glGenBuffers(1, &normVbo);
    glBindBuffer(GL_ARRAY_BUFFER, normVbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(normals), normals, GL_STATIC_DRAW);
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

    float uvs[] = { 0.25f,0.75f,  0.50f,0.25f,  0.75f,0.50f };
    GLuint uvVbo;
    glGenBuffers(1, &uvVbo);
    glBindBuffer(GL_ARRAY_BUFFER, uvVbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(uvs), uvs, GL_STATIC_DRAW);
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

    // Setup Transform Feedback Buffer directly
    GLuint tfBuf;
    glGenBuffers(1, &tfBuf);
    glBindBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, tfBuf);
    glBufferData(GL_TRANSFORM_FEEDBACK_BUFFER, 6 * sizeof(float), nullptr, GL_STATIC_READ);
    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, tfBuf);

    glUseProgram(prog);

    glm::mat4 identity(1.0f);
    glUniformMatrix4fv(glGetUniformLocation(prog, "Proj"),  1, GL_FALSE, glm::value_ptr(identity));
    glUniformMatrix4fv(glGetUniformLocation(prog, "Model"), 1, GL_FALSE, glm::value_ptr(identity));
    glUniformMatrix4fv(glGetUniformLocation(prog, "View"),  1, GL_FALSE, glm::value_ptr(identity));

    glEnable(GL_RASTERIZER_DISCARD);
    glBeginTransformFeedback(GL_POINTS);
    glDrawArrays(GL_POINTS, 0, 3);
    glEndTransformFeedback();
    glDisable(GL_RASTERIZER_DISCARD);

    std::vector<float> captured(6, 0.0f);
    glGetBufferSubData(GL_TRANSFORM_FEEDBACK_BUFFER, 0, captured.size() * sizeof(float), captured.data());

    CHECK(nearFloat(captured[0], 0.25f));
    CHECK(nearFloat(captured[1], 0.75f));
    CHECK(nearFloat(captured[2], 0.50f));
    CHECK(nearFloat(captured[3], 0.25f));
    CHECK(nearFloat(captured[4], 0.75f));
    CHECK(nearFloat(captured[5], 0.50f));

    glDeleteBuffers(1, &tfBuf);
    glDeleteBuffers(1, &uvVbo);
    glDeleteBuffers(1, &normVbo);
    glDeleteBuffers(1, &posVbo);
    glDeleteVertexArrays(1, &vao);
    glDeleteShader(vs);
    glDeleteProgram(prog);

    teardownGLContext(window);
}

TEST_CASE("Camera en world-space") {
    GLFWwindow* window = setupGLContext(64, 64);
    REQUIRE(window != nullptr);

    {
        TestEngineGL engine(64, 64);
        REQUIRE(engine.init());

        Scene *scene = Scene::getInstance();
        engine.onWindowResize(64, 64);
        engine.animate(0);
        engine.render();
        glFinish();
    
        Node *bunny = scene->getNode("Bunny");
        REQUIRE(bunny != nullptr);
        MaterialGL *mat = bunny->getMaterial();
        REQUIRE(mat != nullptr);
        GLuint fpId = mat->fp->getId();

        // Lire cameraPosition depuis le GPU
        GLint locCam = glGetUniformLocation(fpId, "cameraPosition");
        REQUIRE(locCam != -1);
        GLfloat gpuCam[3] = {0.0f, 0.0f, 0.0f};
        glGetUniformfv(fpId, locCam, gpuCam);

        glm::mat4 view = scene->camera()->getViewMatrix();
        glm::vec3 expectedCamPos = glm::vec3(glm::inverse(view)[3]);
        CHECK(nearFloat(gpuCam[0], expectedCamPos.x));
        CHECK(nearFloat(gpuCam[1], expectedCamPos.y));
        CHECK(nearFloat(gpuCam[2], expectedCamPos.z));
    }

    teardownGLContext(window);
}


TEST_CASE("couleurDiffuse lambert - tous types de lumiere") {
    int lightType    = Light::PONCTUELLE;
    glm::vec3 pos    = glm::vec3(0.0f, 3.0f, 3.0f);
    glm::vec3 dir    = glm::vec3(0.0f, -1.0f, -1.0f);
    float angleCone  = 45.0f;

    SUBCASE("DIRECTIONNELLE") {
        lightType = Light::DIRECTIONNELLE;
        dir       = glm::vec3(0.0f, 0.0f, -1.0f);
    }
    SUBCASE("PONCTUELLE") {
        lightType = Light::PONCTUELLE;
    }
    SUBCASE("PROJECTEUR") {
        lightType  = Light::PROJECTEUR;
        angleCone  = 60.0f;
    }

    GLFWwindow* window = setupGLContext(64, 64);
    REQUIRE(window != nullptr);
    {
        TestEngineGL engine(64, 64);
        REQUIRE(engine.init());

        Scene *scene = Scene::getInstance();
        scene->lights = Resource_mgr<Light>();
        Light *light = createDefaultLight();
        light->type            = lightType;
        light->position        = pos;
        light->direction       = dir;
        light->angleCone       = angleCone;
        light->puissance       = 1.0f;
        light->couleurAmbiante  = glm::vec3(0.0f);
        light->couleurDiffuse   = glm::vec3(1.0f, 0.0f, 0.0f); // rouge uniquement
        light->couleurSpeculaire= glm::vec3(0.0f);
        scene->lights.insert("Light1", light);

        engine.onWindowResize(64, 64);
        engine.animate(0);
        engine.render();
        glFinish();

        unsigned char pixel[4] = {0, 0, 0, 0};
        glReadPixels(32, 32, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, pixel);

        CHECK(pixel[0] > 60);
        CHECK(pixel[1] == 0);
        CHECK(pixel[2] == 0);
    }

    teardownGLContext(window);
}

TEST_CASE("Gouraud Lambert - Raster") {
    GLFWwindow* window = setupGLContext(64, 64);
    REQUIRE(window != nullptr);
    
    {
        TestEngineGL engine(64, 64);
        REQUIRE(engine.init());
    
        Scene *scene = Scene::getInstance();
        scene->lights = Resource_mgr<Light>();
        Light* light = createDefaultLight();
        light->type = Light::DIRECTIONNELLE;
        light->direction = glm::vec3(0.0f, 0.0f, -1.0f);
        light->puissance = 1.0f;
        light->couleurAmbiante = glm::vec3(0.0f);
        light->couleurDiffuse = glm::vec3(0.23,0.38,0.74);
        light->couleurSpeculaire = glm::vec3(0.0f);
        scene->lights.insert("Light1", light);
    
        Node* bunny = scene->getNode("Bunny");
        if (bunny) {
            bunny->materialProperties.useGouraud = true;
        }
    
        engine.onWindowResize(64, 64);
        engine.animate(0);
        engine.render();
        glFinish();
    
        unsigned char centerPixel[4] = {0, 0, 0, 0};
        glReadPixels(31, 35, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, centerPixel);
        checkNearColor(centerPixel, 18, 23, 31, 255);
    }
    teardownGLContext(window);
}

TEST_CASE("Gouraud Phong - Raster") {
    GLFWwindow* window = setupGLContext(64, 64);
    REQUIRE(window != nullptr);
    
    {
        TestEngineGL engine(64, 64);
        REQUIRE(engine.init());
    
        Scene *scene = Scene::getInstance();
        scene->lights = Resource_mgr<Light>();
        Light *light = createDefaultLight();
        light->type = Light::DIRECTIONNELLE;
        light->direction = glm::vec3(0.0f, 0.0f, -1.0f);
        light->puissance = 1.0f;
        light->couleurDiffuse = glm::vec3(0.23,0.38,0.74);
        light->couleurSpeculaire = glm::vec3(1.0f);
        scene->lights.insert("Light1", light);

        Node* bunny = scene->getNode("Bunny");
        if (bunny) {
            bunny->materialProperties.useGouraud = true;
            bunny->materialProperties.useBlinnPhong = false;
        }
    
        engine.onWindowResize(64, 64);
        engine.animate(0);
        engine.render();
        glFinish();
    
        unsigned char highlightPixel[4] = {0, 0, 0, 0};
        glReadPixels(39, 32, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, highlightPixel);
        checkNearColor(highlightPixel, 205, 220, 250, 255);
    }
    teardownGLContext(window);
}

TEST_CASE("Gouraud BlinnPhong - Raster") {
    GLFWwindow* window = setupGLContext(64, 64);
    REQUIRE(window != nullptr);

    {
        TestEngineGL engine(64, 64);
        REQUIRE(engine.init());
        
        Scene *scene = Scene::getInstance();
        scene->lights = Resource_mgr<Light>();
        Light *light = createDefaultLight();
        light->type = Light::DIRECTIONNELLE;
        light->direction = glm::vec3(0.0f, 0.0f, -1.0f);
        light->puissance = 1.0f;
        light->couleurDiffuse = glm::vec3(0.23,0.38,0.74);
        light->couleurSpeculaire = glm::vec3(1.0f);
        scene->lights.insert("Light1", light);

        Node* bunny = scene->getNode("Bunny");
        if (bunny) {
            bunny->materialProperties.useGouraud = true;
            bunny->materialProperties.useBlinnPhong = true;
        }
        
        engine.onWindowResize(64, 64);
        engine.animate(0);
        engine.render();
        glFinish();
        
        unsigned char highlightPixel[4] = {0, 0, 0, 0};
        glReadPixels(29, 33, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, highlightPixel);
        checkNearColor(highlightPixel, 177, 193, 227, 255);
    }
    teardownGLContext(window);
}

TEST_CASE("Gouraud - tous types de lumiere") {
    int lightType    = Light::PONCTUELLE;
    glm::vec3 pos    = glm::vec3(0.0f, 3.0f, 3.0f);
    glm::vec3 dir    = glm::vec3(0.0f, -1.0f, -1.0f);
    float angleCone  = 45.0f;

    SUBCASE("DIRECTIONNELLE") {
        lightType = Light::DIRECTIONNELLE;
        dir       = glm::vec3(0.0f, 0.0f, -1.0f);
    }
    SUBCASE("PONCTUELLE") {
        lightType = Light::PONCTUELLE;
    }
    SUBCASE("PROJECTEUR") {
        lightType  = Light::PROJECTEUR;
        angleCone  = 60.0f;
    }
    GLFWwindow* window = setupGLContext(64, 64);
    REQUIRE(window != nullptr);

    {
        TestEngineGL engine(64, 64);
        REQUIRE(engine.init());

        Scene *scene = Scene::getInstance();
        scene->lights = Resource_mgr<Light>();
        Light *light = createDefaultLight();
        light->type            = lightType;
        light->position        = pos;
        light->direction       = dir;
        light->angleCone       = angleCone;
        light->puissance       = 1.0f;
        light->couleurAmbiante  = glm::vec3(0.0f);
        light->couleurDiffuse   = glm::vec3(1.0f, 0.0f, 0.0f);
        light->couleurSpeculaire = glm::vec3(0.0f);
        scene->lights.insert("Light1", light);

        Node* bunny = scene->getNode("Bunny");
        if (bunny) {
            bunny->materialProperties.useGouraud = true;
        }

        engine.onWindowResize(64, 64);
        engine.animate(0);
        engine.render();
        glFinish();

        unsigned char pixel[4] = {0, 0, 0, 0};
        glReadPixels(32, 32, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, pixel);

        CHECK(pixel[0] > 60);
        CHECK(pixel[1] == 0);
        CHECK(pixel[2] == 0);
    }
    teardownGLContext(window);
}

TEST_CASE("calculerDeplacementDeCollision - basic") {
    IntersectionData intersection;
    intersection.p = glm::vec3(0.0f, 0.0f, 0.0f);
    intersection.n = glm::vec3(0.0f, 1.0f, 0.0f);
    
    Ray ray(glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    
    glm::vec3 result = calculerDeplacementDeCollision(intersection, ray);
    checkNearVec3(result, glm::vec3(0.0f, SHADOW_EPSILON, 0.0f), 1e-6f);
}

TEST_CASE("calculerDeplacementDeCollision - prevents self-intersection") {
    Sphere sphere(1.0f, glm::vec3(0.0f));
    
    IntersectionData intersection;
    intersection.p = glm::vec3(0.0f, 0.0f, 1.0f);
    intersection.n = glm::vec3(0.0f, 0.0f, 1.0f); 
    
    Ray ray(glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, 1.0f)); 
    
    glm::vec3 offsetPoint = calculerDeplacementDeCollision(intersection, ray);
    CHECK(glm::length(offsetPoint) > 1.0f);
    
    Ray shadowRay(offsetPoint, intersection.n);
    IntersectionData shadowHit;
    NodeMaterialProperties materialProps;
    
    sphere.intersect(shadowRay, shadowHit, materialProps);
    CHECK(shadowHit.t == FLT_MAX);
    
    Ray shadowRayInward(offsetPoint, -intersection.n);  // Pointing back toward sphere
    IntersectionData shadowHitInward;
    
    sphere.intersect(shadowRayInward, shadowHitInward, materialProps);
    
    bool check = shadowHitInward.t > SHADOW_EPSILON || nearFloat(shadowHitInward.t, FLT_MAX);
    CHECK(check);
}