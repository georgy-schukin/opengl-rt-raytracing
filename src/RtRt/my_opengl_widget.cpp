#include "my_opengl_widget.h"
#include "util.h"

#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QMouseEvent>
#include <QMessageBox>

#include <cmath>
#include <exception>
#include <random>

namespace {

Scene defaultScene() {
    QVector3D red {1, 0.3, 0.3};
    QVector3D blue {0.3, 0.3, 1};
    QVector3D green {0.3, 1, 0.3};
    QVector3D white {0.8, 0.8, 0.8};
    QVector3D yellow {1, 1, 0.3};
    QVector3D purple {1, 0.3, 1};

    Scene scene;
    scene.addObject(Sphere {{0, 2, 1}, 1.5, blue});
    scene.addObject(Sphere {{1, -2, 4}, 2, red});
    scene.addObject(Sphere {{0, -2, -3}, 1, green});
    scene.addObject(Sphere {{1.5, 0.5, -2}, 1, white});
    scene.addObject(Sphere {{-2, 1, 5}, 0.7, yellow});
    scene.addObject(Sphere {{-2.2, 0, 2}, 1, white});
    scene.addObject(Sphere {{1, 1, 4}, 0.7, purple});

    scene.addLight(LightSource {{-15, 15, -15}, {1.0, 1.0, 1.0}});
    scene.addLight(LightSource {{1, 1, 0}, {0.2, 0.2, 1.0}});
    scene.addLight(LightSource {{0, -10, 6}, {1.0, 0.2, 0.2}});
    return scene;
}

Scene randomScene(int num_of_objects) {
    std::random_device rd;
    std::mt19937 re(rd());
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);
    Scene scene;
    const float max_p = 5.0f;
    for (int i = 0; i < num_of_objects; i++) {
        QVector3D pos {2 * max_p * dist(re) - max_p,
                       2 * max_p * dist(re) - max_p,
                       2 * max_p * dist(re) - max_p};
        double radius {1.5f * dist(re) + 0.1f};
        QVector3D color {dist(re), dist(re), dist(re)};
        scene.addObject(Sphere {pos, radius, color});
    }
    scene.addLight(LightSource {{-15, 15, -15}, {1.0, 1.0, 1.0}});
    scene.addLight(LightSource {{1, 1, 0}, {0.2, 0.2, 1.0}});
    scene.addLight(LightSource {{0, -10, 6}, {1.0, 0.2, 0.2}});
    return scene;
}

}

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

    initScene();
    initView();

    program = loadProgram("shaders/raytrace.vert", "shaders/raytrace.frag");

    plane = std::make_shared<GLPlane>(); // plane is in NDC already
    plane->attachVertices(program.get(), "vertex");

    emit initialized();
}

void MyOpenGLWidget::initScene() {
    scene = defaultScene();
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

    const auto num_of_spheres = static_cast<int>(scene.objects.size());
    program->setUniformValue(program->uniformLocation("numOfSpheres"), num_of_spheres);
    int cnt = 0;
    for (const auto &s: scene.objects) {
        program->setUniformValue(program->uniformLocation(QString("spheres[%1].position").arg(cnt)), model_m * s.position);
        program->setUniformValue(program->uniformLocation(QString("spheres[%1].radius").arg(cnt)), static_cast<GLfloat>(s.radius));
        program->setUniformValue(program->uniformLocation(QString("spheres[%1].color").arg(cnt)), s.color);
        cnt++;
    }

    cnt = 0;
    const auto num_of_lights = static_cast<int>(scene.lights.size());
    program->setUniformValue(program->uniformLocation("numOfLightSources"), num_of_lights);
    for (const auto &l: scene.lights) {
        program->setUniformValue(program->uniformLocation(QString("lightSources[%1].position").arg(cnt)), model_m * l.position);
        program->setUniformValue(program->uniformLocation(QString("lightSources[%1].color").arg(cnt)), l.color);
        cnt++;
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

void MyOpenGLWidget::randomScene() {
    scene = ::randomScene(32);
    update();
}
