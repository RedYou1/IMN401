#include "Primitive3DManager.h"
#include "Scene.h"
#include <imgui.h>

void Primitive3DManager::displayInterface()
{
    if (!ImGui::Begin("Primitive Creator"))
    {
        ImGui::End();
        return;
    }

    const char* types[] = { "Cube", "Sphere" };

    int type = static_cast<int>(m_selectedType);
    ImGui::Combo("Type", &type, types, 2);
    m_selectedType = static_cast<PrimitiveType>(type);

    ImGui::Separator();

    ImGui::Text("Color");
    ImGui::ColorEdit3("##color", m_color);

    ImGui::Separator();

    if (ImGui::Button("Create"))
    {
        Node* n = createPrimitiveNode();
        Scene::getInstance()->getRoot()->adopt(n);
    }

    ImGui::End();
}

Node* Primitive3DManager::createPrimitiveNode()
{
    Node* node = new Node("Primitive");

    glm::vec3 color(m_color[0], m_color[1], m_color[2]);
    node->materialProperties.albedo = color;

    /*switch (m_selectedType)
    {
    case PrimitiveType::Cube:
        node->setModel(ModelFactory::CreateCube());
        break;

    case PrimitiveType::Sphere:
        node->setSphere(new Sphere(1.0f));
        break;
    }*/

    return node;
}