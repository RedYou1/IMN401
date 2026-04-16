#include "Primitive3DManager.h"
#include "Scene.h"
#include <imgui.h>
#include "SpherePrimitive.h"
#include "CubePrimitive.h"
#include "./Materials/BaseMaterial/BaseMaterial.h"

void Primitive3DManager::displayInterface()
{
    ImGuiIO& io = ImGui::GetIO();
    ImVec2 displaySize = io.DisplaySize;

    // taille initiale plus grande
    ImVec2 windowSize(350.0f, 250.0f);

    // position en bas à droite
    ImVec2 windowPos(
        displaySize.x - windowSize.x - 10.0f,
        displaySize.y - windowSize.y - 10.0f
    );

    ImGui::SetNextWindowSize(windowSize);
    ImGui::SetNextWindowPos(windowPos);

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
        Scene::getInstance()->getSceneNode()->adopt(n);
        m_onCreateNode();
        n->frame()->scale(glm::vec3(2.0, 2.0, 2.0));
    }

    ImGui::End();
}

Node* Primitive3DManager::createPrimitiveNode()
{
    Node* node = Scene::getInstance()->getNode("Primitive" + std::to_string(primcount++));

    glm::vec3 color(m_color[0], m_color[1], m_color[2]);
    node->materialProperties.albedo = color;
    node->materialProperties.diffuse = 1.0f;
    node->materialProperties.specular = 0.2f;
    node->materialProperties.hardness = 16.0f;

    BaseMaterial* mat = new BaseMaterial("PrimitiveMat");
    mat->enableTexture(false);
    node->setMaterial(mat);

    switch (m_selectedType)
    {
    case PrimitiveType::Cube:
        node->setModel(new CubePrimitive());
        break;

    case PrimitiveType::Sphere:
        node->setModel(new SpherePrimitive(24, 24));
        break;
    }

    return node;
}