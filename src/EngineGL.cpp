
#define GLM_FORCE_SWIZZLE
#include "EngineGL.h"
#include "Scene.h"

#include "Materials/BaseMaterial/BaseMaterial.h"
#include <filesystem>
#include <filesystem>
#include "Sphere.h"
#include "Plane.h"
#include "Texture2D.h"
//Primitive Vectoriel
#include "VectorPrimitive.h"
//End Primitive Vectoriel
#include <glm/gtc/constants.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>
#include <execution>
#include <numeric>
#include <tuple>

#define SHADOW_EPSILON 1e-5f

namespace {
bool hasCameraChanged(const glm::mat4 &current, const glm::mat4 &previous, float epsilon = 0.00001f) {
    for (int row = 0; row < 4; ++row) {
        for (int col = 0; col < 4; ++col) {
            if (std::fabs(current[row][col] - previous[row][col]) > epsilon) {
                return true;
            }
        }
    }
    return false;
}

float computeBunnyAnimationT(float elapsedTime) {
    return 0.5f * (std::sin(elapsedTime) + 1.0f);
}

void setBunnyAnimationPose(Scene *scene, float t) {
    Node *bunny = scene->getNode("Bunny");
    if (!bunny)
        return;

    static bool bunnyBaseInit = false;
    static glm::mat4 bunnyBase(1.0f);
    if (!bunnyBaseInit) {
        bunnyBase = bunny->frame()->getMatrixCopy();
        bunnyBaseInit = true;
    }

    t = glm::clamp(t, 0.0f, 1.0f);
    const glm::quat qa = glm::angleAxis(-glm::half_pi<float>(), glm::vec3(0.0f, 1.0f, 0.0f));
    const glm::quat qb = glm::angleAxis(glm::half_pi<float>(), glm::vec3(0.0f, 1.0f, 0.0f));
    const glm::quat q = bunny->frame()->slerp(qa, qb, t);
    const glm::mat4 rot = glm::mat4_cast(q);

    const glm::vec3 t0(-2.0f, 0.0f, 0.0f);
    const glm::vec3 t1(2.0f, 0.0f, 0.0f);
    const glm::vec3 s0(0.6f, 0.6f, 0.6f);
    const glm::vec3 s1(1.6f, 1.6f, 1.6f);

    const float tx = bunny->frame()->lerp(t0.x, t1.x, t);
    const float ty = bunny->frame()->lerp(t0.y, t1.y, t);
    const float tz = bunny->frame()->lerp(t0.z, t1.z, t);
    const float sx = bunny->frame()->lerp(s0.x, s1.x, t);
    const float sy = bunny->frame()->lerp(s0.y, s1.y, t);
    const float sz = bunny->frame()->lerp(s0.z, s1.z, t);

    const glm::mat4 tr = glm::translate(glm::mat4(1.0f), glm::vec3(tx, ty, tz));
    const glm::mat4 sc = glm::scale(glm::mat4(1.0f), glm::vec3(sx, sy, sz));

    bunny->frame()->setUpFromMatrix(tr * rot * sc * bunnyBase);
}
}

