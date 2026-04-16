#pragma once

#include "ModelGL.h"

class CubePrimitive : public ModelGL
{
public:
    CubePrimitive();

    void drawGeometry(GLint type = GL_TRIANGLES) override;

private:
    void generateCube();
};