#pragma once

#include "sphere.h"
#include "light_source.h"

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

    void clear() {
        objects.clear();
    }

public:
    std::vector<Sphere> objects;
    std::vector<LightSource> lights;
};
