
#include "BaseMaterial.h"
#include "Node.h"
#include <glm/gtc/type_ptr.hpp>

BaseMaterial::BaseMaterial(std::string name) : MaterialGL(name) {

    vp = new GLProgram(MaterialPath + "BaseMaterial/Main-VS.glsl", GL_VERTEX_SHADER);
    fp = new GLProgram(MaterialPath + "BaseMaterial/Main-FS.glsl", GL_FRAGMENT_SHADER);

    m_ProgramPipeline->useProgramStage(vp, GL_VERTEX_SHADER_BIT);
    m_ProgramPipeline->useProgramStage(fp, GL_FRAGMENT_SHADER_BIT);

    l_View = glGetUniformLocation(vp->getId(), "View");
    l_Proj = glGetUniformLocation(vp->getId(), "Proj");
    l_Model = glGetUniformLocation(vp->getId(), "Model");
    l_TextureSampler = glGetUniformLocation(fp->getId(), "textureSampler");
    l_CameraPosition = glGetUniformLocation(fp->getId(), "cameraPosition");
    l_Shininess = glGetUniformLocation(fp->getId(), "shininess");
    l_Metalness = glGetUniformLocation(fp->getId(), "metalness");
    l_UseBlinnPhong = glGetUniformLocation(fp->getId(), "useBlinnPhong");
    l_UseGouraud = glGetUniformLocation(fp->getId(), "useGouraud");
    
    vpCamPosLoc = glGetUniformLocation(vp->getId(), "cameraPosition");
    vpShininessLoc = glGetUniformLocation(vp->getId(), "shininess");
    vpUseBlinnPhongLoc = glGetUniformLocation(vp->getId(), "useBlinnPhong");
    vpUseGouraudLoc = glGetUniformLocation(vp->getId(), "useGouraud");
    vpTextureSampler = glGetUniformLocation(vp->getId(), "textureSampler");
    m_Texture = nullptr;
}

BaseMaterial::~BaseMaterial() {}

void BaseMaterial::render(Node *o) {

    m_ProgramPipeline->bind();

    if (m_Texture != nullptr) {
        glBindTextureUnit(0, m_Texture->getId());
        glProgramUniform1i(fp->getId(), l_TextureSampler, 0);
        glProgramUniform1i(vp->getId(), vpTextureSampler, 0);
    }

    o->drawGeometry(GL_TRIANGLES);

    if (m_Texture != nullptr) {
        glBindTextureUnit(0, 0);
    }

    m_ProgramPipeline->release();
}

void BaseMaterial::setTexture(Texture2D *texture) {
    m_Texture = texture;
}

void BaseMaterial::animate(Node *o, const float elapsedTime) {

    /**********************************************
    TP 2 - A completer
    Calculer et Transmettre les matrices Model View et Proj au shaders
    - Utilisez glm::value_ptr(mat) pour trouver le pointeur de la matrice mat a transmettre au GPU via la fonction glProgramUniform*()
    - Une matrice 4X4 se transmet grace a glProgramUniformMatrix4fv
    ***********************************************/
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

    glProgramUniform1f(fp->getId(), l_Shininess, o->materialProperties.hardness);
    glProgramUniform1f(fp->getId(), l_Metalness, o->materialProperties.shininess);
    glProgramUniform1i(fp->getId(), l_UseBlinnPhong, o->materialProperties.useBlinnPhong ? 1 : 0);
    glProgramUniform1i(fp->getId(), l_UseGouraud, o->materialProperties.useGouraud ? 1 : 0);
    
    // Pass lighting uniforms to vertex shader for Gouraud shading
    glProgramUniform3fv(vp->getId(), vpCamPosLoc, 1, glm::value_ptr(camPos));
    glProgramUniform1f(vp->getId(), vpShininessLoc, o->materialProperties.hardness);
    glProgramUniform1i(vp->getId(), vpUseBlinnPhongLoc, o->materialProperties.useBlinnPhong ? 1 : 0);
    glProgramUniform1i(vp->getId(), vpUseGouraudLoc, o->materialProperties.useGouraud ? 1 : 0);
}