bool EngineGL::init() {
    LOG_INFO << "Initializing Scene" << std::endl;

    glEnable(GL_DEPTH_TEST);
    glViewport(0, 0, m_Width, m_Height);
    setClearColor(glm::vec4(0.5, 0.5, 0.5, 1.0));

    // Création d'un materiau de Base
    BaseMaterial *material = new BaseMaterial("IMN401-Mat");

    // d'un objet, méthode détaillée
    Node *bunny = scene->getNode("Bunny");
    std::string bunnyPath = ObjPath + "Bunny.obj";
    if (!std::filesystem::exists(bunnyPath)) {
        throw std::runtime_error("Missing asset: " + bunnyPath);
    }
    ModelGL *bunnyModel = scene->m_Models.get<ModelGL>(bunnyPath);
    if (!bunnyModel) {
        throw std::runtime_error("Failed to load model: " + bunnyPath);
    }
    bunny->setModel(bunnyModel);
    bunny->frame()->scale(glm::vec3(20.0));

    std::string bunnyTexturePath = ObjPath + "Textures/Bunny1.png";
    if (!std::filesystem::exists(bunnyTexturePath)) {
        throw std::runtime_error("Missing asset: " + bunnyTexturePath);
    }
    Texture2D *bunnyTexture = new Texture2D(bunnyTexturePath);
    material->setTexture(bunnyTexture);

    // Sofa
    Node *sofa = scene->getNode("Sofa");

    std::string sofaPath = ObjPath + "sofa.obj";
    if (!std::filesystem::exists(sofaPath)) {
        throw std::runtime_error("Missing asset: " + sofaPath);
    }

    ModelGL *sofaModel = scene->m_Models.get<ModelGL>(sofaPath);
    if (!sofaModel) {
        throw std::runtime_error("Failed to load model: " + sofaPath);
    }

    sofa->setModel(sofaModel);

    sofa->frame()->scale(glm::vec3(5.0f));      
    sofa->frame()->translate(glm::vec3(3.0f, 0.0f, 0.0f)); 

    BaseMaterial* sofaMat = new BaseMaterial("SofaMat");

    std::string sofaDiffusePath = ObjPath + "Textures/brickL.png";

    if (!std::filesystem::exists(sofaDiffusePath))
        throw std::runtime_error("Missing: " + sofaDiffusePath);

    Texture2D* sofaDiffuse = new Texture2D(sofaDiffusePath);

    sofa->materialProperties.useBlinnPhong = m_rasterUseBlinnPhong;
    sofa->materialProperties.useGouraud = m_rasterUseGouraud;
    
    sofaMat->enableTexture(false);

    sofa->setMaterial(sofaMat);

    sofa->materialProperties.albedo = glm::vec3(1.0f, 0.99f, 0.01f);
    sofa->materialProperties.diffuse = 0.8f;
    sofa->materialProperties.specular = 0.2f;
    sofa->materialProperties.hardness = 16.0f;

    scene->getSceneNode()->adopt(sofa);
    // End Sofa

    // Plante
    Node* plante = scene->getNode("Plante");

    std::string plantePath = ObjPath + "plante.obj";
    if (!std::filesystem::exists(plantePath)) {
        throw std::runtime_error("Missing asset: " + plantePath);
    }

    ModelGL* planteModel = scene->m_Models.get<ModelGL>(plantePath);
    if (!planteModel) {
        throw std::runtime_error("Failed to load model: " + plantePath);
    }

    std::string planteTexturePath = ObjPath + "Textures/plante.jpg";

    if (!std::filesystem::exists(planteTexturePath))
        throw std::runtime_error("Missing asset: " + planteTexturePath);

    Texture2D* planteTexture = new Texture2D(planteTexturePath);

    plante->setModel(planteModel);
    scene->getSceneNode()->adopt(plante);

    BaseMaterial* planteMat = new BaseMaterial("PlanteMat");

    plante->setMaterial(planteMat);
    planteMat->setTexture(planteTexture);

    plante->materialProperties.albedo = glm::vec3(0.1f, 0.8f, 0.2f);
    plante->materialProperties.diffuse = 0.9f;
    plante->materialProperties.specular = 0.1f;
    plante->materialProperties.hardness = 8.0f;

    plante->frame()->scale(glm::vec3(1.0f));
    plante->frame()->translate(glm::vec3(-5.0f, 0.0f, 0.0f));
    // Edn Plante

    bunny->setMaterial(material);
    scene->getSceneNode()->adopt(bunny);
    bunny->materialProperties.albedo = glm::vec3(0.2f, 0.35f, 1.0f);
    bunny->materialProperties.diffuse = 0.7f;
    bunny->materialProperties.specular = 0.35f;
    bunny->materialProperties.hardness = 32.0f;
    bunny->materialProperties.useBlinnPhong = m_rasterUseBlinnPhong;
    bunny->materialProperties.useGouraud = m_rasterUseGouraud;

    const glm::vec3 bunnyWorldPos = glm::vec3(bunny->frame()->getModelMatrix()[3]);
    const glm::mat4 cameraFrame = scene->camera()->frame()->getMatrixCopy();
    const glm::vec3 cameraPos = glm::vec3(cameraFrame[3]);
    const glm::vec3 cameraRight = glm::normalize(glm::vec3(cameraFrame[0]));
    const glm::vec3 cameraUp = glm::normalize(glm::vec3(cameraFrame[1]));
    const glm::vec3 cameraForward = glm::normalize(-glm::vec3(cameraFrame[2]));

    // Ground plane under the bunny.
    Node *groundPlane = scene->getNode("GroundPlane");
    groundPlane->setPlane(new Plane(glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f)));
    groundPlane->frame()->translate(glm::vec3(bunnyWorldPos.x, bunnyWorldPos.y - 1.0f, bunnyWorldPos.z));
    groundPlane->materialProperties.albedo = glm::vec3(0.65f, 0.65f, 0.65f);
    groundPlane->materialProperties.diffuse = 0.9f;
    groundPlane->materialProperties.specular = 0.1f;
    groundPlane->materialProperties.hardness = 8.0f;
    scene->getSceneNode()->adopt(groundPlane);

    //lumiere de base
    Light *light = new Light();
    light->type = Light::PONCTUELLE;
    light->position = glm::vec3(3.0f, 3.0f, 3.0f);
    light->puissance = 1.0f;
    light->attenuation = 0.2f;
    light->couleurAmbiante = glm::vec3(0.1f, 0.1f, 0.1f);
    light->couleurDiffuse = glm::vec3(1.0f, 1.0f, 1.0f);
    light->couleurSpeculaire = glm::vec3(1.0f, 1.0f, 1.0f);
    scene->lights.insert("Light1", light);

    //lumiere projecteur
    Light *lightProj = new Light();
    lightProj->type = Light::PROJECTEUR;
    lightProj->position = glm::vec3(-3.0f, 3.0f, 3.0f);
    lightProj->direction = glm::normalize(bunnyWorldPos - lightProj->position);
    lightProj->puissance = 1.0f;
    lightProj->attenuation = 0.1f;
    lightProj->angleCone = 20.0f; 
    lightProj->couleurAmbiante = glm::vec3(0.1f, 0.1f, 0.1f);
    lightProj->couleurDiffuse = glm::vec3(0.5f, 0.5f, 1.0f);
    lightProj->couleurSpeculaire = glm::vec3(1.0f, 0.0f, 0.0f);
    scene->lights.insert("LightProj", lightProj);

    setupEngine();
    setupRaytracer();
    return (true);
}

