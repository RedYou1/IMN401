#include "Ray.h"

Ray::Ray(glm::vec3 _origin, glm::vec3 _direction) :
    origin(_origin),
    direction(_direction)
{
}

Ray::~Ray()
{
}

void Ray::computePoint(float t, glm::vec3& p) const
{
    //Done: compute the point p at distance t along the ray
    p = origin + t * direction;
}
