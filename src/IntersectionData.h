#pragma once
#ifndef INTERSECTIONDATA_H
#define INTERSECTIONDATA_H

#define _USE_MATH_DEFINES
#include <math.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

/**
 * Convenient class to hold information of the closest intersection for a given ray. 
 */
class IntersectionData
{
public:
	IntersectionData() {}
	virtual ~IntersectionData() {}

	void reset()
	{
		n = glm::vec3(0.0f, 0.0f, 0.0f);
		p = glm::vec3(0.0f, 0.0f, 0.0f);
		albedo = glm::vec3(1.0f, 1.0f, 1.0f);
		diffuse = 0.0f;
		specular = 0.0f;
		hardness = 0.0f;
		t = FLT_MAX;
	}

	glm::vec3 n = glm::vec3(0.0f, 0.0f, 0.0f); // Intersection normal
	glm::vec3 p = glm::vec3(0.0f, 0.0f, 0.0f); // Intersection point
	glm::vec3 albedo = glm::vec3(1.0f, 1.0f, 1.0f); // Base color of the material at the intersection point
	float diffuse = 0.0f; // Diffuse coefficient of the material at the intersection point
	float specular = 0.0f; // Specular coefficient of the material at the intersection point
	float hardness = 0.0f; // Hardness coefficient of the material at the intersection point
	float t = FLT_MAX; // Ray parameter giving the position of intersection
};

struct NodeMaterialProperties {
	glm::vec3 albedo = glm::vec3(1.0f, 1.0f, 1.0f);
	float diffuse = 0.5f;
	float specular = 64.0f;
	float hardness = 32.0f;
	float shininess = 0.5f;
	bool useBlinnPhong = false;
	bool useGouraud = false;
};

#endif
