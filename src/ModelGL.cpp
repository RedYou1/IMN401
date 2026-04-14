#include "ModelGL.h"
#include <glm/glm.hpp>

ModelGL::ModelGL(std::string name, bool loadnow):m_hasLoaded(loadnow){
    this->m_Name = name;

    m_Model = new GeometricModel(name, loadnow);

    if (loadnow)
        loadToGPU();
}

ModelGL::~ModelGL() {
    if (m_hasLoaded){
        glDeleteBuffers(1, &VBO_Vertex);
        glDeleteBuffers(1, &VBO_Normals);
        glDeleteBuffers(1, &VBO_Faces);
        glDeleteBuffers(1, &VBO_TexCoords);
        glDeleteBuffers(1, &VBO_Tangents);

        glBindVertexArray(0);
        glDeleteVertexArrays(1, &VA_Main);
    }
}

void ModelGL::loadToGPU() {
    if (m_Model != NULL) {
        glCreateVertexArrays(1, &VA_Main);

        // Create VBO and Send data to GPU
        if (m_Model->listVertex.size() > 0) {
            glCreateBuffers(1, &VBO_Vertex);
            glNamedBufferData(VBO_Vertex, m_Model->nb_vertex * sizeof(glm::vec3), &(m_Model->listVertex.front()), GL_STATIC_DRAW);
            glEnableVertexArrayAttrib(VA_Main, 0);
            glVertexArrayAttribFormat(VA_Main, 0, 3, GL_FLOAT, GL_FALSE, 0);
            glVertexArrayVertexBuffer(VA_Main, 0, VBO_Vertex, 0, sizeof(glm::vec3));
            glVertexArrayAttribBinding(VA_Main, 0, 0);
        }

        if (m_Model->listNormals.size() > 0) {
            glCreateBuffers(1, &VBO_Normals);
            glNamedBufferData(VBO_Normals, m_Model->nb_vertex * sizeof(glm::vec3), &(m_Model->listNormals.front()), GL_STATIC_DRAW_ARB);
            glEnableVertexArrayAttrib(VA_Main, 2);
            glVertexArrayAttribFormat(VA_Main, 2, 3, GL_FLOAT, GL_TRUE, 0);
            glVertexArrayVertexBuffer(VA_Main, 2, VBO_Normals, 0, sizeof(glm::vec3));
            glVertexArrayAttribBinding(VA_Main, 2, 2);
        }
        if (m_Model->listCoords.size() != 0) {

            glCreateBuffers(1, &VBO_TexCoords);
            glNamedBufferData(VBO_TexCoords, m_Model->nb_vertex * sizeof(glm::vec3), &(m_Model->listCoords.front()), GL_STATIC_DRAW_ARB);
            glEnableVertexArrayAttrib(VA_Main, 3);
            glVertexArrayAttribFormat(VA_Main, 3, 3, GL_FLOAT, GL_FALSE, 0);
            glVertexArrayVertexBuffer(VA_Main, 3, VBO_TexCoords, 0, sizeof(glm::vec3));
            glVertexArrayAttribBinding(VA_Main, 3, 3);
        }

        if (m_Model->listTangents.size() > 0) {
            glCreateBuffers(1, &VBO_Tangents);
            glNamedBufferData(VBO_Tangents, m_Model->nb_vertex * sizeof(glm::vec4), &(m_Model->listTangents.front()), GL_STATIC_DRAW_ARB);
            glEnableVertexArrayAttrib(VA_Main, 4);
            glVertexArrayAttribFormat(VA_Main, 4, 4, GL_FLOAT, GL_FALSE, 0);
            glVertexArrayVertexBuffer(VA_Main, 4, VBO_Tangents, 0, sizeof(glm::vec4));
            glVertexArrayAttribBinding(VA_Main, 4, 4);
        }

        if (m_Model->listFaces.size() > 0) {
            glCreateBuffers(1, &VBO_Faces);
            glNamedBufferData(VBO_Faces, m_Model->nb_faces * sizeof(Face), &(m_Model->listFaces.front()), GL_STATIC_DRAW_ARB);
        }

        glBindVertexArray(VA_Main);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, VBO_Faces);

        glBindVertexArray(0);
    }
}

