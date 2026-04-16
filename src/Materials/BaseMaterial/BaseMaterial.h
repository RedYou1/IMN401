#ifndef _BaseMaterial_
#define _BaseMaterial_

#include "MaterialGL.h"
#include "Texture2D.h"

class BaseMaterial : public MaterialGL {
public:
    BaseMaterial(std::string name = "");

    ~BaseMaterial();

    virtual void render(Node* o);
    virtual void animate(Node* o, const float elapsedTime);

    void setTexture(Texture2D* texture);
    void enableTexture(bool enable);
    bool isTextureEnabled() const;

    virtual void displayInterface() {};

protected:
    GLuint l_View, l_Proj, l_Model;
    GLuint l_TextureSampler;
    GLuint l_CameraPosition;
    GLuint l_Shininess;
    GLuint l_Metalness;
    GLuint l_UseBlinnPhong;
    GLuint l_UseGouraud;
    GLuint l_Albedo;
    GLuint l_UseTexture;

    Texture2D* m_Texture;
    bool m_UseTexture = true;

    GLint vpCamPosLoc;
    GLint vpShininessLoc;
    GLint vpUseBlinnPhongLoc;
    GLint vpUseGouraudLoc;
    GLint vpTextureSampler;
};

#endif