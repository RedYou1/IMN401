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

    v = {
        {-1,-1,-1}, {1,-1,-1}, {1,1,-1}, {-1,1,-1},
        {-1,-1, 1}, {1,-1, 1}, {1,1, 1}, {-1,1, 1}
    };

    auto addFace = [&](int a, int b, int c)
        {
            Face face;
            face.s1 = a;
            face.s2 = b;
            face.s3 = c;
            f.push_back(face);
        };

    // cube (12 triangles)
    addFace(0, 1, 2); addFace(2, 3, 0);
    addFace(1, 5, 6); addFace(6, 2, 1);
    addFace(5, 4, 7); addFace(7, 6, 5);
    addFace(4, 0, 3); addFace(3, 7, 4);
    addFace(3, 2, 6); addFace(6, 7, 3);
    addFace(4, 5, 1); addFace(1, 0, 4);

    m_Model->nb_vertex = v.size();
    m_Model->nb_faces = f.size();

    // normals simples (flat shading)
    m_Model->listNormals.resize(v.size(), glm::vec3(0.0f));
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