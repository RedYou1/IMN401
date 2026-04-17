#include "Scene.h"
#include "EffectGL.h"
#include "EngineGL.h"

Scene::Scene() {

    LOG_TRACE << "Creating Scene" << std::endl;
    // Get the root Node
    m_Root = m_Nodes.get("_Root");

    // Create a projective default camera with standard parameter and place it in the scene

    Camera *camera = new Camera("DefaultCamera");
    camera->setPerspectiveProjection(glm::radians(45.0f), 1.77777f, 0.01f, 2000.0f);
    camera->lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 10.0f, 10.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    camera->attachTo(m_Root->frame());
    current_Camera = camera;

    // Create a Scene Node and link it to root node
    m_Scene = m_Nodes.get("_Scene");
    m_Root->adopt(m_Scene);

    current_ManipulatedNode = m_Scene;
}

Scene::~Scene() {
    // m_Nodes.release("_Root");
}


Camera *Scene::camera() {
    return current_Camera;
}

void Scene::setCamera(Camera *camera) {
    current_Camera = camera;
}

void Scene::nextManipulatedNode() {
    current_ManipulatedNode->isManipulated = false;
    current_ManipulatedNode = m_Nodes.nextObject(current_ManipulatedNode->getName());
    if (current_ManipulatedNode == m_Root)
        current_ManipulatedNode = m_Nodes.nextObject(current_ManipulatedNode->getName());
    current_ManipulatedNode->isManipulated = true;
    LOG_INFO << "manipulating Node " << current_ManipulatedNode->getName() << std::endl;
}
void Scene::manipulateNode(std::string name) {
    Node *c_node = m_Nodes.find(name);
    if (c_node == NULL)
        LOG_WARNING << "Error : Node " << name << " does not exists." << std::endl;
    else {
        if (current_ManipulatedNode != NULL)
            current_ManipulatedNode->isManipulated = false;
        current_ManipulatedNode = c_node;
        c_node->isManipulated = true;
        LOG_INFO << "Manipulated node is now " << current_ManipulatedNode->getName() << std::endl;
    }
}

Node *Scene::getSceneNode() {
    return m_Scene;
}
Node *Scene::getManipulatedNode() {
    return current_ManipulatedNode;
}
Frame *Scene::frame() {
    return m_Root->frame();
}
Node *Scene::getRoot() {
    return m_Root;
}
Node *Scene::getNode(std::string name) {
    return (m_Nodes.get(name));
}

void Scene::releaseNode(Node *n) {
    m_Nodes.release(n->getName());
}

void Scene::releaseNode(std::string name) {
    m_Nodes.release(name);
}

void Scene::releaseModel(std::string a) {
    m_Models.release(a);
}

void Scene::releaseModel(ModelGL *m) {
    m_Models.release(m->getName());
}

void Scene::resizeViewport(int w, int h) {
    m_width = w;
    m_height = h;
}
Node* Scene::findNode(std::string name) {
    return m_Nodes.find(name);
}
void Scene::displayInterface(void* engine) {
    if (ImGui::Begin("Manipulated Nodes")) {
        if(current_ManipulatedNode != nullptr && current_ManipulatedNode->getName().at(0) != '_' && ImGui::Button("Delete Selected")) {
            getSceneNode()->disown(current_ManipulatedNode);
            ((EngineGL*)engine)->refreshNodeCollector();
            releaseNode(current_ManipulatedNode);
            current_ManipulatedNode->setDelete();
            current_ManipulatedNode = nullptr;
        }

        for (int i = 0; i < m_Nodes.size(); i++) {
            Node *n = m_Nodes.get(i);
            if(n->deleted()) continue;

            if (ImGui::RadioButton(n->getName().c_str(), current_ManipulatedNode == n)) {
                manipulateNode(n->getName());
            }
        }

        ImGui::End();
    }

    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("Scene Information")) {
            if (ImGui::BeginMenu("Nodes")) {
                for (int i = 0; i < m_Nodes.size(); i++){
                    Node* node = m_Nodes.get(i);
                    if(node->deleted()) continue;
                    ImGui::MenuItem(node->getName().c_str(), NULL, &(node->show_interface));
                }
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Effects")) {
                for (int i = 0; i < m_Effects.size(); i++)
                    ImGui::MenuItem(m_Effects.get(i)->getName().c_str(), NULL, &(m_Effects.get(i)->show_interface));
                ImGui::EndMenu();
            }

            ImGui::EndMenu();
        }

        for (int i = 0; i < m_Nodes.size(); i++){
            Node* node = m_Nodes.get(i);
            if(node->deleted()) continue;
            if (node->show_interface)
                node->displayInterface();
        }

        for (int i = 0; i < m_Effects.size(); i++)
            if (m_Effects.get(i)->show_interface)
                m_Effects.get(i)->displayInterface();

        ImGui::EndMainMenuBar();
    }
}
