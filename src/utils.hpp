#pragma once

#include <fstream>
#include <sstream>
#include <string>
#include <glm/glm.hpp>
#include <glm/gtc/epsilon.hpp>

static inline std::string readFile(const std::string &filePath) {
    std::ifstream ifs(filePath, std::ifstream::in);
    if (!ifs.is_open())
        throw std::ios_base::failure("cannot open file: " + filePath);

    std::stringstream s;
    s << ifs.rdbuf();
    ifs.close();
    return s.str();
}

static inline bool matEquals(glm::mat4 a, glm::mat4 b, float epsilon = 1e-6f){
    return glm::all(glm::epsilonEqual(a[0], b[0], epsilon)) &&
           glm::all(glm::epsilonEqual(a[1], b[1], epsilon)) &&
           glm::all(glm::epsilonEqual(a[2], b[2], epsilon)) &&
           glm::all(glm::epsilonEqual(a[3], b[3], epsilon));
}