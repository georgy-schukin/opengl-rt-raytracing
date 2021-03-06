#pragma once

#include "gl_objects/gl_plane.h"
#include "objects/scene.h"

#include <QOpenGLWidget>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLBuffer>
#include <QMatrix4x4>
#include <QOpenGLTexture>
#include <QOpenGLShaderProgram>
#include <QTimer>
#include <memory>

class MyOpenGLWidget : public QOpenGLWidget {
    Q_OBJECT


public:
    enum SamplingMode : int {
        SM_RANDOM = 0,
        SM_MULTIJITTERED = 1
    };

public:
    explicit MyOpenGLWidget(QWidget *parent=nullptr);

    void setBackgroundColor(QColor color);
    QColor getBackgroundColor() const;

    void setIterationLimit(int limit);
    int getIterationLimit() const;

    void setNumOfSamples(int num);
    int getNumOfSamples() const;

    void setSamplingMode(SamplingMode mode);
    SamplingMode getSamplingMode() const;

    void enableTransparency(bool enabled);
    bool transparencyEnabled() const;

    void randomScene();
    void clearScene();
    void addRandomObject();

signals:
    void initialized();

protected:
    virtual void initializeGL() override;
    virtual void resizeGL(int width, int height) override;
    virtual void paintGL() override;

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;

private:
    std::shared_ptr<QOpenGLShaderProgram> loadProgram(QString vertex_shader_file, QString fragment_shader_file);

    void initScene();
    void initView();
    void initTextures();

    void onTimer();

private:
    std::shared_ptr<QOpenGLShaderProgram> program;

    QMatrix4x4 model_matrix, view_matrix, projection_matrix;
    QVector3D eye = QVector3D(-10.0f, 0.0f, -10.0f);
    float cameraFOV = 45.0f;

    float rotation_y_angle {0.0f}, rotation_x_angle {0.0f};

    QPoint mouse_pos {0, 0};

    QColor background_color {0, 0, 0};

    std::shared_ptr<GLPlane> plane;

    Scene scene;

    int num_of_steps = 5;
    int num_of_samples = 1;
    SamplingMode sampling_mode = SM_RANDOM;

    QOpenGLTexture jitter;
    int jitter_size = 1;

    QOpenGLTexture randoms;
    int randoms_size= 1;

    bool transparency_enabled = false;
};
