#include "SpherePrimitive.h"
#include "GeometricModel.h"
#include <cmath>

SpherePrimitive::SpherePrimitive(int lat, int lon)
    : ModelGL("SpherePrimitive", false),
    m_lat(lat),
    m_lon(lon)
{
    generateSphere();
    loadToGPU();
}

void SpherePrimitive::generateSphere()
{
    m_Model = new GeometricModel();

    auto& v = m_Model->listVertex;
    auto& f = m_Model->listFaces;

    const float PI = 3.14159265359f;

    for (int y = 0; y <= m_lat; ++y)
    {
        for (int x = 0; x <= m_lon; ++x)
        {
            float xSeg = (float)x / m_lon;
            float ySeg = (float)y / m_lat;

            glm::vec3 p;
            p.x = std::cos(xSeg * 2.0f * PI) * std::sin(ySeg * PI);
            p.y = std::cos(ySeg * PI);
            p.z = std::sin(xSeg * 2.0f * PI) * std::sin(ySeg * PI);

            v.push_back(p);
        }
    }

    for (int y = 0; y < m_lat; ++y)
    {
        for (int x = 0; x < m_lon; ++x)
        {
            int i0 = y * (m_lon + 1) + x;
            int i1 = i0 + 1;
            int i2 = i0 + (m_lon + 1);
            int i3 = i2 + 1;

            Face f1{ i0, i2, i1 };
            Face f2{ i1, i2, i3 };

            f.push_back(f1);
            f.push_back(f2);
        }
    }

    m_Model->nb_vertex = v.size();
    m_Model->nb_faces = f.size();

    m_Model->listNormals.resize(v.size());

    // normales simples (approx sphère)
    for (size_t i = 0; i < v.size(); ++i)
        m_Model->listNormals[i] = glm::normalize(v[i]);
}

void SpherePrimitive::drawGeometry(GLint type)
{
    glBindVertexArray(VA_Main);
    glDrawElements(
        type,
        static_cast<GLsizei>(m_Model->listFaces.size() * 3),
        GL_UNSIGNED_INT,
        0
    );
}