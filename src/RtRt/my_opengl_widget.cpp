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

    auto redMat = scene.addMaterial(Material {red * 0.4, red * 0.6, 250});
    auto blueMat = scene.addMaterial(Material {blue * 0.4, blue * 0.6, 50});
    auto greenMat = scene.addMaterial(Material {green * 0.8, green * 0.2, 10});
    auto whiteMat = scene.addMaterial(Material {white * 0.9, white * 0.1, 50});
    auto yellowMat = scene.addMaterial(Material {yellow * 0.1,yellow * 0.9, 500});
    auto purpleMat = scene.addMaterial(Material {purple * 0.6, purple * 0.4, 30});    

    scene.getMaterial(blueMat).makeTransparent(0.9, 1.03);
    scene.getMaterial(redMat).makeTransparent(0.6, 0.8);

    scene.addObject(Sphere {{0, 2, 1}, 1.5, blueMat});
    scene.addObject(Sphere {{1, -2, 4}, 2, redMat});
    scene.addObject(Sphere {{0, -2, -3}, 1, greenMat});
    scene.addObject(Sphere {{1.5, 0.5, -2}, 1, whiteMat});
    scene.addObject(Sphere {{-2, 1, 5}, 0.7, yellowMat});
    scene.addObject(Sphere {{-2.2, 0, 2}, 1, whiteMat});
    scene.addObject(Sphere {{1, 1, 4}, 0.7, purpleMat});

    scene.addLight(LightSource {{-15, 15, -15}, {1.0, 1.0, 1.0}});
    scene.addLight(LightSource {{1, 1, 0}, {0.2, 0.2, 1.0}});
    scene.addLight(LightSource {{0, -10, 6}, {1.0, 0.2, 0.2}});
    return scene;
}

void addRandomObject(Scene &scene, float max_pos = 5.0f, float min_rad = 0.2f, float max_rad = 1.5f) {
    std::random_device rd;
    std::mt19937 re(rd());
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);
    std::normal_distribution<float> normDist(0.5, 0.5);
    QVector3D pos {2 * max_pos * dist(re) - max_pos,
                   2 * max_pos * dist(re) - max_pos,
                   2 * max_pos * dist(re) - max_pos};
    double radius {(max_rad - min_rad) * dist(re) + min_rad};
    QVector3D diffuse {dist(re), dist(re), dist(re)};
    //QVector3D specular {dist(re), dist(re), dist(re)};
    const auto diffCoeff = dist(re);
    const auto specCoeff = 1.0f - diffCoeff;
    auto material = scene.addMaterial(Material {diffuse * diffCoeff, diffuse * specCoeff, dist(re) * 1000});
    scene.getMaterial(material).makeTransparent(normDist(re), 1.5f - normDist(re));
    scene.addObject(Sphere {pos, radius, material});
}

Scene randomScene(int num_of_objects) {
    Scene scene;    
    for (int i = 0; i < num_of_objects; i++) {
        addRandomObject(scene);
    }
    scene.addLight(LightSource {{-15, 15, -15}, {1.0, 1.0, 1.0}});
    scene.addLight(LightSource {{1, 1, 0}, {0.2, 0.2, 1.0}});
    scene.addLight(LightSource {{0, -10, 6}, {1.0, 0.2, 0.2}});
    return scene;
}

}

MyOpenGLWidget::MyOpenGLWidget(QWidget *parent) :
    QOpenGLWidget(parent),
    jitter(QOpenGLTexture::Target2D),
    randoms(QOpenGLTexture::Target1D)
{
    QSurfaceFormat format;
    format.setDepthBufferSize(24);
    format.setStencilBufferSize(8);
    format.setProfile(QSurfaceFormat::CoreProfile);    
    format.setSamples(1);
    setFormat(format);
}

void MyOpenGLWidget::initializeGL() {
    auto *gl = context()->functions();

    gl->glDisable(GL_MULTISAMPLE);
    gl->glDisable(GL_DEPTH_TEST);

    initScene();
    initView();
    initTextures();

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

std::vector<QVector2D> jitter2D(int size) {
    std::vector<QVector2D> jitter;
    std::random_device rd;
    std::mt19937 mt(rd());
    std::uniform_real_distribution<GLfloat> dist(0.0f, 1.0f);
    for (int i = 0; i < size; i++)
    for (int j = 0; j < size; j++) {
        float x = (i + dist(mt)) / size;
        float y = (j + dist(mt)) / size;
        jitter.push_back(QVector2D(x, y));
    }
    return jitter;
}

std::vector<float> randoms1D(int size) {
    std::vector<float> randoms;
    std::random_device rd;
    std::mt19937 mt(rd());
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);
    for (int i = 0; i < size; i++) {
        randoms.push_back(dist(mt));
    }
    return randoms;
}

