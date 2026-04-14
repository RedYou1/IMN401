/*
 *	(c) XLim, UMR-CNRS
 *	Authors: G.Gilet, A. Mercier-Aubin
 *
 */
#include "Frame.h"
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>
#include <algorithm>

Frame::Frame() {
    reference = NULL;
    matrix = glm::mat4(1.0f);
    isCamera = false;
}

void Frame::attachTo(Frame *f) {
    reference = f;
    if (f != NULL)
        f->m_Childs.push_back(this);
}
glm::mat4 Frame::getModelMatrix() {
    if (reference != NULL)
        return (reference->getModelMatrix() * matrix);
    else
        return (matrix);
}

glm::mat4 Frame::getMatrixCopy() {
    return (matrix);
}

Frame::~Frame() {}

void Frame::setUpFromMatrix(const glm::mat4 &m) {
    matrix = m;
    setUpdate(true);
}

void Frame::loadIdentity() {
    matrix = glm::mat4(1.0f);

    setUpdate(true);
}

// Transformations geometriques

void Frame::rotate(glm::vec3 axis, float angle) {
    glm::vec3 a = glm::normalize(axis);
    const float c = std::cos(angle);
    const float s = std::sin(angle);
    const float t = 1.0f - c;

    glm::mat4 r(1.0f);
    r[0][0] = t * a.x * a.x + c;
    r[0][1] = t * a.x * a.y + s * a.z;
    r[0][2] = t * a.x * a.z - s * a.y;

    r[1][0] = t * a.x * a.y - s * a.z;
    r[1][1] = t * a.y * a.y + c;
    r[1][2] = t * a.y * a.z + s * a.x;

    r[2][0] = t * a.x * a.z + s * a.y;
    r[2][1] = t * a.y * a.z - s * a.x;
    r[2][2] = t * a.z * a.z + c;

    matrix = matrix * r;
    setUpdate(true);
}

#include <glm/gtc/quaternion.hpp>
void Frame::rotateFromQuaternion(glm::quat t) {
    glm::mat4 rotmat = glm::mat4_cast(t);
    matrix = rotmat * matrix;
    setUpdate(true);
}

void Frame::translate(glm::vec3 t) {
    glm::mat4 tr(1.0f);
    tr[3][0] = t.x;
    tr[3][1] = t.y;
    tr[3][2] = t.z;
    matrix = matrix * tr;
    setUpdate(true);
}

void Frame::scale(glm::vec3 s) {
    glm::mat4 sc(1.0f);
    sc[0][0] = s.x;
    sc[1][1] = s.y;
    sc[2][2] = s.z;
    matrix = matrix * sc;
    setUpdate(true);
}

bool Frame::isCameraFrame() {
    return isCamera;
}

void Frame::setAsCameraFrame(bool r) {
    isCamera = r;
}

float Frame::lerp(float a, float b, float t) {
    return a + t * (b - a);
}

float Frame::angleBetweenQuaternions(glm::quat a, glm::quat b) {
    float dot = glm::dot(a, b);
    float clampedDot = std::clamp(dot, -1.0f, 1.0f);
    return std::acos(clampedDot);
}

glm::quat Frame::slerp(glm::quat a, glm::quat b, float t) {
    float theta =angleBetweenQuaternions(a, b);
    float poidP1 = glm::sin((1.0f - t) * theta) / std::sin(theta);
    float poidP2 = std::sin(t * theta) / std::sin(theta);
    return poidP1 * a + poidP2 * b;
}

glm::vec3 Frame::convertPtFrom(glm::vec3 pt, Frame *from) {
    glm::vec4 pInRoot = from->getModelMatrix() * glm::vec4(pt, 1.0);
    glm::vec4 ret = glm::inverse(getModelMatrix()) * pInRoot;
    return (glm::vec3(ret));
}

glm::vec3 Frame::convertDirFrom(glm::vec3 dir, Frame *from) {
    glm::mat4 T = glm::inverseTranspose(glm::inverse(getModelMatrix()) * from->getModelMatrix());
    glm::vec4 ret = T * glm::vec4(dir, 0.0);
    return (glm::vec3(ret));
}

glm::vec3 Frame::convertPtTo(glm::vec3 pt, Frame *to) {
    glm::vec4 pInRoot = getModelMatrix() * glm::vec4(pt, 1.0);
    glm::vec4 ret = glm::inverse(to->getModelMatrix()) * pInRoot;
    return (glm::vec3(ret));
}

glm::vec3 Frame::convertDirTo(glm::vec3 dir, Frame *to) {

    glm::mat4 T = glm::inverseTranspose(glm::inverse(to->getModelMatrix()) * getModelMatrix());
    glm::vec4 ret = T * glm::vec4(dir, 0.0);

    return (glm::vec3(ret));
}

void Frame::setUpdate(bool t) {
    m_ToUpdate = t;

    if (t && !isCameraFrame()) {
        for (int i = 0; i < m_Childs.size(); i++)
            m_Childs[i]->setUpdate(t);
    }
}

bool Frame::updateNeeded() {
    return (m_ToUpdate);
}

bool Frame::detach(Frame *f) {
    bool isInLeaves = false;
    for (int i = 0; i < f->m_Childs.size() && !isInLeaves; i++) {
        if (m_Childs[i] == f) {
            m_Childs.erase(m_Childs.begin() + i);
            return true;
        } else {
            isInLeaves = m_Childs[i]->detach(f);
        }
    }
    return isInLeaves;
}

Frame *Frame::parent() {
    return reference;
}