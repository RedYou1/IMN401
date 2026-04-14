#include "VectorPrimitive.h"
#include "Frame.h"
#include "Scene.h"
#include <glad/glad.h>
#include <imgui/imgui.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>  
#include <cmath>
#include <iostream>

// Simple shader
static GLuint primitiveShader = 0;
static GLint locColor = -1;
static GLint locMVP = -1;

static void initPrimitiveShader()
{
    if (primitiveShader != 0) return;

    const char* vsSrc = R"(
        #version 460 core
        layout(location = 0) in vec3 aPos;
        uniform mat4 MVP;
        void main() {
            gl_Position = MVP * vec4(aPos, 1.0);
        }
    )";

    const char* fsSrc = R"(
        #version 460 core
        out vec4 FragColor;
        uniform vec4 uColor;
        void main() {
            FragColor = uColor;
        }
    )";

    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &vsSrc, nullptr);
    glCompileShader(vs);

    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &fsSrc, nullptr);
    glCompileShader(fs);

    primitiveShader = glCreateProgram();
    glAttachShader(primitiveShader, vs);
    glAttachShader(primitiveShader, fs);
    glLinkProgram(primitiveShader);

    glDeleteShader(vs);
    glDeleteShader(fs);

    locMVP = glGetUniformLocation(primitiveShader, "MVP");
    locColor = glGetUniformLocation(primitiveShader, "uColor");
}

VectorPrimitive::VectorPrimitive(Type type, std::string name)
    : Node(name.empty() ? "VectorPrim" : name), m_type(type)
{
    show_interface = true;
    m_Name = name.empty() ? "VectorPrim_" + std::to_string(rand() % 10000) : name;
    initPrimitiveShader();
}

VectorPrimitive::~VectorPrimitive()
{
    if (m_VAO) glDeleteVertexArrays(1, &m_VAO);
    if (m_VBO) glDeleteBuffers(1, &m_VBO);
}

void VectorPrimitive::setStrokeColor(const glm::vec4& color) { m_strokeColor = color; }
void VectorPrimitive::setFillColor(const glm::vec4& color)   { m_fillColor = color; }
void VectorPrimitive::setStrokeThickness(float thickness)    { m_thickness = std::max(1.0f, thickness); }

// ==================== CREATION - CENTER OF THE WORLD ====================

void VectorPrimitive::startCreation(glm::vec2 startPos)
{
    m_vertices.clear();
    m_uploaded = false;
}

void VectorPrimitive::updateCreation(glm::vec2 currentPos)
{
    // Only Line uses drag
    if (m_type == Type::LINE && !m_vertices.empty())
    {
        m_vertices.back() = currentPos;
        m_uploaded = false;
    }
}

void VectorPrimitive::finishCreation()
{
    glm::vec2 worldCenter = glm::vec2(0.0f, 8.0f);   // Center of the world (near bunny)

    if (m_type == Type::POINT)
    {
        m_vertices = { worldCenter };
        uploadToGPU();
    }
    else if (m_type == Type::LINE)
    {
        // Fixed length horizontal line centered
        m_vertices = { 
            worldCenter + glm::vec2(-15.0f, 0.0f),
            worldCenter + glm::vec2( 15.0f, 0.0f)
        };
        uploadToGPU();
    }
    else
    {
        // Rectangle, Triangle, Circle
        generateGeometry(worldCenter);
    }
}

void VectorPrimitive::generateGeometry(glm::vec2 basePos)
{
    m_vertices.clear();
    const float size = 28.0f;

    if (m_type == Type::RECTANGLE)
    {
        glm::vec2 half(size * 1.3f, size);
        m_vertices = {
            basePos - half,
            glm::vec2(basePos.x + half.x, basePos.y - half.y),
            basePos + half,
            basePos - half,
            basePos + half,
            glm::vec2(basePos.x - half.x, basePos.y + half.y)
        };
    }
    else if (m_type == Type::TRIANGLE)
    {
        m_vertices = {
            basePos + glm::vec2(0.0f, size),
            basePos + glm::vec2(-size * 0.866f, -size * 0.5f),
            basePos + glm::vec2(size * 0.866f, -size * 0.5f)
        };
    }
    else if (m_type == Type::CIRCLE)
    {
        const int segments = 64;
        m_vertices.push_back(basePos);
        for (int i = 0; i <= segments; ++i)
        {
            float angle = 2.0f * glm::pi<float>() * i / segments;
            m_vertices.push_back(basePos + glm::vec2(std::cos(angle), std::sin(angle)) * size);
        }
    }
    else if (m_type == Type::LINE)
    {
        m_vertices = { basePos, basePos + glm::vec2(size * 2.0f, 0.0f) };
    }
    else if (m_type == Type::POINT)
    {
        m_vertices = { basePos };
    }

    uploadToGPU();
}