float genererLesEchantillonsSSAA(int samples, std::vector<std::tuple<float, float>>& offsets) {
    if (samples < 1)
        samples = 1;

    offsets.clear();
    const int gridSize = static_cast<int>(std::ceil(std::sqrt(static_cast<float>(samples))));

    for (int y = 0; y < gridSize && static_cast<int>(offsets.size()) < samples; ++y) {
        for (int x = 0; x < gridSize && static_cast<int>(offsets.size()) < samples; ++x) {
             
            float offsetX = (x + 0.5f) / gridSize;
            float offsetY = (y + 0.5f) / gridSize;

            offsets.emplace_back(offsetX, offsetY);
        }
    }
    
    //La fonction va retourner le facteur de normalisation à appliquer à la couleur (1 / nombre d'échantillons).
    return 1.0f / static_cast<float>(offsets.size());
}

void EngineGL::render()
{
    glEnable(GL_DEPTH_TEST);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (m_renderScene == RenderScene::Raytracer) {
        setBunnyAnimationPose(scene, 0.5f);

        int width = myFBO->getWidth();
        int height = myFBO->getHeight();
        glm::mat4 cameraFrame = scene->camera()->frame()->getMatrixCopy();
        glm::mat4 viewMatrix = scene->camera()->getViewMatrix();
        glm::mat4 projectionMatrix = scene->camera()->getProjectionMatrix();
        glm::mat4 inverseViewProjection = glm::inverse(projectionMatrix * viewMatrix);
        float fov = scene->camera()->getFoV();
        float aspect = scene->camera()->getAspectRatio();
        GLfloat clearColorBuffer[4] = {0.0f, 0.0f, 0.0f, 1.0f};
        glGetFloatv(GL_COLOR_CLEAR_VALUE, clearColorBuffer);
        const glm::vec4 clearColor(clearColorBuffer[0], clearColorBuffer[1], clearColorBuffer[2], clearColorBuffer[3]);

        if (!m_lastRaytracerParamsValid ||
            hasCameraChanged(cameraFrame, m_lastRaytracerCameraFrame) ||
            std::fabs(fov - m_lastRaytracerFoV) > 0.0001f ||
            std::fabs(aspect - m_lastRaytracerAspect) > 0.0001f) {
            m_raytracerDirty = true;
        }

        launchRaytracerIfNeeded(cameraFrame, inverseViewProjection, fov, aspect, width, height, clearColor);
        pollRaytracerResult();

        const size_t expectedImageSize = static_cast<size_t>(width) * static_cast<size_t>(height) * 4;
        if (m_raytracerHasImage && m_raytracerImage.size() == expectedImageSize) {
            glTextureSubImage2D(
                myFBO->getColorTexture()->getId(),
                0,
                0,
                0,
                width,
                height,
                GL_RGBA,
                GL_UNSIGNED_BYTE,
                m_raytracerImage.data());
        } else if (m_raytracerHasImage) {
            m_raytracerHasImage = false;
            m_raytracerDirty = true;
        } else {
            const GLfloat clearData[4] = {clearColor.r, clearColor.g, clearColor.b, clearColor.a};
            glClearTexImage(myFBO->getColorTexture()->getId(), 0, GL_RGBA, GL_FLOAT, clearData);
        }

        display->apply(myFBO, nullptr);
    } 
    else {
        // === NORMAL RASTERIZATION ===
        for (unsigned int i = 0; i < allNodes->nodes.size(); i++) {
            Node *node = allNodes->nodes[i];
            MaterialGL *material = node->getMaterial();
            if (material != nullptr && material->fp != nullptr) {
                Light::uniformLight(scene->lights, material->fp, material->vp);
            }
            node->render();
        }

        // === RENDER VECTOR PRIMITIVES ===

        for (unsigned int i = 0; i < allNodes->nodes.size(); i++) {
            VectorPrimitive* prim = dynamic_cast<VectorPrimitive*>(allNodes->nodes[i]);
            if (prim) {

                prim->render();
            }
        }
    }
}

