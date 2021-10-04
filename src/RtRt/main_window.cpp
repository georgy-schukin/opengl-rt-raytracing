#include "main_window.h"
#include "ui_main_window.h"

#include <QStatusBar>
#include <QToolBar>
#include <QFileDialog>
#include <QMessageBox>
#include <QColorDialog>
#include <QSettings>
#include <QSlider>
#include <QLineEdit>

#include <cmath>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    gl_widget = ui->openGLWidget;
    connect(gl_widget, &MyOpenGLWidget::initialized, this, &MainWindow::initGlWidget);

    initMenu();
    initStatusbar();
    initToolbar();

    default_title = windowTitle();
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::initMenu() {
}

void MainWindow::initStatusbar() {
    ui->statusBar->setVisible(false);
}

void MainWindow::initToolbar() {
    ui->mainToolBar->setVisible(false);
}

void MainWindow::initGlWidget() {
    initSettings();
}

void MainWindow::initSettings() {
}

void MainWindow::resetSettings() {
}

void MainWindow::on_actionAbout_triggered() {
    QString about =
            QString("Real Time Ray Tracing v1.0<br>") +
            QString("Programmed by Georgy Schukin<br>") +
            QString("<a href='mailto:georgy.schukin@gmail.com'>georgy.schukin@gmail.com</a>");
    QMessageBox::information(this, "About", about);
}

void MainWindow::on_actionExit_triggered() {
    qApp->exit();
}

void MainWindow::showError(QString message) {
    QMessageBox::critical(this, "Error", message);
}