// ==================== RENDER & UPLOAD ====================

void VectorPrimitive::uploadToGPU()
{
    if (m_vertices.empty()) return;

    if (!m_VAO) glCreateVertexArrays(1, &m_VAO);
    if (!m_VBO) glCreateBuffers(1, &m_VBO);

    std::vector<glm::vec3> gpuData(m_vertices.size());
    for (size_t i = 0; i < m_vertices.size(); ++i)
        gpuData[i] = glm::vec3(m_vertices[i], 0.0f);

    glNamedBufferData(m_VBO, gpuData.size() * sizeof(glm::vec3), gpuData.data(), GL_DYNAMIC_DRAW);

    glVertexArrayVertexBuffer(m_VAO, 0, m_VBO, 0, sizeof(glm::vec3));
    glEnableVertexArrayAttrib(m_VAO, 0);
    glVertexArrayAttribFormat(m_VAO, 0, 3, GL_FLOAT, GL_FALSE, 0);
    glVertexArrayAttribBinding(m_VAO, 0, 0);

    m_uploaded = true;
}

void VectorPrimitive::render()
{
    if (!m_uploaded) uploadToGPU();
    if (m_vertices.empty() || !m_VAO) return;

    glUseProgram(primitiveShader);

    glm::mat4 model = frame()->getModelMatrix();
    glm::mat4 view  = Scene::getInstance()->camera()->getViewMatrix();
    glm::mat4 proj  = Scene::getInstance()->camera()->getProjectionMatrix();
    glm::mat4 mvp   = proj * view * model;

    glUniformMatrix4fv(locMVP, 1, GL_FALSE, &mvp[0][0]);

    glBindVertexArray(m_VAO);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_DEPTH_TEST);

    if (m_type == Type::POINT)
    {
        glUniform4fv(locColor, 1, &m_strokeColor[0]);
        glPointSize(m_thickness * 4.0f);
        glDrawArrays(GL_POINTS, 0, 1);
    }
    else if (m_type == Type::LINE)
    {
        glUniform4fv(locColor, 1, &m_strokeColor[0]);
        glLineWidth(std::max(1.0f, m_thickness));
        glDrawArrays(GL_LINES, 0, 2);
    }
    else
    {
        // Fill
        glUniform4fv(locColor, 1, &m_fillColor[0]);
        if (m_type == Type::CIRCLE)
            glDrawArrays(GL_TRIANGLE_FAN, 0, (GLsizei)m_vertices.size());
        else
            glDrawArrays(GL_TRIANGLES, 0, (GLsizei)m_vertices.size());

        // Stroke
        glUniform4fv(locColor, 1, &m_strokeColor[0]);
        glLineWidth(std::max(1.0f, m_thickness));
        if (m_type == Type::CIRCLE)
            glDrawArrays(GL_LINE_LOOP, 1, (GLsizei)m_vertices.size() - 1);
        else
            glDrawArrays(GL_LINE_LOOP, 0, (GLsizei)m_vertices.size());
    }

    glBindVertexArray(0);
    glUseProgram(0);

    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
}

void VectorPrimitive::displayInterface()
{
    if (!ImGui::Begin(m_Name.c_str(), &show_interface))
    {
        ImGui::End();
        return;
    }

    ImGui::Text("Type: %s",
        m_type == Type::POINT ? "Point" :
        m_type == Type::LINE ? "Line" :
        m_type == Type::RECTANGLE ? "Rectangle" :
        m_type == Type::TRIANGLE ? "Triangle" : "Circle");

    ImGui::ColorEdit4("Fill Color", &m_fillColor[0]);
    ImGui::ColorEdit4("Stroke Color", &m_strokeColor[0]);
    ImGui::SliderFloat("Thickness", &m_thickness, 1.0f, 20.0f);

    ImGui::End();
}