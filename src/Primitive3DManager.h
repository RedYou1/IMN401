#pragma once

#include <glm/glm.hpp>
#include "Node.h"

class Primitive3DManager {
public:
    void displayInterface();

private:
    enum class PrimitiveType {
        Cube = 0,
        Sphere = 1
    };

private:
    PrimitiveType m_selectedType = PrimitiveType::Cube;

    float m_color[3] = { 0.8f, 0.8f, 0.8f };

private:
    Node* createPrimitiveNode();
};