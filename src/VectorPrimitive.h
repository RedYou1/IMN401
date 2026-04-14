//Primitive Vectoriel
#pragma once
#ifndef VECTOR_PRIMITIVE_H
#define VECTOR_PRIMITIVE_H

#include "Node.h"
#include <glm/glm.hpp>
#include <vector>
#include <string>

class VectorPrimitive : public Node {
public:
    enum class Type {
        POINT,
        LINE,
        RECTANGLE,
        TRIANGLE,
        CIRCLE
    };

    VectorPrimitive(Type type, std::string name = "");
    virtual ~VectorPrimitive();

    void setStrokeColor(const glm::vec4& color);
    void setFillColor(const glm::vec4& color);
    void setStrokeThickness(float thickness);

    // For creation
    void startCreation(glm::vec2 startPos);
    void updateCreation(glm::vec2 currentPos);
    void finishCreation();

    virtual void render();
    virtual void displayInterface() override;

    Type getType() const { return m_type; }

protected:
    Type m_type;
    glm::vec4 m_strokeColor = glm::vec4(1.0f, 0.0f, 1.0f, 1.0f);
    glm::vec4 m_fillColor   = glm::vec4(0.8f, 0.0f, 1.0f, 0.85f);
    float m_thickness = 5.0f;

    std::vector<glm::vec2> m_vertices;   // local 2D coordinates

    GLuint m_VAO = 0;
    GLuint m_VBO = 0;
    bool m_uploaded = false;

    void uploadToGPU();
    void generateGeometry(glm::vec2 basePos); // helper for rect, circle, etc.
};

#endif