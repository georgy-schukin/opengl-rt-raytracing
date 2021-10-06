#pragma once

#include <QVector3D>
#include <QColor>

namespace util {

QColor vecToColor(const QVector3D &vec);
QVector3D colorToVec(const QColor &color);

}
