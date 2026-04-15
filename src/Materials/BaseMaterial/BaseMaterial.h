

#ifndef _BaseMaterial_
#define _BaseMaterial_

#include "MaterialGL.h"
#include "Texture2D.h"

class BaseMaterial : public MaterialGL {
public:
    BaseMaterial(std::string name = "");

    ~BaseMaterial();

    virtual void render(Node *o);

    virtual void animate(Node *o, const float elapsedTime);

    void setTexture(Texture2D *texture);

    virtual void displayInterface(){};
protected:
    GLuint l_View, l_Proj, l_Model; // location of uniforms
    GLuint l_TextureSampler;
    GLuint l_CameraPosition;
    GLuint l_Shininess;
    GLuint l_Metalness;
    GLuint l_UseBlinnPhong;
    GLuint l_UseGouraud;
    Texture2D *m_Texture;
    
    GLint vpCamPosLoc;
    GLint vpShininessLoc;
    GLint vpUseBlinnPhongLoc;
    GLint vpUseGouraudLoc;
    GLint vpTextureSampler;
};

#endif