std::vector<unsigned char> appliquerFlouMoyen(const std::vector<unsigned char>& imageBuffer, int width, int height, int kernelSize) {
    std::vector<unsigned char> outputBuffer = imageBuffer;

    
    int half = kernelSize / 2;


    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            
            if (x < half || x >= width - half || y < half || y >= height - half) {
                int idx = 4 * (y * width + x);
                outputBuffer[idx + 0] = 0;
                outputBuffer[idx + 1] = 0;
                outputBuffer[idx + 2] = 0;
                outputBuffer[idx + 3] = 255;
                continue;
            }
            glm::vec3 sum(0.0f);
            int count = 0;
    
            
            for (int ky = -half; ky <= half; ++ky) {
                for (int kx = -half; kx <= half; ++kx) {

                    int nx = x + kx;
                    int ny = y + ky;
                    int idx = 4 * (ny * width + nx);

                    sum.r += imageBuffer[idx + 0];
                    sum.g += imageBuffer[idx + 1];
                    sum.b += imageBuffer[idx + 2];
                    count++;
                }
            }

            sum /= (float)count;

            
            int idx = 4 * (y * width + x);
            outputBuffer[idx + 0] = (unsigned char)sum.r;
            outputBuffer[idx + 1] = (unsigned char)sum.g;
            outputBuffer[idx + 2] = (unsigned char)sum.b;
            outputBuffer[idx + 3] = 255;
        }
    }
    return outputBuffer;
}

void calculerVecteurLumiere(const Light* light, const glm::vec3& intersectionPoint, glm::vec3& lightVec) {
    
    if (light->type == Light::DIRECTIONNELLE) {
        lightVec = glm::normalize(-light->direction);
    }
    else {
        lightVec = glm::normalize(light->position - intersectionPoint);
    }

}

void EngineGL::phaseLargeRayonIntersections(const Ray& ray, IntersectionData& intersection) const {
    
    intersection.t = FLT_MAX;

    for (auto node : allNodes->nodes) {

        if (node->getSphere()) {
            node->getSphere()->intersect(ray, intersection, node->materialProperties);
        }

        if (node->getPlane()) {
            node->getPlane()->intersect(ray, intersection, node->materialProperties);
        }

        if (node->getModel()) {
            node->getModel()->intersect(ray, intersection, node->materialProperties);
        }
    }
}

void calculerEchantillonsGrilleRayons(std::tuple<float, float> offset, float &NDCx, float &NDCy, float width, float height, bool vibration) {

    float offsetX = std::get<0>(offset);
    float offsetY = std::get<1>(offset);

    if (vibration) {
        
        offsetX += ((float)rand() / RAND_MAX - 0.5f) * 0.5f;
        offsetY += ((float)rand() / RAND_MAX - 0.5f) * 0.5f;
    }

    NDCx += offsetX / width;
    NDCy += offsetY / height;

}

glm::vec3 calculerDeplacementDeCollision(const IntersectionData &intersection, const Ray &ray) {
    
    return intersection.p + SHADOW_EPSILON * intersection.n;
}

glm::vec3 EngineGL::calculerBlinnPhongLancerRayons(const Light *light, const IntersectionData &intersection, const glm::vec3 &lightVec, const glm::vec3 &eyeVec) const {
    
    glm::vec3 h = glm::normalize(lightVec + eyeVec);

    float diff = glm::max(glm::dot(intersection.n, lightVec), 0.0f);
    float spec = pow(glm::max(glm::dot(intersection.n, h), 0.0f), intersection.hardness);

    glm::vec3 diffuse = light->couleurDiffuse * diff * intersection.diffuse * intersection.albedo;
    glm::vec3 specular = light->couleurSpeculaire * spec * intersection.specular;

    return light->puissance * (diffuse + specular);
    return glm::vec3(0.0f);
}

glm::vec3 EngineGL::calculerIlluminationLancerRayons(const IntersectionData &intersection, const glm::vec3 &eyeVec, const glm::vec3 &ambient) const {
    glm::vec3 samplecolour = ambient * intersection.albedo * intersection.diffuse;
    glm::vec3 lightVec(0.0f);
    Ray shadowRay;
    IntersectionData shadowIntersection;

    shadowRay.origin = calculerDeplacementDeCollision(intersection, shadowRay);

    for (int k = 0; k < scene->lights.size(); k++) {
      
        Light* light = scene->lights.get(k);

        calculerVecteurLumiere(light, intersection.p, lightVec);

        shadowRay.direction = lightVec;
        shadowRay.origin = calculerDeplacementDeCollision(intersection, shadowRay);

        shadowIntersection.t = FLT_MAX;
        phaseLargeRayonIntersections(shadowRay, shadowIntersection);

        bool inShadow = shadowIntersection.t < FLT_MAX;

        if (!inShadow) {
            samplecolour += calculerBlinnPhongLancerRayons(light, intersection, lightVec, eyeVec);
        }
    }

    return samplecolour;
}

