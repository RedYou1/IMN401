#include "Sphere.h"
#include "Ray.h"
#include "IntersectionData.h"

float SPHERE_EPSILON = 1e-4f; // To prevent shadow acne

Sphere::Sphere() :
    radius(1.0f),
    center(glm::vec3(0.0f, 0.0f, 0.0f))
{
}

Sphere::Sphere(float _radius, glm::vec3 _center) :
    radius(_radius),
    center(_center)
{
}

Sphere::Sphere(float _radius) :
    radius(_radius),
    center(glm::vec3(0.0f, 0.0f, 0.0f))
{
}

Sphere::~Sphere()
{
}

void Sphere::intersect(const Ray& ray, IntersectionData& intersection, const NodeMaterialProperties &materialProperties)
{
    //TODO: implémenter l'intersection d'un rayon avec une sphère et remplir les données d'intersection
    glm::vec3 OC = ray.origin - center;

    float a = glm::dot(ray.direction, ray.direction);
    float b = 2.0f * glm::dot(OC, ray.direction);
    float c = glm::dot(OC, OC) - radius * radius;

    float discriminant = b * b - 4 * a * c;

    // pas d'intersection
    if (discriminant < 0.0f)
        return;

    float sqrtDisc = sqrt(discriminant);

    float t1 = (-b - sqrtDisc) / (2.0f * a);
    float t2 = (-b + sqrtDisc) / (2.0f * a);

    // prendre le plus petit t positif
    float t = t1;
    if (t < 0.0f)
        t = t2;

    if (t < 0.0f)
        return;

    // vérifier si c'est plus proche que ce qu'on avait
    if (t < intersection.t) {

        intersection.t = t;

        // position d'intersection
        intersection.p = ray.origin + t * ray.direction;

        // normale
        intersection.n = glm::normalize(intersection.p - center);

        // matériel
        intersection.albedo = materialProperties.albedo;
        intersection.diffuse = materialProperties.diffuse;
        intersection.specular = materialProperties.specular;
        intersection.hardness = materialProperties.hardness;
    }
}