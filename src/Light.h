#pragma once
#ifndef LIGHT_H
#define LIGHT_H

#define _USE_MATH_DEFINES
#include <math.h>

#include <string>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "Resource_mgr.hpp"
#include "GLProgram.h"

class Light
{
public:
	enum LightType { DIRECTIONNELLE, PONCTUELLE, PROJECTEUR };
	Light() :
		couleurAmbiante(0.2f, 0.2f, 0.2f),
		couleurDiffuse(1.0f, 1.0f, 1.0f),
		couleurSpeculaire(1.0f, 1.0f, 1.0f),
		position(0.0f, 0.0f, 0.0f),
		puissance(1.0f),
		type(PONCTUELLE),
		direction(0.0f, -1.0f, 0.0f),
		attenuation(0.01f),
		angleCone(45.0f)
		{}
	virtual ~Light() {}

	int type;
	glm::vec3 couleurAmbiante;
    glm::vec3 couleurDiffuse;
    glm::vec3 couleurSpeculaire;
	float puissance;
	glm::vec3 position;
    glm::vec3 direction;
	float attenuation;
    float angleCone;

	static void uniformLight( Resource_mgr<Light>& lights, GLProgram* shader) {
		uniformLight(lights, shader, nullptr);
	}
	
	static void uniformLight( Resource_mgr<Light>& lights, GLProgram* fp, GLProgram* vp) {
		int numLights = lights.size();
		
		// Set uniforms in fragment shader
		const GLuint fpId = fp->getId();
		glProgramUniform1i(fpId, glGetUniformLocation(fpId, "lightArraySize"), numLights);
		for(int k = 0; k < numLights; k++) {
			const Light& light = *lights.get(k);
			std::string prefix = "lightArray[" + std::to_string(k) + "].";
			glProgramUniform1i(fpId, glGetUniformLocation(fpId, (prefix + "type").c_str()), light.type);
			glProgramUniform3fv(fpId, glGetUniformLocation(fpId, (prefix + "couleurAmbiante").c_str()), 1, glm::value_ptr(light.couleurAmbiante));
			glProgramUniform3fv(fpId, glGetUniformLocation(fpId, (prefix + "couleurDiffuse").c_str()), 1, glm::value_ptr(light.couleurDiffuse));
			glProgramUniform3fv(fpId, glGetUniformLocation(fpId, (prefix + "couleurSpeculaire").c_str()), 1, glm::value_ptr(light.couleurSpeculaire));
			glProgramUniform1f(fpId, glGetUniformLocation(fpId, (prefix + "puissance").c_str()), light.puissance);
			glProgramUniform3fv(fpId, glGetUniformLocation(fpId, (prefix + "position").c_str()), 1, glm::value_ptr(light.position));
			glProgramUniform3fv(fpId, glGetUniformLocation(fpId, (prefix + "direction").c_str()), 1, glm::value_ptr(light.direction));
			glProgramUniform1f(fpId, glGetUniformLocation(fpId, (prefix + "attenuation").c_str()), light.attenuation);
			glProgramUniform1f(fpId, glGetUniformLocation(fpId, (prefix + "angleCone").c_str()), light.angleCone);
		}
		
		// Set uniforms in vertex shader (for Gouraud)
		if (vp != nullptr) {
			const GLuint vpId = vp->getId();
			glProgramUniform1i(vpId, glGetUniformLocation(vpId, "lightArraySize"), numLights);
			for(int k = 0; k < numLights; k++) {
				const Light& light = *lights.get(k);
				std::string prefix = "lightArray[" + std::to_string(k) + "].";
				glProgramUniform1i(vpId, glGetUniformLocation(vpId, (prefix + "type").c_str()), light.type);
				glProgramUniform3fv(vpId, glGetUniformLocation(vpId, (prefix + "couleurAmbiante").c_str()), 1, glm::value_ptr(light.couleurAmbiante));
				glProgramUniform3fv(vpId, glGetUniformLocation(vpId, (prefix + "couleurDiffuse").c_str()), 1, glm::value_ptr(light.couleurDiffuse));
				glProgramUniform3fv(vpId, glGetUniformLocation(vpId, (prefix + "couleurSpeculaire").c_str()), 1, glm::value_ptr(light.couleurSpeculaire));
				glProgramUniform1f(vpId, glGetUniformLocation(vpId, (prefix + "puissance").c_str()), light.puissance);
				glProgramUniform3fv(vpId, glGetUniformLocation(vpId, (prefix + "position").c_str()), 1, glm::value_ptr(light.position));
				glProgramUniform3fv(vpId, glGetUniformLocation(vpId, (prefix + "direction").c_str()), 1, glm::value_ptr(light.direction));
				glProgramUniform1f(vpId, glGetUniformLocation(vpId, (prefix + "attenuation").c_str()), light.attenuation);
				glProgramUniform1f(vpId, glGetUniformLocation(vpId, (prefix + "angleCone").c_str()), light.angleCone);
			}
		}
	}
};

#endif