void EngineGL::calculerOrientationRayon(
    const glm::mat4 &inverseViewProjection,
    float clipX,
    float clipY,
    const glm::vec3 &cameraPos,
    Ray &ray,
    glm::vec3 &eyeVec) const {
    glm::vec4 clip = glm::vec4(clipX, clipY, -1.0f, 1.0f);
    glm::vec4 world = inverseViewProjection * clip;
    world /= world.w;

    ray.origin = cameraPos;
    ray.direction = glm::normalize(glm::vec3(world) - cameraPos);

    eyeVec = glm::normalize(cameraPos - glm::vec3(world));
    
}

std::vector<unsigned char> EngineGL::renderRaytracerImage(const glm::mat4 &cameraFrame, const glm::mat4 &inverseViewProjection, int width, int height, const glm::vec4 &clearColor) const {
    std::vector<std::tuple<float, float>> offsets;
    float pixelScale = genererLesEchantillonsSSAA(m_raytracerSsaaSamples, offsets);
    glm::vec3 cameraPos = cameraFrame[3].xyz();
    glm::vec3 ambient(0.2f, 0.2f, 0.2f);

    std::vector<unsigned char> imageBuffer(width * height * 4, 255);

    m_raytracerRowsTotal.store(width, std::memory_order_relaxed);
    m_raytracerRowsDone.store(0, std::memory_order_relaxed);

    std::vector<int> columns(width);
    std::iota(columns.begin(), columns.end(), 0);


    std::for_each(std::execution::par, columns.begin(), columns.end(), [&](int i) {
        
        glm::vec3 eyeVec(0.0f);
        Ray ray;
        IntersectionData intersection;

        for (int j = 0; j < height; j++) {
            glm::vec3 colour(0.0f);
            for (const auto &offset : offsets) {
                glm::vec3 samplecolour = glm::vec3(0.0f, 0.0f, 0.0f);
                float NDCx = (float)i / (float)width;
                float NDCy = (float)j / (float)height;
                

                calculerEchantillonsGrilleRayons(offset, NDCx, NDCy, (float)width, (float)height, m_raytracerJitterEnabled);

                float clipX = 2.0f * NDCx - 1.0f;
                float clipY = 2.0f * NDCy - 1.0f;

                calculerOrientationRayon(inverseViewProjection, clipX, clipY, cameraPos, ray, eyeVec);
                
                intersection.t = FLT_MAX;
                this->phaseLargeRayonIntersections(ray, intersection);

                if (intersection.t < FLT_MAX) {
                    samplecolour = calculerIlluminationLancerRayons(intersection, eyeVec, ambient);
                } else {
                    samplecolour = glm::vec3(clearColor);
                }
                colour += samplecolour * pixelScale;
            }

            colour.r = glm::min(1.0f, colour.r);
            colour.g = glm::min(1.0f, colour.g);
            colour.b = glm::min(1.0f, colour.b);
            
            int pixelIndex = 4 * (j * width + i);

            imageBuffer[pixelIndex + 0] = (unsigned char)(colour.r * 255.0f);
            imageBuffer[pixelIndex + 1] = (unsigned char)(colour.g * 255.0f);
            imageBuffer[pixelIndex + 2] = (unsigned char)(colour.b * 255.0f);
            imageBuffer[pixelIndex + 3] = 255;
            
        }
        
        m_raytracerRowsDone.fetch_add(1, std::memory_order_relaxed);
    });

    imageBuffer = appliquerFlouMoyen(imageBuffer, width, height, m_blurKernelSize);

    return imageBuffer;
}

void EngineGL::launchRaytracerIfNeeded(const glm::mat4 &cameraFrame, const glm::mat4 &inverseViewProjection, float fov, float aspect, int width, int height, const glm::vec4 &clearColor) {
    if (!m_raytracerDirty || m_raytracerTaskRunning)
        return;

    m_raytracerTaskRunning = true;
    m_raytracerDirty = false;
    m_lastRaytracerCameraFrame = cameraFrame;
    m_lastRaytracerFoV = fov;
    m_lastRaytracerAspect = aspect;
    m_lastRaytracerParamsValid = true;
    m_raytracerRowsTotal.store(width, std::memory_order_relaxed);
    m_raytracerRowsDone.store(0, std::memory_order_relaxed);

    m_raytracerTask = std::async(std::launch::async, [this, cameraFrame, inverseViewProjection, width, height, clearColor]() {
        return renderRaytracerImage(cameraFrame, inverseViewProjection, width, height, clearColor);
    });
}

