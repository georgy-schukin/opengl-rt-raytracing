#pragma once

#include <QOpenGLBuffer>
#include <QOpenGLShaderProgram>

#include <vector>

class GLBuffer {
public:
    GLBuffer(QOpenGLBuffer::Type type) :
        buffer(type) {
    }

    template <class T>
    void init(const typename std::vector<T> &elems,
              GLenum el_type,
              int el_size,
              QOpenGLBuffer::UsagePattern pattern = QOpenGLBuffer::StaticDraw)
    {
        buffer.create();
        buffer.setUsagePattern(pattern);
        buffer.bind();
        buffer.allocate(elems.data(), elems.size()*sizeof(T));
        elem_type = el_type;
        elem_size = el_size;
        num_of_elems = elems.size();
    }

    GLenum elemType() const {
        return elem_type;
    }

    int elemSize() const {
        return elem_size;
    }

    void bind() {
        buffer.bind();
    }

    void release() {
        buffer.release();
    }

    int size() const {
        return num_of_elems;
    }

private:
    QOpenGLBuffer buffer;
    GLenum elem_type;
    int elem_size;
    int num_of_elems {0};
};
