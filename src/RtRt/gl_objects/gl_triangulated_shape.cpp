#include "gl_triangulated_shape.h"

#include <QOpenGLFunctions>

GLTriangulatedShape::GLTriangulatedShape() :
    vertex_buffer(QOpenGLBuffer::VertexBuffer),
    color_buffer(QOpenGLBuffer::VertexBuffer),
    tex_coord_buffer(QOpenGLBuffer::VertexBuffer),
    index_buffer(QOpenGLBuffer::IndexBuffer)
{
    vao.create();
}

void GLTriangulatedShape::attachVertices(QOpenGLShaderProgram *program, QString var) {
    attachBuffer(program, var, vertex_buffer);
}

void GLTriangulatedShape::attachTextureCoords(QOpenGLShaderProgram *program, QString var) {
    attachBuffer(program, var, tex_coord_buffer);
}

void GLTriangulatedShape::attachColors(QOpenGLShaderProgram *program, QString var) {
    attachBuffer(program, var, color_buffer);
}

void GLTriangulatedShape::attachBuffer(QOpenGLShaderProgram *program, QString var, GLBuffer &buf) {
    const int loc = program->attributeLocation(var);
    if (loc == -1) {
        return;
    }
    vao.bind();
    buf.bind();
    program->enableAttributeArray(loc);
    program->setAttributeBuffer(loc, buf.elemType(), 0, buf.elemSize());
    vao.release();
}

void GLTriangulatedShape::draw(QOpenGLFunctions *gl) {
    vao.bind();
    gl->glDrawElements(GL_TRIANGLES, index_buffer.size(), index_buffer.elemType(), 0);
    vao.release();
}
