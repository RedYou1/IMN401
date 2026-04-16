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
    m_fbo1 = new FrameBufferObject("MirroirFBO1", width, height);
    m_fbo2 = new FrameBufferObject("MirroirFBO2", width, height);
    m_material = new MirroirMaterial("IMN401-MatMirroir");
    if (m_mirroirNode) m_mirroirNode->setMaterial(m_material);
}

MirroirManager::~MirroirManager() {
    delete m_fbo1;
    delete m_fbo2;
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

    glm::mat4 mirrorUpMatrix = m_cameraNode->frame()->getMatrixCopy();

    // --- extraire position caméra ---
    glm::vec3 mirrorPos = glm::vec3(mirrorUpMatrix[3]);
    glm::vec3 mirrorForward = -glm::vec3(mirrorUpMatrix[2]);

    // --- reconstruire view matrix ---
    glm::mat4 mirroredView = glm::lookAt(
        mirrorPos,
        mirrorPos + mirrorForward,
        glm::vec3(0, 1, 0)
    );

    m_camera->setUpFromMatrix(glm::inverse(mirroredView));
    m_camera->setProjectionMatrix(camera->getProjectionMatrix());
    m_camera->setUpdate(true);
    scene->setCamera(m_camera);
    
    MaterialGL* camMat = m_cameraNode->getMaterial();
    m_cameraNode->setMaterial(nullptr);
    
    FrameBufferObject* fbo;
    if(m_current_fbo){
        fbo = m_fbo1;
    }else{
        fbo = m_fbo2;
    }
    m_current_fbo = !m_current_fbo;

    // ---- Render scene into FBO ----
    // Reflection flips handedness — reverse winding so culling stays correct
    glFrontFace(GL_CW);
    m_material->rendering = false;
    fbo->enable();
    engine->animate(secondsSinceStart);
    engine->render();
    fbo->disable();
    m_material->rendering = true;
    glFrontFace(GL_CCW);

    m_material->setMirroirTexture(fbo->getColorTexture());

    // ---- Restore camera ----
    scene->setCamera(camera);
    m_cameraNode->setMaterial(camMat);
}
