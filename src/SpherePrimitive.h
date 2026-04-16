#pragma once

#include "ModelGL.h"


class SpherePrimitive : public ModelGL
{
public:
    SpherePrimitive(int latSegments = 16, int lonSegments = 16);

    void drawGeometry(GLint type = GL_TRIANGLES) override;

private:
    int m_lat, m_lon;

    void generateSphere();
};