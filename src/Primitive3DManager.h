#pragma once

#include <glm/glm.hpp>
#include "Node.h"
#include <functional>

class Primitive3DManager {
public:
    void displayInterface();

    void Primitive3DManager::setOnCreateNode(std::function<void()> fn)
    {
        m_onCreateNode = fn;
    }

private:
    enum class PrimitiveType {
        Cube = 0,
        Sphere = 1
    };

private:
    PrimitiveType m_selectedType = PrimitiveType::Cube;

    float m_color[3] = { 0.8f, 0.8f, 0.8f };

private:
    void applyColorToSelected();
    Node* createPrimitiveNode();
    int primcount = 1;

    std::function<void()> m_onCreateNode;
};