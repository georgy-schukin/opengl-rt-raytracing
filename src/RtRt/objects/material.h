#pragma once

#include <QVector3D>
#include <QColor>

class Material {
public:
    Material() {}
    Material(const QVector3D &diffuse, const QVector3D &specular, double shininess = 50.0) :
        diffuse(diffuse), specular(specular), shininess(shininess) {
    }

public:
    QVector3D diffuse {1.0, 1.0, 1.0};
    QVector3D specular {0.0, 0.0, 0.0};
    double shininess {50.0};
};
