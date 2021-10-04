#pragma once

#include <QOpenGLWidget>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLBuffer>
#include <QMatrix4x4>
#include <QOpenGLTexture>
#include <QTimer>
#include <memory>

class MyOpenGLWidget : public QOpenGLWidget {
    Q_OBJECT

public:
    explicit MyOpenGLWidget(QWidget *parent=nullptr);

    void setBackgroundColor(QColor color);
    QColor getBackgroundColor() const;

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
    void initView();

    void onTimer();

private:
    QMatrix4x4 model_matrix, view_matrix, projection_matrix;

    float rotation_y_angle {0.0f}, rotation_x_angle {0.0f};

    QPoint mouse_pos {0, 0};

    QColor background_color {0, 0, 25};
};

