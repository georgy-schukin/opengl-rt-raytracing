#pragma once

#include "gl_shape.h"
#include "gl_buffer.h"

#include <QOpenGLVertexArrayObject>
#include <QOpenGLBuffer>
#include <QVector3D>
#include <QOpenGLShaderProgram>

#include <vector>

class GLTriangulatedShape : public GLShape {
public:
    GLTriangulatedShape();

    void draw(QOpenGLFunctions *gl) override;

    template <class T>
    void setVertices(const typename std::vector<T> &vertices,
                     GLenum el_type,
                     int el_size,
                     QOpenGLBuffer::UsagePattern pattern = QOpenGLBuffer::StaticDraw)
    {
        setBuffer(vertex_buffer, vertices, el_type, el_size, pattern);
    }

    template <class T>
    void setTextureCoords(const typename std::vector<T> &coords,
                          GLenum el_type,
                          int el_size,
                          QOpenGLBuffer::UsagePattern pattern = QOpenGLBuffer::StaticDraw)
    {
        setBuffer(tex_coord_buffer, coords, el_type, el_size, pattern);
    }

    template <class T>
    void setColors(const typename std::vector<T> &colors,
                   GLenum el_type,
                   int el_size,
                   QOpenGLBuffer::UsagePattern pattern = QOpenGLBuffer::StaticDraw)
    {
        setBuffer(color_buffer, colors, el_type, el_size, pattern);
    }

    template <class T>
    void setIndices(const typename std::vector<T> &indices,
                    GLenum el_type,
                    QOpenGLBuffer::UsagePattern pattern = QOpenGLBuffer::StaticDraw)
    {
        setBuffer(index_buffer, indices, el_type, 1, pattern);
    }

    void attachVertices(QOpenGLShaderProgram *program, QString var);
    void attachTextureCoords(QOpenGLShaderProgram *program, QString var);
    void attachColors(QOpenGLShaderProgram *program, QString var);

protected:
    template <class T>
    void setBuffer(GLBuffer &buf,
                   const typename std::vector<T> &elems,
                   GLenum elem_type,
                   int elem_size,
                   QOpenGLBuffer::UsagePattern pattern)
    {
        vao.bind();
        buf.init(elems, elem_type, elem_size, pattern);
        vao.release();
    }

    void attachBuffer(QOpenGLShaderProgram *program, QString var, GLBuffer &buf);

protected:
    QOpenGLVertexArrayObject vao;
    GLBuffer vertex_buffer, color_buffer, tex_coord_buffer, index_buffer;
};
