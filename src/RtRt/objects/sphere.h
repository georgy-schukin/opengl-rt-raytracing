#pragma once

#include <QVector3D>
#include <QColor>

class Sphere {
public:
    Sphere() {}
    Sphere(const QVector3D &pos, double radius, const QVector3D &color) :
        position(pos), radius(radius), color(color) {
    }

public:
    QVector3D position {0.0, 0.0, 0.0};
    double radius {1.0};
    QVector3D color {1.0, 0.0, 0.0};
};
