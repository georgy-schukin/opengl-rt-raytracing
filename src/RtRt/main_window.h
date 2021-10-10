#pragma once

#include <QMainWindow>
#include <QOpenGLFunctions>
#include <QVector3D>
#include <QLabel>
#include <QSlider>
#include <QSpinBox>

#include <vector>
#include <memory>

namespace Ui {
class MainWindow;
}

class MyOpenGLWidget;

template <typename T>
class Frame3D;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
    void initGlWidget();

    void on_actionAbout_triggered();

    void on_actionExit_triggered();

    void on_actionShow_Toolbar_triggered();

    void on_actionRandom_Scene_triggered();

private:
    void initMenu();
    void initStatusbar();
    void initToolbar();

    void initSettings();
    void resetSettings();

    void showError(QString message);

private:
    Ui::MainWindow *ui;
    MyOpenGLWidget *gl_widget;
    QString default_title;

    QSpinBox *steps;
};

