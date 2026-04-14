#include "Plane.h"
#include "Ray.h"
#include "IntersectionData.h"

float PLANE_EPSILON = 1e-4f; // To prevent shadow acne

Plane::Plane() :
    normal(glm::vec3(0.0f, 1.0f, 0.0f)),
    position(glm::vec3(0.0f, 0.0f, 0.0f))
{
}

Plane::Plane(glm::vec3 _normal, glm::vec3 _position) :
    normal(_normal),
    position(_position)
{
}

Plane::Plane(glm::vec3 _normal) :
    normal(_normal), position(0.0f, 0.0f, 0.0f)
{
}

Plane::~Plane()
{
}

void Plane::intersect(
    const Ray& ray,
    IntersectionData& intersection,
    const NodeMaterialProperties &materialProperties)
{
    //Done: calculer l'intersection du rayon avec le plan et remplir les données d'intersection
    float denom = glm::dot(ray.direction, normal);

    // Rayon parallèle au plan
    if (fabs(denom) < PLANE_EPSILON)
        return;

    float t = glm::dot(position - ray.origin, normal) / denom;

    // Intersection derrière la caméra ou trop proche
    if (t < PLANE_EPSILON)
        return;

    // Garder la plus proche intersection
    if (t < intersection.t) {

        intersection.t = t;

        // Position d'intersection
        intersection.p = ray.origin + t * ray.direction;

        // Normale (toujours normalisée)
        intersection.n = glm::normalize(normal);

        // Matériau
        intersection.albedo = materialProperties.albedo;
        intersection.diffuse = materialProperties.diffuse;
        intersection.specular = materialProperties.specular;
        intersection.hardness = materialProperties.hardness;
    }
}