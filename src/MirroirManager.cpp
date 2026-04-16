#include "MirroirManager.h"
#include "Plane.h"
#include "Scene.h"
#include "Camera.h"
#include "EngineGL.h"
#include <glm/gtc/matrix_transform.hpp>

MirroirManager::MirroirManager(Node* cameraNode, Node* mirroirNode, int width, int height)
    : m_cameraNode(cameraNode), m_mirroirNode(mirroirNode)
{
    m_camera = new Camera("mirror camera");
    m_fbo = new FrameBufferObject("MirroirFBO", width, height);
    m_material = new MirroirMaterial("IMN401-MatMirroir");
    m_material->setMirroirTexture(m_fbo->getColorTexture());
    if (m_mirroirNode) m_mirroirNode->setMaterial(m_material);
}

MirroirManager::~MirroirManager() {
    delete m_fbo;
    delete m_material;
}

void MirroirManager::renderMirroirView(void* engineptr, float secondsSinceStart) {
    EngineGL* engine = (EngineGL*)engineptr;
    if (!m_mirroirNode) return;
    Scene* scene = Scene::getInstance();
    Camera* camera = scene->camera();

    // ---- Save camera state ----
    glm::mat4 savedViewMatrix = camera->getViewMatrix();
    glm::mat4 savedUpMatrix = camera->frame()->getMatrixCopy();
    glm::mat4 savedProjMatrix = camera->getProjectionMatrix();

    glm::mat4 mirrorUpMatrix = m_cameraNode->frame()->getMatrixCopy();
    glm::mat4 mirrorWorld = glm::inverse(mirrorUpMatrix);

    // --- extraire position caméra ---
    glm::vec3 mirrorPos = glm::vec3(mirrorWorld[3]);
    glm::vec3 mirrorForward = -glm::vec3(mirrorWorld[2]);

    // --- reconstruire view matrix ---
    glm::mat4 mirroredView = glm::lookAt(
        mirrorPos,
        mirrorPos + mirrorForward,
        glm::vec3(0, 1, 0)
    );

    m_camera->setUpFromMatrix(glm::inverse(mirroredView));
    m_camera->setProjectionMatrix(savedProjMatrix);
    m_camera->setUpdate(true);
    scene->setCamera(m_camera);
    
    // ---- Render scene into FBO ----
    // Reflection flips handedness — reverse winding so culling stays correct
    glFrontFace(GL_CW);
    m_material->rendering = false;
    m_fbo->enable();
    engine->animate(secondsSinceStart);
    engine->render();
    m_fbo->disable();
    m_material->rendering = true;
    glFrontFace(GL_CCW);

    // ---- Restore camera ----
    camera->setUpFromMatrix(savedUpMatrix);
    camera->setProjectionMatrix(savedProjMatrix);
    camera->setUpdate(true);
    scene->setCamera(camera);
}
