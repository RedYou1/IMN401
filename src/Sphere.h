#pragma once
#ifndef SPHERE_H
#define SPHERE_H

#include "IntersectionData.h"
#include "Ray.h"

class Sphere
{
public:
	Sphere();
	Sphere(float _radius, glm::vec3 _center);
	Sphere(float _radius);

	virtual ~Sphere();

	void intersect(const Ray& ray, IntersectionData& intersection, const NodeMaterialProperties &materialProperties);

	float radius;
	glm::vec3 center;
};

#endif
