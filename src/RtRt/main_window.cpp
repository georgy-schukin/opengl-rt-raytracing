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
    ui->actionShow_Toolbar->setChecked(true);
}

void MainWindow::initStatusbar() {
    ui->statusBar->setVisible(false);
}

void MainWindow::initToolbar() {
    steps = new QSpinBox(this);
    steps->setMinimum(1);
    steps->setMaximum(1000);
    steps->setFocusPolicy(Qt::TabFocus);
    steps->setValue(gl_widget->getIterationLimit());
    connect(steps, qOverload<int>(&QSpinBox::valueChanged), [this](int value) {
        gl_widget->setIterationLimit(value);
        gl_widget->update();
    });

    ui->mainToolBar->addWidget(new QLabel("Steps: ", this));
    ui->mainToolBar->addWidget(steps);
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

void MainWindow::on_actionShow_Toolbar_triggered() {
    ui->mainToolBar->setVisible(!ui->mainToolBar->isVisible());
}

void MainWindow::on_actionRandom_Scene_triggered() {
    gl_widget->randomScene();
}

void MainWindow::on_actionBackground_Color_triggered() {
    QColor color = QColorDialog::getColor(gl_widget->getBackgroundColor(), this,
                                          "Choose background color",
                                          QColorDialog::DontUseNativeDialog);
    if (color.isValid()) {
        gl_widget->setBackgroundColor(color);
        gl_widget->update();
    }
}