void EngineGL::pollRaytracerResult() {
    if (!m_raytracerTaskRunning)
        return;

    if (!m_raytracerTask.valid()) {
        m_raytracerTaskRunning = false;
        m_raytracerDirty = true;
        return;
    }

    if (m_raytracerTask.wait_for(std::chrono::milliseconds(0)) != std::future_status::ready)
        return;

    try {
        m_raytracerImage = m_raytracerTask.get();
        m_raytracerHasImage = true;
        m_raytracerRowsDone.store(m_raytracerRowsTotal.load(std::memory_order_relaxed), std::memory_order_relaxed);
    } catch (const std::exception &e) {
        LOG_WARNING << "Raytracer async task failed: " << e.what() << std::endl;
        m_raytracerHasImage = false;
        m_raytracerDirty = true;
    } catch (...) {
        LOG_WARNING << "Raytracer async task failed with unknown exception" << std::endl;
        m_raytracerHasImage = false;
        m_raytracerDirty = true;
    }

    m_raytracerTaskRunning = false;
}

void EngineGL::animate(const float elapsedTime) {

    // Animate each node
    if (m_renderScene == RenderScene::Raytracer) {
        setBunnyAnimationPose(scene, 0.5f);
        return; // No animation for raytracer scene
    }

    setBunnyAnimationPose(scene, computeBunnyAnimationT(elapsedTime));

    for (unsigned int i = 0; i < allNodes->nodes.size(); i++) {
        allNodes->nodes[i]->animate(elapsedTime);
    }

    // Update Camera Buffer
    scene->camera()->updateBuffer();
}

void EngineGL::onWindowResize(int w, int h) {
    m_Width = w;
    m_Height = h;
    glViewport(0, 0, w, h);
    float ratio = (float)w / (float)h;

    scene->resizeViewport(w, h);
    scene->camera()->setPerspectiveProjection(glm::radians(45.0f), ratio, 1.0f, 2000.0f);

    if (myFBO) {
        m_raytracerDirty = true;
    }
}

void EngineGL::setClearColor(glm::vec4 color) {
    glClearColor(color.x, color.y, color.z, color.w);
}

