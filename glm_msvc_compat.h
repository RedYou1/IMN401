#pragma once
#ifdef __cplusplus
// On s'assure que les types GLM sont disponibles avant de les surcharger
#define GLM_FORCE_SWIZZLE
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

// Macro génératrice pour définir tous les opérateurs entre un double et un type GLM
#define GLM_MSVC_COMPAT_SCALAR_OPS(Type) \
    inline Type operator*(double s, const Type& v) { return static_cast<float>(s) * v; } \
    inline Type operator*(const Type& v, double s) { return v * static_cast<float>(s); } \
    inline Type operator/(double s, const Type& v) { return static_cast<float>(s) / v; } \
    inline Type operator/(const Type& v, double s) { return v / static_cast<float>(s); } \
    inline Type operator+(double s, const Type& v) { return static_cast<float>(s) + v; } \
    inline Type operator+(const Type& v, double s) { return v + static_cast<float>(s); } \
    inline Type operator-(double s, const Type& v) { return static_cast<float>(s) - v; } \
    inline Type operator-(const Type& v, double s) { return v - static_cast<float>(s); }

// Application de la macro aux vecteurs
GLM_MSVC_COMPAT_SCALAR_OPS(glm::vec2)
GLM_MSVC_COMPAT_SCALAR_OPS(glm::vec3)
GLM_MSVC_COMPAT_SCALAR_OPS(glm::vec4)

// Application de la macro aux matrices
GLM_MSVC_COMPAT_SCALAR_OPS(glm::mat2)
GLM_MSVC_COMPAT_SCALAR_OPS(glm::mat3)
GLM_MSVC_COMPAT_SCALAR_OPS(glm::mat4)

// Application de la macro aux quaternions
GLM_MSVC_COMPAT_SCALAR_OPS(glm::quat)

// Nettoyage de la macro pour ne pas polluer le reste de la compilation
#undef GLM_MSVC_COMPAT_SCALAR_OPS
#endif // __cplusplus