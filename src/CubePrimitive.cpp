#include "CubePrimitive.h"
#include "GeometricModel.h"

CubePrimitive::CubePrimitive()
    : ModelGL("CubePrimitive", false)
{
    generateCube();
    loadToGPU();
}

void CubePrimitive::generateCube()
{
    m_Model = new GeometricModel();

    auto& v = m_Model->listVertex;
    auto& f = m_Model->listFaces;
    auto& n = m_Model->listNormals;

    v.clear();
    f.clear();
    n.clear();

    auto addFace = [&](glm::vec3 a, glm::vec3 b, glm::vec3 c)
        {
            int start = v.size();

            v.push_back(a);
            v.push_back(b);
            v.push_back(c);

            glm::vec3 normal = glm::normalize(glm::cross(b - a, c - a));

            n.push_back(normal);
            n.push_back(normal);
            n.push_back(normal);

            Face face;
            face.s1 = start;
            face.s2 = start + 1;
            face.s3 = start + 2;
            f.push_back(face);
        };

    // 6 faces du cube (2 triangles par face)

    // -Z
    addFace({ -1,-1,-1 }, { 1,-1,-1 }, { 1, 1,-1 });
    addFace({ 1, 1,-1 }, { -1, 1,-1 }, { -1,-1,-1 });

    // +Z
    addFace({ 1,-1, 1 }, { -1,-1, 1 }, { -1, 1, 1 });
    addFace({ -1, 1, 1 }, { 1, 1, 1 }, { 1,-1, 1 });

    // -X
    addFace({ -1,-1,-1 }, { -1, 1,-1 }, { -1, 1, 1 });
    addFace({ -1, 1, 1 }, { -1,-1, 1 }, { -1,-1,-1 });

    // +X
    addFace({ 1,-1, 1 }, { 1, 1, 1 }, { 1, 1,-1 });
    addFace({ 1, 1,-1 }, { 1,-1,-1 }, { 1,-1, 1 });

    // -Y
    addFace({ -1,-1,-1 }, { -1,-1, 1 }, { 1,-1, 1 });
    addFace({ 1,-1, 1 }, { 1,-1,-1 }, { -1,-1,-1 });

    // +Y
    addFace({ -1, 1, 1 }, { -1, 1,-1 }, { 1, 1,-1 });
    addFace({ 1, 1,-1 }, { 1, 1, 1 }, { -1, 1, 1 });

    m_Model->nb_vertex = v.size();
    m_Model->nb_faces = f.size();
}

void CubePrimitive::drawGeometry(GLint type)
{
    glBindVertexArray(VA_Main);

    glDrawElements(
        type,
        static_cast<GLsizei>(m_Model->listFaces.size() * 3),
        GL_UNSIGNED_INT,
        0
    );
}