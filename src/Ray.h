#pragma once
#ifndef RAY_H
#define RAY_H

#define _USE_MATH_DEFINES
#include <math.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class Ray
{
public:
	Ray(glm::vec3 _origin, glm::vec3 _direction);
	Ray() {}
	virtual ~Ray();

	void computePoint(float t, glm::vec3& p) const;

	glm::vec3 origin = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 direction = glm::vec3(0.0f, 0.0f, 1.0f);
};

#endif
