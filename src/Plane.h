#pragma once
#ifndef PLANE_H
#define PLANE_H

#include "IntersectionData.h"
#include "Ray.h"

/**
A plane object. Requires a normal and a point on the plane to be defined. 
*/
class Plane
{
public:
	Plane();
	Plane(glm::vec3 _normal, glm::vec3 _position);
	Plane(glm::vec3 _normal);

	virtual ~Plane();

	void intersect(const Ray& ray, IntersectionData& intersection, const NodeMaterialProperties &materialProperties);

	glm::vec3 normal;
	glm::vec3 position;
};

#endif