void EngineGL::displayInterface() {
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("Render Scene")) {
            bool rasterizationSelected = (m_renderScene == RenderScene::Rasterization);
            bool raytracerSelected = (m_renderScene == RenderScene::Raytracer);

            if (ImGui::MenuItem("Bunny", NULL, rasterizationSelected)) {
                m_renderScene = RenderScene::Rasterization;
            }
            if (ImGui::MenuItem("Raytracer", NULL, raytracerSelected)) {
                m_renderScene = RenderScene::Raytracer;
                m_raytracerDirty = true;
            }

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Projection")) {
            bool ortho = scene->camera()->GetTypeProjection() == Camera::ORTHOGRAPHIC;
            
            if (ImGui::MenuItem("Perspective", NULL, !ortho)) {
                scene->camera()->setProjectionMode(Camera::PERSPECTIVE);
            }
            if (ImGui::MenuItem("Orthographique", NULL, ortho)) {
                scene->camera()->setProjectionMode(Camera::ORTHOGRAPHIC);
            }
            ImGui::EndMenu();
        }

        if (myFBO) {
            if (ImGui::BeginMenu("FBOs")) {
                ImGui::MenuItem(myFBO->getName().c_str(), NULL, &(myFBO->show_interface));
                ImGui::EndMenu();
            }
        }

        ImGui::EndMainMenuBar();
    }

    if (myFBO) {
        myFBO->displayInterface();
    }

    if (m_renderScene == RenderScene::Rasterization) {
        if (ImGui::Begin("Rasterization")) {
            bool useBlinnPhong = m_rasterUseBlinnPhong;
            if (ImGui::Checkbox("Use Blinn-Phong", &useBlinnPhong)) {
                m_rasterUseBlinnPhong = useBlinnPhong;
                if (allNodes) {
                    for (unsigned int i = 0; i < allNodes->nodes.size(); i++) {
                        Node *node = allNodes->nodes[i];
                        if (node && node->getMaterial() != nullptr) {
                            node->materialProperties.useBlinnPhong = m_rasterUseBlinnPhong;
                        }
                    }
                }
            }
            
            if (ImGui::Checkbox("Use Gouraud", &m_rasterUseGouraud)) {
                if (allNodes) {
                    for (unsigned int i = 0; i < allNodes->nodes.size(); i++) {
                        Node *node = allNodes->nodes[i];
                        if (node && node->getMaterial() != nullptr) {
                            node->materialProperties.useGouraud = m_rasterUseGouraud;
                        }
                    }
                }
            }
            ImGui::Text("Current model: %s %s", m_rasterUseBlinnPhong ? "Blinn-Phong" : "Phong", m_rasterUseGouraud ? "(Gouraud)" : "");
        }

        primitive3DManager.displayInterface();
        ImGui::End();
    }

    if (m_renderScene == RenderScene::Raytracer) {
        const int totalRows = m_raytracerRowsTotal.load(std::memory_order_relaxed);
        const int doneRows = m_raytracerRowsDone.load(std::memory_order_relaxed);
        const float progress = (totalRows > 0) ? glm::clamp(static_cast<float>(doneRows) / static_cast<float>(totalRows), 0.0f, 1.0f) : 0.0f;

        if (ImGui::Begin("Raytracer")) {
            ImGui::Text("Resolution: %d x %d", myFBO ? myFBO->getWidth() : m_raytracerResolutionWidth, myFBO ? myFBO->getHeight() : m_raytracerResolutionHeight);

            ImGui::Text("Presets:");
            if (ImGui::Button("100x100")) {
                m_raytracerResolutionWidth = 100;
                m_raytracerResolutionHeight = 100;
                if (myFBO) {
                    myFBO->resizeFBO(m_raytracerResolutionWidth, m_raytracerResolutionHeight);
                    m_raytracerRowsDone.store(0, std::memory_order_relaxed);
                    m_raytracerRowsTotal.store(m_raytracerResolutionWidth, std::memory_order_relaxed);
                    m_raytracerHasImage = false;
                    m_raytracerDirty = true;
                }
            }
            ImGui::SameLine();
            if (ImGui::Button("256x256")) {
                m_raytracerResolutionWidth = 256;
                m_raytracerResolutionHeight = 256;
                if (myFBO) {
                    myFBO->resizeFBO(m_raytracerResolutionWidth, m_raytracerResolutionHeight);
                    m_raytracerRowsDone.store(0, std::memory_order_relaxed);
                    m_raytracerRowsTotal.store(m_raytracerResolutionWidth, std::memory_order_relaxed);
                    m_raytracerHasImage = false;
                    m_raytracerDirty = true;
                }
            }
            ImGui::SameLine();
            if (ImGui::Button("512x512")) {
                m_raytracerResolutionWidth = 512;
                m_raytracerResolutionHeight = 512;
                if (myFBO) {
                    myFBO->resizeFBO(m_raytracerResolutionWidth, m_raytracerResolutionHeight);
                    m_raytracerRowsDone.store(0, std::memory_order_relaxed);
                    m_raytracerRowsTotal.store(m_raytracerResolutionWidth, std::memory_order_relaxed);
                    m_raytracerHasImage = false;
                    m_raytracerDirty = true;
                }
            }
            if (ImGui::Button("Window Size")) {
                m_raytracerResolutionWidth = m_Width;
                m_raytracerResolutionHeight = m_Height;
                if (myFBO) {
                    myFBO->resizeFBO(m_raytracerResolutionWidth, m_raytracerResolutionHeight);
                    m_raytracerRowsDone.store(0, std::memory_order_relaxed);
                    m_raytracerRowsTotal.store(m_raytracerResolutionWidth, std::memory_order_relaxed);
                    m_raytracerHasImage = false;
                    m_raytracerDirty = true;
                }
            }

            ImGui::Separator();

            ImGui::PushItemWidth(120.0f);
            ImGui::InputInt("Width", &m_raytracerResolutionWidth);
            ImGui::InputInt("Height", &m_raytracerResolutionHeight);
            ImGui::PopItemWidth();

            if (m_raytracerResolutionWidth < 1)
                m_raytracerResolutionWidth = 1;
            if (m_raytracerResolutionHeight < 1)
                m_raytracerResolutionHeight = 1;

            if (ImGui::Button("Apply Resolution") && myFBO) {
                myFBO->resizeFBO(m_raytracerResolutionWidth, m_raytracerResolutionHeight);
                m_raytracerRowsDone.store(0, std::memory_order_relaxed);
                m_raytracerRowsTotal.store(m_raytracerResolutionWidth, std::memory_order_relaxed);
                m_raytracerHasImage = false;
                m_raytracerDirty = true;
            }

            ImGui::Separator();

            ImGui::PushItemWidth(120.0f);
            ImGui::InputInt("Blur Kernel Size", &m_blurKernelSize, 2, 4);
            ImGui::PopItemWidth();

            if (m_blurKernelSize < 1)
                m_blurKernelSize = 1;
            if (m_blurKernelSize % 2 == 0)
                m_blurKernelSize -= 1;

            if (ImGui::Button("Apply Blur Size")) {
                m_raytracerHasImage = false;
                m_raytracerDirty = true;
            }

            ImGui::Separator();

            ImGui::PushItemWidth(120.0f);
            ImGui::InputInt("SSAA Samples", &m_raytracerSsaaSamples, 1, 2);
            ImGui::PopItemWidth();
            if (m_raytracerSsaaSamples < 1)
                m_raytracerSsaaSamples = 1;

            ImGui::Checkbox("Jitter", &m_raytracerJitterEnabled);

            if (ImGui::Button("Apply SSAA/Jitter")) {
                m_raytracerHasImage = false;
                m_raytracerDirty = true;
            }

            ImGui::Separator();

            if (m_raytracerTaskRunning) {
                ImGui::Text("Rendering...");
                ImGui::ProgressBar(progress, ImVec2(280.0f, 0.0f));
                ImGui::Text("Columns rendered: %d / %d", doneRows, totalRows);
            } else {
                ImGui::Text("Ready");
                ImGui::ProgressBar(1.0f, ImVec2(280.0f, 0.0f));
                ImGui::Text("Columns rendered: %d / %d", doneRows, totalRows);
            }
        }
        ImGui::End();
    }
}

