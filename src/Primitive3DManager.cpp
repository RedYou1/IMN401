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

    ImVec2 windowSize(350.0f, 300.0f);

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

    // --- état persistant ---
    static int mode = 0; // 0 = RGB, 1 = HSB
    static float hsv[3] = { 0.0f, 0.0f, 0.0f };
    static bool hsvInitialized = false;

    // initialiser HSV une seule fois (ou si besoin au démarrage)
    if (!hsvInitialized)
    {
        ImGui::ColorConvertRGBtoHSV(
            m_color[0], m_color[1], m_color[2],
            hsv[0], hsv[1], hsv[2]
        );
        hsvInitialized = true;
    }

    // choix du mode
    ImGui::Combo("Mode", &mode, "RGB\0HSB\0");

    // affichage selon mode
    if (mode == 0)
    {
        if (ImGui::ColorEdit3("##color", m_color))
        {
            // sync RGB -> HSV
            ImGui::ColorConvertRGBtoHSV(
                m_color[0], m_color[1], m_color[2],
                hsv[0], hsv[1], hsv[2]
            );

            applyColorToSelected();
        }
    }
    else
    {
        if (ImGui::SliderFloat3("##hsb", hsv, 0.0f, 1.0f))
        {
            // sync HSV -> RGB
            ImGui::ColorConvertHSVtoRGB(
                hsv[0], hsv[1], hsv[2],
                m_color[0], m_color[1], m_color[2]
            );

            applyColorToSelected();
        }
    }

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

void Primitive3DManager::applyColorToSelected()
{
    Node* node = Scene::getInstance()->getManipulatedNode();
    if (!node)
        return;

    const std::string& name = node->getName();

    if (name.rfind("Primitive", 0) == 0) // commence par "Primitive"
    {
        node->materialProperties.albedo = glm::vec3(
            m_color[0], m_color[1], m_color[2]
        );
    }
}