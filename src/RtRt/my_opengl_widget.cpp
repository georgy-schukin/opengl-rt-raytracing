#include "my_opengl_widget.h"
#include "util.h"

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
    format.setSamples(2);
    setFormat(format);
}

void MyOpenGLWidget::initializeGL() {
    auto *gl = context()->functions();

    gl->glEnable(GL_MULTISAMPLE);

    initScene();
    initView();

    program = loadProgram("shaders/raytrace.vert", "shaders/raytrace.frag");

    plane = std::make_shared<GLPlane>(); // plane is in NDC already
    plane->attachVertices(program.get(), "vertex");

    emit initialized();
}

void MyOpenGLWidget::initScene() {
    QVector3D red {1, 0.3, 0.3};
    QVector3D blue {0.3, 0.3, 1};
    QVector3D green {0.3, 1, 0.3};
    QVector3D white {0.5, 0.5, 0.5};
    QVector3D yellow {1, 1, 0.3};
    QVector3D purple {1, 0.3, 1};

    objects.push_back(Sphere {{0, 2, 1}, 1.5, blue});
    objects.push_back(Sphere {{1, -2, 4}, 2, red});
    objects.push_back(Sphere {{0, -2, -3}, 1, green});
    objects.push_back(Sphere {{1.5, 0.5, -2}, 1, white});
    objects.push_back(Sphere {{-2, 1, 5}, 0.7, yellow});
    objects.push_back(Sphere {{-2.2, 0, 2}, 1, white});
    objects.push_back(Sphere {{1, 1, 4}, 0.7, purple});

    lights.push_back(LightSource {{-15, 15, -15}, {0.1, 0.1, 0.1}});
    lights.push_back(LightSource {{1, 1, 0}, {0.1, 0.1, 0.5}});
    lights.push_back(LightSource {{0, -10, 6}, {0.5, 0.1, 0.1}});
}

void MyOpenGLWidget::initView() {
    model_matrix.setToIdentity();

    view_matrix.setToIdentity();    
    view_matrix.lookAt(eye, QVector3D(0.0f, 0.0f, 0.0f), QVector3D(0.0f, 1.0f, 0.0f));

    projection_matrix.setToIdentity();
    const auto aspect = float(width()) / float(height());
    projection_matrix.perspective(cameraFOV, aspect, 0.001f, 100.0f);
}

void MyOpenGLWidget::setBackgroundColor(QColor color) {
    background_color = color;
}

QColor MyOpenGLWidget::getBackgroundColor() const {
    return background_color;
}

void MyOpenGLWidget::setIterationLimit(int limit) {
    num_of_steps = limit;
}

int MyOpenGLWidget::getIterationLimit() const {
    return num_of_steps;
}

void MyOpenGLWidget::resizeGL(int width, int height) {
    auto *gl = context()->functions();

    gl->glViewport(0, 0, width, height);

    initView();
}

void MyOpenGLWidget::paintGL() {
    auto *gl = context()->functions();

    const auto bg_color = util::colorToVec(background_color);
    gl->glClearColor(bg_color.x(), bg_color.y(), bg_color.z(), 1.0f);
    gl->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (!program) {
        return;
    }

    QMatrix4x4 rotate;
    rotate.rotate(rotation_y_angle, QVector3D(0.0f, 1.0f, 0.0f));
    rotate.rotate(rotation_x_angle, QVector3D(1.0f, 0.0f, 0.0f));

    auto model_m = rotate * model_matrix;

    program->bind();

    program->setUniformValue(program->uniformLocation("numOfSteps"), num_of_steps);

    const auto num_of_spheres = static_cast<int>(objects.size());
    program->setUniformValue(program->uniformLocation("numOfSpheres"), num_of_spheres);
    for (int i = 0; i < num_of_spheres; i++) {
        program->setUniformValue(program->uniformLocation(QString("spheres[%1].position").arg(i)), model_m * objects[i].position);
        program->setUniformValue(program->uniformLocation(QString("spheres[%1].radius").arg(i)), static_cast<GLfloat>(objects[i].radius));
        program->setUniformValue(program->uniformLocation(QString("spheres[%1].color").arg(i)), objects[i].color);
    }

    const auto num_of_lights = static_cast<int>(lights.size());
    program->setUniformValue(program->uniformLocation("numOfLightSources"), num_of_lights);
    for (int i = 0; i < num_of_lights; i++) {
        program->setUniformValue(program->uniformLocation(QString("lightSources[%1].position").arg(i)), model_m * lights[i].position);
        program->setUniformValue(program->uniformLocation(QString("lightSources[%1].color").arg(i)), lights[i].color);
    }

    program->setUniformValue(program->uniformLocation("backgroundColor"), util::colorToVec(background_color));

    program->setUniformValue(program->uniformLocation("camToWorld"), view_matrix.inverted());
    program->setUniformValue(program->uniformLocation("windowSize"), QVector2D(width(), height()));
    program->setUniformValue(program->uniformLocation("cameraFOV"), cameraFOV);
    const float PI = 3.141592653589793;
    program->setUniformValue(program->uniformLocation("fovTangent"), std::tan(cameraFOV * PI / 360.0f));

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
    const auto coeff = (event->angleDelta().y() > 0 ? 0.5f : -0.5f);
    eye += coeff * QVector3D(1, 1, 1);
    initView();
    update();
}