void ModelGL::drawGeometry(GLint type) {
    if (m_Model != NULL) {
        glBindVertexArray(VA_Main);

        glDrawRangeElements(type, 0, 3 * m_Model->nb_faces, 3 * m_Model->nb_faces, GL_UNSIGNED_INT, 0);

        glBindVertexArray(0);
    }
}

void ModelGL::displayInterface() {
    m_Model->displayInterface();
}

void ModelGL::intersectionTriangleRayon(
    const Ray& ray,
    const glm::vec3 &vertex1,
    const glm::vec3 &vertex2,
    const glm::vec3 &vertex3,
    IntersectionData& intersection,
    const NodeMaterialProperties &materialProperties) {
    constexpr float rayEpsilon = 1e-8f;

    const glm::vec3 edge1 = vertex2 - vertex1;
    const glm::vec3 edge2 = vertex3 - vertex1;
    const glm::vec3 pvec = glm::cross(ray.direction, edge2);
    const float det = glm::dot(edge1, pvec);

    // Done: Vous devez implémenter l'intersection d'un rayon avec un triangle défini par les trois sommets vertex1, vertex2 et vertex3. Utiliser rayEpsilon pour éviter les problèmes de précision numérique. 
    // Si une intersection est trouvée, vous devez mettre à jour les champs de la structure intersection avec les informations de l'intersection (position, normale, albedo, etc.) et utiliser les propriétés du matériau fournies dans materialProperties.
    // Vous pouvez utiliser la fonction ray.computePoint(t, intersection.p) pour calculer la position de l'intersection à partir du paramètre t du rayon.
     // Rayon parallèle au triangle
    if (fabs(det) < rayEpsilon)
        return;

    float invDet = 1.0f / det;

    const glm::vec3 tvec = ray.origin - vertex1;
    float u = glm::dot(tvec, pvec) * invDet;

    if (u < 0.0f || u > 1.0f)
        return;

    const glm::vec3 qvec = glm::cross(tvec, edge1);
    float v = glm::dot(ray.direction, qvec) * invDet;

    if (v < 0.0f || (u + v) > 1.0f)
        return;

    float t = glm::dot(edge2, qvec) * invDet;

    // Intersection derrière ou trop proche
    if (t < rayEpsilon)
        return;

    // Garder la plus proche
    if (t < intersection.t) {

        intersection.t = t;

        // Position
        ray.computePoint(t, intersection.p);

        // Normale
        glm::vec3 normal = glm::normalize(glm::cross(edge1, edge2));
        intersection.n = normal;

        // Matériau
        intersection.albedo = materialProperties.albedo;
        intersection.diffuse = materialProperties.diffuse;
        intersection.specular = materialProperties.specular;
        intersection.hardness = materialProperties.hardness;
    }
}

void ModelGL::intersect(
    const Ray& ray,
    IntersectionData& intersection,
    const NodeMaterialProperties &materialProperties)
{
	glm::vec3 vertex1 = glm::vec3();
	glm::vec3 vertex2 = glm::vec3();
	glm::vec3 vertex3 = glm::vec3();

    // Nous itérons en force brute sur tous les triangles du modèle, en testant l'intersection avec chacun d'eux.
    // Peut-être que vous devriez considérer comment il serait possible d'accélérer cette étape par exemple avec le résultat d'une recherche spatiale.
	for (int i = 0; i < m_Model->nb_faces; i++) 
	{
        vertex1 = m_Model->listVertex[m_Model->listFaces[i].s1];
        vertex2 = m_Model->listVertex[m_Model->listFaces[i].s2];
        vertex3 = m_Model->listVertex[m_Model->listFaces[i].s3];

        intersectionTriangleRayon(ray, vertex1, vertex2, vertex3, intersection, materialProperties);
	}
}