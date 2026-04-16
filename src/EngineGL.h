
#ifndef _ENGINE_GL_H
#define _ENGINE_GL_H

#include "NodeCollector.h"
#include "Scene.h"
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <atomic>
#include <future>
#include <string>
#include <vector>

#include "FrameBufferObject.h"

#include "GLProgram.h"
#include "GLProgramPipeline.h"

#include "Effects/Display/Display.h"
#include "Light.h"
#include "Ray.h"
#include "IntersectionData.h"

class EngineGL {
public:
    EngineGL(int width, int height);

    ~EngineGL();

    bool init();

    bool loadScene(std::string filename);

    void render();

    void animate(const float elapsedTime);

    void onWindowResize(int w, int h);

    double getFrameTime();

    int getWidth() { return m_Width; };

    int getHeight() { return m_Height; };

    void setClearColor(glm::vec4 color);

    virtual void setupEngine();

    virtual void displayInterface();

    enum class RenderScene {
        Rasterization = 0,
        Raytracer
    };

    void setRenderScene(RenderScene sceneType) { m_renderScene = sceneType; }
    RenderScene getRenderScene() const { return m_renderScene; }

    const float* get_triangle_fan_vertices() const { return m_fan_vertices; }
    void phaseLargeRayonIntersections(const Ray& ray, IntersectionData& intersection) const;

    void refreshNodeCollector();
protected:
    int m_Width;
    int m_Height;

    Scene *scene;

    NodeCollector *allNodes;

    FrameBufferObject *myFBO;
    Display *display;

    RenderScene m_renderScene;

    // Triangle fan scene resources
    bool m_raytracerReady;
    GLuint m_triangleVao;
    GLuint m_triangleVbo;
    int m_triangleVertexCount;
    GLProgram *m_triangleVs;
    GLProgram *m_triangleFs;
    GLProgramPipeline *m_trianglePipeline;
    const float m_fan_vertices[28] = {
        0.0f,  0.0f, 0.0f, 1.0f,
        0.6f,  0.4f, 0.0f, 1.0f,
        0.8f, -0.2f, 0.0f, 1.0f,
        0.0f, -0.7f, 0.0f, 1.0f,
       -0.8f, -0.2f, 0.0f, 1.0f,
       -0.6f,  0.4f, 0.0f, 1.0f,
        0.6f,  0.4f, 0.0f, 1.0f
    };

    std::vector<unsigned char> renderRaytracerImage(const glm::mat4 &cameraFrame, const glm::mat4 &inverseViewProjection, int width, int height, const glm::vec4 &clearColor) const;
private:
    void calculerOrientationRayon(
        const glm::mat4 &inverseViewProjection,
        float clipX,
        float clipY,
        const glm::vec3 &cameraPos,
        Ray &ray,
        glm::vec3 &eyeVec) const;

    glm::vec3 calculerIlluminationLancerRayons(const IntersectionData &intersection, const glm::vec3 &eyeVec, const glm::vec3 &ambient) const;
    glm::vec3 calculerBlinnPhongLancerRayons(const Light *light, const IntersectionData &intersection, const glm::vec3 &lightVec, const glm::vec3 &eyeVec) const;

    void launchRaytracerIfNeeded(const glm::mat4 &cameraFrame, const glm::mat4 &inverseViewProjection, float fov, float aspect, int width, int height, const glm::vec4 &clearColor);
    void pollRaytracerResult();

    void setupRaytracer();

    std::future<std::vector<unsigned char>> m_raytracerTask;
    bool m_raytracerTaskRunning;
    bool m_raytracerDirty;
    bool m_raytracerHasImage;
    std::vector<unsigned char> m_raytracerImage;

    glm::mat4 m_lastRaytracerCameraFrame;
    float m_lastRaytracerFoV;
    float m_lastRaytracerAspect;
    bool m_lastRaytracerParamsValid;
    int m_raytracerResolutionWidth;
    int m_raytracerResolutionHeight;
    int m_blurKernelSize;
    int m_raytracerSsaaSamples;
    bool m_raytracerJitterEnabled;
    bool m_rasterUseBlinnPhong;
    bool m_rasterUseGouraud;

    mutable std::atomic<int> m_raytracerRowsDone;
    mutable std::atomic<int> m_raytracerRowsTotal;
};
#endif
