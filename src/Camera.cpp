#include "Camera.h"
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>

Camera::Camera(std::string name) {

    Znear = 0.01f;
    Zfar = 10000.0f;
    aspectRatio = 1.0f;
    foV = 45.0f;

    projectionType = PERSPECTIVE;

    orthoLeft = -7.0f;
    orthoRight = 7.0f;
    orthoBottom = -7.0f;
    orthoTop = 7.0f;

    m_Frame = new Frame();
    m_Frame->setAsCameraFrame(true);

    projection_frame = new Frame();
    projection_frame->setAsCameraFrame(true);

    m_Name = name;
    needUpdate = true;
}

Camera::~Camera() {
    if (m_Frame != NULL)
        delete m_Frame;
    if (projection_frame != NULL)
        delete projection_frame;
}

const std::string Camera::getName() {
    return m_Name;
}

void Camera::attachTo(Frame* f) {
    m_Frame->attachTo(f);
}

//*********************** Camera position and orientation transformation**********************//

void Camera::lookAt(glm::vec3 pointTo, glm::vec3 center, glm::vec3 up) {
    m_Frame->setUpFromMatrix(glm::inverse(glm::lookAt(center, pointTo, up)));
    setUpdate(true);
}

//*********************** Camera projection transformation **********************//

void Camera::setProjectionMatrix(glm::mat4& m) {
    projection_frame->setUpFromMatrix(m);
    setUpdate(true);
}

//*********************** Define a perspective projection *********************//

glm::mat4 Camera::computeFrustum(float left, float right, float bottom, float top, float near, float far) {

    glm::mat4 m(0.0f);

    float rl = right - left;
    float tb = top - bottom;
    float fn = far - near;

    m[0][0] = (2.0f * near) / rl;
    m[1][1] = (2.0f * near) / tb;

    m[2][0] = (right + left) / rl;
    m[2][1] = (top + bottom) / tb;
    m[2][2] = -(far + near) / fn;
    m[2][3] = -1.0f;

    m[3][2] = -(2.0f * far * near) / fn;

    return m;
}

glm::mat4 Camera::computePerspectiveProjection(float foV, float aspectRatio, float near, float far) {

    float half = tan(foV * 0.5f) * near;

    float right = half * aspectRatio;
    float left = -right;
    float top = half;
    float bottom = -top;

    return computeFrustum(left, right, bottom, top, near, far);
}

void Camera::setPerspectiveProjection(float foV, float aspectRatio, float near, float far) {

    projectionType = PERSPECTIVE;

    this->foV = foV;
    this->aspectRatio = aspectRatio;
    this->Znear = near;
    this->Zfar = far;

    glm::mat4 projmat = computePerspectiveProjection(foV, aspectRatio, near, far);

    projection_frame->setUpFromMatrix(projmat);
    setUpdate(true);
}

void Camera::setFrustum(float left, float right, float bottom, float top, float near, float far) {

    projectionType = PERSPECTIVE;

    this->Znear = near;
    this->Zfar = far;

    glm::mat4 projmat = computeFrustum(left, right, bottom, top, near, far);

    projection_frame->setUpFromMatrix(projmat);
    setUpdate(true);
}

void Camera::setOrthographicProjection(float left, float right, float bottom, float top, float near, float far) {

    projectionType = ORTHOGRAPHIC;

    orthoLeft = left;
    orthoRight = right;
    orthoBottom = bottom;
    orthoTop = top;

    Znear = near;
    Zfar = far;

    glm::mat4 projmat = glm::ortho(left, right, bottom, top, near, far);

    projection_frame->setUpFromMatrix(projmat);
    setUpdate(true);
}

void Camera::setProjectionType(ProjectionType type) {

    if (type == PERSPECTIVE) {
        setPerspectiveProjection(foV, aspectRatio, Znear, Zfar);
    }
    else {
        setOrthographicProjection(orthoLeft, orthoRight, orthoBottom, orthoTop, Znear, Zfar);
    }
}

// *********************** Members accessors ***********************//

float Camera::getZnear() {
    return Znear;
}

float Camera::getZfar() {
    return Zfar;
}

void Camera::setZnear(float n) {
    Znear = n;

    if (projectionType == PERSPECTIVE)
        setPerspectiveProjection(foV, aspectRatio, Znear, Zfar);
    else
        setOrthographicProjection(orthoLeft, orthoRight, orthoBottom, orthoTop, Znear, Zfar);
}

void Camera::setZfar(float f) {
    Zfar = f;

    if (projectionType == PERSPECTIVE)
        setPerspectiveProjection(foV, aspectRatio, Znear, Zfar);
    else
        setOrthographicProjection(orthoLeft, orthoRight, orthoBottom, orthoTop, Znear, Zfar);
}

void Camera::setProjectionMode(ProjectionType type)
{
    projectionType = type;

    if (projectionType == PERSPECTIVE)
    {
        setPerspectiveProjection(foV, aspectRatio, Znear, Zfar);
    }
    else // ORTHOGRAPHIC
    {
        setOrthographicProjection(
            orthoLeft,
            orthoRight,
            orthoBottom,
            orthoTop,
            Znear,
            Zfar
        );
    }

    setUpdate(true);
}

float Camera::getFoV() {
    return foV;
}

void Camera::setFoV(float v) {
    foV = v;

    if (projectionType == PERSPECTIVE)
        setPerspectiveProjection(foV, aspectRatio, Znear, Zfar);
}

float Camera::getAspectRatio() {
    return aspectRatio;
}

void Camera::setAspectRatio(float a) {
    aspectRatio = a;

    if (projectionType == PERSPECTIVE)
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

void Camera::setUpFromMatrix(glm::mat4& m) {
    m_Frame->setUpFromMatrix(m);
    setUpdate(true);
}

bool Camera::updateNeeded() {
    return (m_Frame->updateNeeded() || projection_frame->updateNeeded());
}

void Camera::setUpdate(bool r) {
    m_Frame->setUpdate(r);
    projection_frame->setUpdate(r);
    needUpdate = true;
}

Camera::ProjectionType Camera::GetTypeProjection() {
    return projectionType;
}

void Camera::updateBuffer() {}