// Message callbck error for getting OpenGL problems
// All credits to https://github.com/fendevel/Guide-to-Modern-OpenGL-Functions/blob/master/README.md#gltexture
void message_callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, GLchar const *message, void const *user_param) {
    auto const src_str = [source]() {
        switch (source) {
        case GL_DEBUG_SOURCE_API: return "API";
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM: return "WINDOW SYSTEM";
        case GL_DEBUG_SOURCE_SHADER_COMPILER: return "SHADER COMPILER";
        case GL_DEBUG_SOURCE_THIRD_PARTY: return "THIRD PARTY";
        case GL_DEBUG_SOURCE_APPLICATION: return "APPLICATION";
        case GL_DEBUG_SOURCE_OTHER: return "OTHER";
        default: return "";
        }
    }();

    auto const type_str = [type]() {
        switch (type) {
        case GL_DEBUG_TYPE_ERROR: return "ERROR";
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: return "DEPRECATED_BEHAVIOR";
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR: return "UNDEFINED_BEHAVIOR";
        case GL_DEBUG_TYPE_PORTABILITY: return "PORTABILITY";
        case GL_DEBUG_TYPE_PERFORMANCE: return "PERFORMANCE";
        case GL_DEBUG_TYPE_MARKER: return "MARKER";
        case GL_DEBUG_TYPE_OTHER: return "OTHER";
        default: return "";
        }
    }();

    auto const severity_str = [severity]() {
        switch (severity) {
        case GL_DEBUG_SEVERITY_NOTIFICATION: return "NOTIFICATION";
        case GL_DEBUG_SEVERITY_LOW: return "LOW";
        case GL_DEBUG_SEVERITY_MEDIUM: return "MEDIUM";
        case GL_DEBUG_SEVERITY_HIGH: return "HIGH";
        default: return "";
        }
    }();
    LOG_INFO << src_str << ", " << type_str << ", " << severity_str << ", " << id << ": " << message << '\n';
}

EngineGL::EngineGL(int width, int height) {
    m_Width = width;
    m_Height = height;

    myFBO = NULL;
    display = NULL;

    m_renderScene = RenderScene::Rasterization;
    m_raytracerReady = false;
    m_triangleVao = 0;
    m_triangleVbo = 0;
    m_triangleVertexCount = 0;
    m_triangleVs = nullptr;
    m_triangleFs = nullptr;
    m_trianglePipeline = nullptr;
    m_raytracerTaskRunning = false;
    m_raytracerDirty = true;
    m_raytracerHasImage = false;
    m_lastRaytracerFoV = 0.0f;
    m_lastRaytracerAspect = 0.0f;
    m_lastRaytracerParamsValid = false;
    m_raytracerResolutionWidth = 100;
    m_raytracerResolutionHeight = 100;
    m_blurKernelSize = 3;
    m_raytracerSsaaSamples = 1;
    m_raytracerJitterEnabled = true;
    m_rasterUseBlinnPhong = false;
    m_rasterUseGouraud = false;
    m_raytracerRowsDone.store(0, std::memory_order_relaxed);
    m_raytracerRowsTotal.store(0, std::memory_order_relaxed);
    primitive3DManager = Primitive3DManager();
    primitive3DManager.setOnCreateNode([this]() {refreshNodeCollector();}); // On passe la fonction pour refresh le node collect

    scene = Scene::getInstance();
    scene->resizeViewport(m_Width, m_Height);
}

EngineGL::~EngineGL() {
    if (m_raytracerTask.valid()) {
        m_raytracerTask.wait();
    }

    if (m_triangleVbo != 0)
        glDeleteBuffers(1, &m_triangleVbo);
    if (m_triangleVao != 0)
        glDeleteVertexArrays(1, &m_triangleVao);

    delete m_trianglePipeline;
    delete m_triangleVs;
    delete m_triangleFs;
    delete display;
    delete myFBO;
}

void EngineGL::setupEngine() {
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, nullptr, GL_FALSE);
    glDebugMessageCallback(message_callback, nullptr);

    this->allNodes = new NodeCollector();
    allNodes->collect(scene->getRoot());

    LOG_INFO << "initialisation complete" << std::endl;
}

void EngineGL::setupRaytracer() {
    if (m_raytracerReady)
        return;

    myFBO = new FrameBufferObject("Raytracer", m_raytracerResolutionWidth, m_raytracerResolutionHeight);
    display = new Display("Display");
    m_raytracerReady = true;
    m_raytracerDirty = true;

}
void EngineGL::refreshNodeCollector()
{
    if (allNodes) {
        delete allNodes;
    }
    allNodes = new NodeCollector();
    allNodes->collect(scene->getRoot());
}