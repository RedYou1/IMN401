/*
 *	(c) XLim, UMR-CNRS
 *	Authors: G.Gilet, A. Mercier-Aubin
 *
 */
#ifndef _MODELGL_H
#define _MODELGL_H

#define MESH_EPSILON 1e-4f

#include "GeometricModel.h"
#include <glad/glad.h>
#include <string>
#include "Ray.h"
#include "IntersectionData.h"

class ModelGL {
public:
    ModelGL(std::string name, bool loadnow = true);
    ~ModelGL();
    virtual void drawGeometry(GLint type = GL_TRIANGLES);
    void loadToGPU();

    std::string getName() { return m_Name; }
    bool show_interface;
    void displayInterface();
    GeometricModel *getGeometricModel() {
        return m_Model;
    }

    void intersect(const Ray& ray, IntersectionData& intersection, const NodeMaterialProperties &materialProperties);

protected:
    std::string m_Name;
    bool m_hasLoaded;

    GeometricModel *m_Model;

    // Buffers and Arrays
    unsigned int VA_Main;
    unsigned int VBO_Vertex;
    unsigned int VBO_Faces;
    unsigned int VBO_TexCoords;
    unsigned int VBO_Normals;
    unsigned int VBO_Tangents;
    unsigned int VBO_BorderData;

    void intersectionTriangleRayon(const Ray& ray, const glm::vec3 &vertex1, const glm::vec3 &vertex2, const glm::vec3 &vertex3, IntersectionData& intersection, const NodeMaterialProperties &materialProperties);
};

#endif
