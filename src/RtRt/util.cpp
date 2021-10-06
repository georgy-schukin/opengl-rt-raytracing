#include "util.h"

namespace util {

QColor vecToColor(const QVector3D &vec) {
    return QColor(static_cast<int>(255 * vec.x()),
                  static_cast<int>(255 * vec.y()),
                  static_cast<int>(255 * vec.z()));
}

QVector3D colorToVec(const QColor &color) {
    return QVector3D(static_cast<float>(color.redF()),
                     static_cast<float>(color.greenF()),
                     static_cast<float>(color.blueF()));
}

}
