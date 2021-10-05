#include "gl_plane.h"

GLPlane::GLPlane() {
    // Vertex coordinates.
    const std::vector<QVector3D> vertices = {
        QVector3D(-1.0f, -1.0f,  0.0f),
        QVector3D(1.0f, -1.0f,  0.0f),
        QVector3D(1.0f,  1.0f,  0.0f),
        QVector3D(-1.0f,  1.0f,  0.0f),
    };

    const std::vector<GLuint> indices = {
        0, 1, 2,
        2, 3, 0
    };

    setVertices(vertices, GL_FLOAT, 3);
    setIndices(indices, GL_UNSIGNED_INT);
}
