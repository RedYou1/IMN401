#include "MirroirMaterial.h"
#include "Node.h"
#include <glm/gtc/type_ptr.hpp>

MirroirMaterial::MirroirMaterial(std::string name) : MaterialGL(name), m_MirroirTexture(nullptr) {
    // Utilise les shaders spécifiques du mirroir
    vp = new GLProgram(MaterialPath + "MirroirMaterial/Main-VS.glsl", GL_VERTEX_SHADER);
    fp = new GLProgram(MaterialPath + "MirroirMaterial/Main-FS.glsl", GL_FRAGMENT_SHADER);
    m_ProgramPipeline->useProgramStage(vp, GL_VERTEX_SHADER_BIT);
    m_ProgramPipeline->useProgramStage(fp, GL_FRAGMENT_SHADER_BIT);
    l_View = glGetUniformLocation(vp->getId(), "View");
    l_Proj = glGetUniformLocation(vp->getId(), "Proj");
    l_Model = glGetUniformLocation(vp->getId(), "Model");
    l_CameraPosition = glGetUniformLocation(fp->getId(), "cameraPosition");
    vpCamPosLoc = glGetUniformLocation(vp->getId(), "cameraPosition");

    l_MirroirTextureSampler = glGetUniformLocation(fp->getId(), "mirroirTextureSampler");
    l_MirroirRendering = glGetUniformLocation(fp->getId(), "mirrorRendering");
}

void MirroirMaterial::setMirroirTexture(Texture2D *texture) {
    m_MirroirTexture = texture;
}

void MirroirMaterial::render(Node *o) {
    m_ProgramPipeline->bind();
    if (m_MirroirTexture != nullptr) {
        glBindTextureUnit(0, m_MirroirTexture->getId());
        glProgramUniform1i(fp->getId(), l_MirroirTextureSampler, 0);
        glProgramUniform1f(fp->getId(), l_MirroirRendering, (float)rendering);
    }
    o->drawGeometry(GL_TRIANGLES);
    if (m_MirroirTexture != nullptr) {
        glBindTextureUnit(0, 0);
    }
    m_ProgramPipeline->release();
}

void MirroirMaterial::animate(Node *o, const float elapsedTime) {
    (void)elapsedTime;

    Scene *scene = Scene::getInstance();
    Camera *cam = scene->camera();

    const glm::mat4 model = o->frame()->getModelMatrix();
    const glm::mat4 view = cam->getViewMatrix();
    const glm::mat4 proj = cam->getProjectionMatrix();

    glProgramUniformMatrix4fv(vp->getId(), l_Model, 1, GL_FALSE, glm::value_ptr(model));
    glProgramUniformMatrix4fv(vp->getId(), l_View, 1, GL_FALSE, glm::value_ptr(view));
    glProgramUniformMatrix4fv(vp->getId(), l_Proj, 1, GL_FALSE, glm::value_ptr(proj));

    glm::vec3 camPos = glm::vec3(glm::inverse(view)[3]);
    glProgramUniform3fv(fp->getId(), l_CameraPosition, 1, glm::value_ptr(camPos));
    
    // Pass lighting uniforms to vertex shader for Gouraud shading
    glProgramUniform3fv(vp->getId(), vpCamPosLoc, 1, glm::value_ptr(camPos));
}
