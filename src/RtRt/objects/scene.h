#pragma once

#include "sphere.h"
#include "light_source.h"
#include "material.h"

#include <vector>

class Scene {
public:
    Scene() {}

    void addObject(const Sphere &s) {
        objects.push_back(s);
    }

    void addLight(const LightSource &l) {
        lights.push_back(l);
    }

    int addMaterial(const Material &material) {
        materials.push_back(material);
        return static_cast<int>(materials.size()) - 1;
    }

    void clear() {
        objects.clear();
    }

public:
    std::vector<Sphere> objects;
    std::vector<LightSource> lights;
    std::vector<Material> materials;
};
