#include "my_opengl_widget.h"

#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QMouseEvent>
#include <QMessageBox>

#include <cmath>
#include <exception>

MyOpenGLWidget::MyOpenGLWidget(QWidget *parent) :
    QOpenGLWidget(parent)
{
    QSurfaceFormat format;
    format.setDepthBufferSize(24);
    format.setStencilBufferSize(8);
    format.setProfile(QSurfaceFormat::CoreProfile);
    format.setSamples(4);
    setFormat(format);
}

void MyOpenGLWidget::initializeGL() {
    auto *gl = context()->functions();

    gl->glEnable(GL_MULTISAMPLE);

    initView();

    program = loadProgram("shaders/raytrace.vert", "shaders/raytrace.frag");

    plane = std::make_shared<GLPlane>();
    plane->attachVertices(program.get(), "vertex");

    emit initialized();
}

void MyOpenGLWidget::initView() {
    model_matrix.setToIdentity();

    view_matrix.setToIdentity();
    view_matrix.ortho(-1, 1, -1, 1, 0, 1);
    //view_matrix.lookAt(QVector3D(3.0f, 3.0f, 3.0f), QVector3D(0.0f, 0.0f, 0.0f), QVector3D(0.0f, 1.0f, 0.0f));

    projection_matrix.setToIdentity();
    const auto aspect = float(width()) / float(height());
    projection_matrix.perspective(45.0f, aspect, 0.01f, 100.0f);
}

void MyOpenGLWidget::setBackgroundColor(QColor color) {
    background_color = color;
}

QColor MyOpenGLWidget::getBackgroundColor() const {
    return background_color;
}

void MyOpenGLWidget::resizeGL(int width, int height) {
    auto *gl = context()->functions();

    gl->glViewport(0, 0, width, height);

    initView();
}

void MyOpenGLWidget::paintGL() {
    auto *gl = context()->functions();

    gl->glClearColor(static_cast<GLfloat>(background_color.redF()),
                     static_cast<GLfloat>(background_color.greenF()),
                     static_cast<GLfloat>(background_color.blueF()), 1.0f);
    gl->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (!program) {
        return;
    }

    program->bind();

    const auto mvp = projection_matrix * view_matrix * model_matrix;
    program->setUniformValue(program->uniformLocation("MVP"), mvp);

    plane->draw(gl);

    program->release();
}

std::shared_ptr<QOpenGLShaderProgram> MyOpenGLWidget::loadProgram(QString vertex_shader_file, QString fragment_shader_file) {
    auto prog = std::make_shared<QOpenGLShaderProgram>();
    if (!prog->addShaderFromSourceFile(QOpenGLShader::Vertex, QString(vertex_shader_file))) {
        throw std::runtime_error(std::string("Failed to load vertex shaders from ") + vertex_shader_file.toStdString()
                                 + ":\n" + prog->log().toStdString());
    }
    if (!prog->addShaderFromSourceFile(QOpenGLShader::Fragment, QString(fragment_shader_file))) {
        throw std::runtime_error(std::string("Failed to load fragment shaders from ") + fragment_shader_file.toStdString()
                                 + ":\n" + prog->log().toStdString());
    }
    if (!prog->link()) {
        throw std::runtime_error(std::string("Failed to link program:\n") + prog->log().toStdString());
    }
    return prog;
}

void MyOpenGLWidget::onTimer() {
    rotation_y_angle += 1.0f;
    update();
}

void MyOpenGLWidget::mousePressEvent(QMouseEvent *event) {
    mouse_pos = event->pos();
}

void MyOpenGLWidget::mouseMoveEvent(QMouseEvent *event) {
    rotation_y_angle += float(event->pos().x() - mouse_pos.x());
    rotation_x_angle += float(event->pos().y() - mouse_pos.y());
    mouse_pos = event->pos();
    update();
}

void MyOpenGLWidget::wheelEvent(QWheelEvent *event) {
    const auto coeff = (event->angleDelta().y() > 0 ? 1.0f : -1.0f);
    const auto shift = coeff*QVector3D(0.2f, 0.2f, 0.2f);
    view_matrix.translate(shift);
    update();
}
