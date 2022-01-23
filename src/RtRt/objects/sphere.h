#pragma once

#include <QVector3D>
#include <QColor>

class Sphere {
public:
    Sphere() {}
    Sphere(const QVector3D &pos, double radius, int matId) :
        position(pos), radius(radius), materialId(matId) {
    }

public:
    QVector3D position {0.0, 0.0, 0.0};
    double radius {1.0};
    int materialId;
};
