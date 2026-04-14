/*
 *	(c) XLim, UMR-CNRS
 *	Authors: G.Gilet, A. Mercier-Aubin
 */
#include "Camera.h"
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>

Camera::Camera(std::string name) {
    Znear = 0.01f;
    Zfar = 10000.0f;
    aspectRatio = 1.0f;
    foV = 45.0;
    m_Frame = new Frame();
    m_Frame->setAsCameraFrame(true);
    projection_frame = new Frame();
    projection_frame->setAsCameraFrame(true);
    m_Name = name;
    needUpdate = true;
}
const std::string Camera::getName() {
    return m_Name;
}
Camera::~Camera() {
    if (m_Frame != NULL)
        delete m_Frame;
    if (projection_frame != NULL)
        delete projection_frame;
}

glm::mat4 Camera::computeFrustum(float left, float right, float bottom, float top, float near, float far) {
    glm::mat4 m(0.0f);

    const float rl = right - left;
    const float tb = top - bottom;
    const float fn = far - near;

    m[0][0] = (2.0f * near) / rl;
    m[1][1] = (2.0f * near) / tb;
    m[2][0] = (right + left) / rl;
    m[2][1] = (top + bottom) / tb;
    m[2][2] = -(far + near) / fn;
    m[2][3] = -1.0f;
    m[3][2] = -(2.0f * far * near) / fn;

    return m;
}

void Camera::setFrustum(float left, float right, float bottom, float top, float near, float far) {
    this->Znear = near;
    this->Zfar = far;

    glm::mat4 pmat = computeFrustum(left, right, bottom, top, near, far);
    projection_frame->setUpFromMatrix(pmat);
    setUpdate(true);
}

glm::mat4 Camera::computePerspectiveProjection(float foV, float aspectRatio, float near, float far) {
    const float half = std::tan(foV * 0.5f) * near;
    const float right = half * aspectRatio;
    const float left = -right;
    const float top = half;
    const float bottom = -top;

    return computeFrustum(left, right, bottom, top, near, far);
}

void Camera::setPerspectiveProjection(float foV, float aspectRatio, float near, float far) {
    this->foV = foV;
    this->aspectRatio = aspectRatio;
    this->Znear = near;
    this->Zfar = far;

    glm::mat4 projmat = computePerspectiveProjection(foV, aspectRatio, near, far);

    projection_frame->setUpFromMatrix(projmat);

    setUpdate(true);
}
void Camera::lookAt(glm::vec3 pointTo, glm::vec3 center, glm::vec3 up) {
    m_Frame->setUpFromMatrix(glm::inverse(glm::lookAt(center, pointTo, up)));

    setUpdate(true);
}

void Camera::setProjectionMatrix(glm::mat4 &m) {
    projection_frame->setUpFromMatrix(m);
}

void Camera::setUpFromMatrix(glm::mat4 &m) {
    m_Frame->setUpFromMatrix(m);
}

float Camera::getZnear() {
    return (Znear);
}

void Camera::setZnear(float n) {
    this->Znear = n;
    setPerspectiveProjection(foV, aspectRatio, Znear, Zfar);
}

float Camera::getZfar() {
    return (Zfar);
}

void Camera::setZfar(float f) {
    this->Zfar = f;
    setPerspectiveProjection(foV, aspectRatio, Znear, Zfar);
}

float Camera::getFoV() {
    return (foV);
}

void Camera::setFoV(float v) {
    this->foV = v;
    setPerspectiveProjection(foV, aspectRatio, Znear, Zfar);
}

float Camera::getAspectRatio() {
    return (aspectRatio);
}

void Camera::setAspectRatio(float a) {
    this->aspectRatio = a;
    setPerspectiveProjection(foV, aspectRatio, Znear, Zfar);
}

glm::mat4 Camera::getProjectionMatrix() {
    return *(projection_frame->getMatrix());
}

glm::mat4 Camera::getViewMatrix() {
    if (m_Frame->updateNeeded())
        viewMatrix = glm::inverse(m_Frame->getModelMatrix());

    return viewMatrix;
}

bool Camera::updateNeeded() {
    return (m_Frame->updateNeeded() || projection_frame->updateNeeded());
}

void Camera::setUpdate(bool r) {
    m_Frame->setUpdate(r);
    projection_frame->setUpdate(r);
    needUpdate = true;
}

void Camera::attachTo(Frame *f) {
    m_Frame->attachTo(f);
}

void Camera::updateBuffer() {}
