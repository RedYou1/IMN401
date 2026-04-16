#ifndef _MirroirMaterial_
#define _MirroirMaterial_

#include "MaterialGL.h"
#include "Texture2D.h"

class MirroirMaterial : public MaterialGL {
public:
    MirroirMaterial(std::string name = "");
    ~MirroirMaterial() = default;

    void setMirroirTexture(Texture2D *texture);
    void render(Node *o);
    void animate(Node *o, const float elapsedTime);

    bool rendering;
protected:
    GLuint l_View, l_Proj, l_Model;
    GLuint l_CameraPosition;
    GLint vpCamPosLoc;

    GLuint l_MirroirTextureSampler, l_MirroirRendering;
    Texture2D *m_MirroirTexture;
};

#endif