void MyOpenGLWidget::initTextures() {
    jitter_size = 256;
    jitter.setSize(jitter_size, jitter_size);
    jitter.setMinMagFilters(QOpenGLTexture::Nearest, QOpenGLTexture::Nearest);
    jitter.setWrapMode(QOpenGLTexture::Repeat);
    jitter.setFormat(QOpenGLTexture::RG32F);
    jitter.allocateStorage();
    const auto jitter_data = jitter2D(jitter_size);
    jitter.setData(QOpenGLTexture::RG, QOpenGLTexture::Float32, jitter_data.data());

    randoms_size = 4096;
    randoms.setSize(randoms_size);
    randoms.setMinMagFilters(QOpenGLTexture::Nearest, QOpenGLTexture::Nearest);
    randoms.setWrapMode(QOpenGLTexture::Repeat);
    randoms.setFormat(QOpenGLTexture::R32F);
    randoms.allocateStorage();
    const auto randoms_data = randoms1D(randoms_size);
    randoms.setData(QOpenGLTexture::Red, QOpenGLTexture::Float32, randoms_data.data());
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

void MyOpenGLWidget::setNumOfSamples(int num) {
    num_of_samples = num;
}

int MyOpenGLWidget::getNumOfSamples() const {
    return num_of_samples;
}

void MyOpenGLWidget::setSamplingMode(MyOpenGLWidget::SamplingMode mode) {
    sampling_mode = mode;
}

MyOpenGLWidget::SamplingMode MyOpenGLWidget::getSamplingMode() const {
    return sampling_mode;
}

void MyOpenGLWidget::enableTransparency(bool enabled) {
    transparency_enabled = enabled;
}

bool MyOpenGLWidget::transparencyEnabled() const {
    return transparency_enabled;
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
    gl->glClear(GL_COLOR_BUFFER_BIT);

    if (!program) {
        return;
    }    

    QMatrix4x4 rotate;
    rotate.rotate(rotation_y_angle, QVector3D(0.0f, 1.0f, 0.0f));
    rotate.rotate(rotation_x_angle, QVector3D(1.0f, 0.0f, 0.0f));

    auto model_m = rotate * model_matrix;

    program->bind();

    gl->glActiveTexture(GL_TEXTURE0);
    program->setUniformValue(program->uniformLocation("jitter"), 0);
    jitter.bind();

    gl->glActiveTexture(GL_TEXTURE1);
    program->setUniformValue(program->uniformLocation("randoms"), 1);
    randoms.bind();

    program->setUniformValue(program->uniformLocation("jitterSize"), jitter_size);
    program->setUniformValue(program->uniformLocation("randomsSize"), randoms_size);

    program->setUniformValue(program->uniformLocation("numOfSamples"), num_of_samples);
    program->setUniformValue(program->uniformLocation("samplingMode"), int(sampling_mode));
    program->setUniformValue(program->uniformLocation("numOfSteps"), num_of_steps);    
    program->setUniformValue(program->uniformLocation("refractionEnabled"), transparency_enabled);

    const auto num_of_spheres = static_cast<int>(scene.objects.size());
    program->setUniformValue(program->uniformLocation("numOfSpheres"), num_of_spheres);
    int cnt = 0;
    for (const auto &s: scene.objects) {
        program->setUniformValue(program->uniformLocation(QString("spheres[%1].position").arg(cnt)), model_m * s.position);
        program->setUniformValue(program->uniformLocation(QString("spheres[%1].radius").arg(cnt)), static_cast<GLfloat>(s.radius));
        program->setUniformValue(program->uniformLocation(QString("spheres[%1].materialId").arg(cnt)), s.materialId);
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

    cnt = 0;
    for (const auto &m: scene.materials) {
        program->setUniformValue(program->uniformLocation(QString("materials[%1].diffuse").arg(cnt)), m.diffuse);
        program->setUniformValue(program->uniformLocation(QString("materials[%1].specular").arg(cnt)), m.specular);
        program->setUniformValue(program->uniformLocation(QString("materials[%1].shininess").arg(cnt)), m.shininess);
        program->setUniformValue(program->uniformLocation(QString("materials[%1].refractionCoeff").arg(cnt)), m.refractionCoeff);
        program->setUniformValue(program->uniformLocation(QString("materials[%1].refractionIndex").arg(cnt)), m.refractionIndex);
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
}

void MyOpenGLWidget::clearScene() {
    scene.clear();
}

void MyOpenGLWidget::addRandomObject() {
    ::addRandomObject(scene);
}
