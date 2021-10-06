#pragma once

#include <QVector3D>
#include <QColor>

class LightSource {
public:
    LightSource() {}
    LightSource(const QVector3D &pos, const QVector3D &color) :
        position(pos), color(color) {
    }

public:
    QVector3D position {0.0, 0.0, 0.0};
    QVector3D color {1.0, 1.0, 1.0};
};
