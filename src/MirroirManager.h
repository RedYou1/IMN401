#ifndef _MirroirManager_
#define _MirroirManager_

#include "FrameBufferObject.h"
#include "Node.h"
#include "Materials/MirroirMaterial/MirroirMaterial.h"
#include "Camera.h"
#include <glm/glm.hpp>

class MirroirManager {
public:
    MirroirManager(Node* cameraNode, Node* mirroirNode, int width, int height);
    ~MirroirManager();

    void renderMirroirView(void* engine, float secondsSinceStart);

    MirroirMaterial* getMaterial() { return m_material; }

private:
    bool m_current_fbo;
    Camera* m_camera;
    Node* m_mirroirNode;
    Node* m_cameraNode;
    FrameBufferObject* m_fbo1;
    FrameBufferObject* m_fbo2;
    MirroirMaterial* m_material;
    glm::mat4 m_lastViewMatrix;
    glm::mat4 m_lastProjMatrix;
};

#endif