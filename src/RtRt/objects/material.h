#pragma once

#include <QVector3D>
#include <QColor>

class Material {
public:
    Material() {}
    Material(const QVector3D &diffuse, const QVector3D &specular, float shininess = 50.0) :
        diffuse(diffuse), specular(specular), shininess(shininess) {
    }

    void makeTransparent(float refrCoeff, float refrIndex = 1.0) {
        refractionCoeff = refrCoeff;
        refractionIndex = refrIndex;
    }

public:
    QVector3D diffuse {1.0, 1.0, 1.0};
    QVector3D specular {0.0, 0.0, 0.0};
    float shininess {50.0};
    float refractionCoeff {0.0};
    float refractionIndex {1.0};
};
