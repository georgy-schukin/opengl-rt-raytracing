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
#include <QSettings>

#include <cmath>

namespace {

    static const QString MAX_DEPTH = "max-depth";
    static const QString NUM_OF_SAMPLES = "num-of-samples";
    static const QString SAMPLING_MODE = "sampling-mode";
    static const QString BG_COLOR = "background-color";
    static const QString ENABLE_TRASNSPARENCY = "enable-transparency";
    static const QString SHOW_TOOLBAR = "show-toolbar";

    static QSettings appSettings;
}

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
    initSettings();

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
    //steps->setFocusPolicy(Qt::TabFocus);
    steps->setValue(gl_widget->getIterationLimit());
    connect(steps, qOverload<int>(&QSpinBox::valueChanged), [this](int value) {
        gl_widget->setIterationLimit(value);
        gl_widget->update();
        appSettings.setValue(MAX_DEPTH, value);
    });

    samples = new QSpinBox(this);
    samples->setMinimum(1);
    samples->setMaximum(1000);
    //samples->setFocusPolicy(Qt::TabFocus);
    samples->setValue(gl_widget->getNumOfSamples());
    connect(samples, qOverload<int>(&QSpinBox::valueChanged), [this](int value) {
        gl_widget->setNumOfSamples(value);
        gl_widget->update();
        appSettings.setValue(NUM_OF_SAMPLES, value);
    });

    sampling_mode = new QComboBox(this);
    sampling_mode->addItem("Random", MyOpenGLWidget::SamplingMode::SM_RANDOM);
    sampling_mode->addItem("Multi Jittered", MyOpenGLWidget::SamplingMode::SM_MULTIJITTERED);
    sampling_mode->setCurrentIndex((int)gl_widget->getSamplingMode());
    connect(sampling_mode, qOverload<int>(&QComboBox::currentIndexChanged), [this](int index) {
        gl_widget->setSamplingMode(static_cast<MyOpenGLWidget::SamplingMode>(index));
        gl_widget->update();
        appSettings.setValue(SAMPLING_MODE, index);
    });

    ui->mainToolBar->addWidget(new QLabel("Max depth: ", this));
    ui->mainToolBar->addWidget(steps);
    ui->mainToolBar->addWidget(new QLabel("Samples: ", this));
    ui->mainToolBar->addWidget(samples);
    ui->mainToolBar->addWidget(new QLabel("Sampling: ", this));
    ui->mainToolBar->addWidget(sampling_mode);
}

void MainWindow::initGlWidget() {
    initSettings();
}

void MainWindow::initSettings() {
    if (appSettings.contains(SHOW_TOOLBAR)) {
        ui->actionShow_Toolbar->setChecked(appSettings.value(SHOW_TOOLBAR).toBool());
    }
    if (appSettings.contains(ENABLE_TRASNSPARENCY)) {
        ui->actionEnable_Transparency->setChecked(appSettings.value(ENABLE_TRASNSPARENCY).toBool());
    }
    if (appSettings.contains(MAX_DEPTH)) {
        steps->setValue(appSettings.value(MAX_DEPTH).toInt());
    }
    if (appSettings.contains(NUM_OF_SAMPLES)) {
        samples->setValue(appSettings.value(NUM_OF_SAMPLES).toInt());
    }
    if (appSettings.contains(SAMPLING_MODE)) {
        sampling_mode->setCurrentIndex(appSettings.value(SAMPLING_MODE).toInt());
    }
    if (appSettings.contains(BG_COLOR)) {
        gl_widget->setBackgroundColor(appSettings.value(BG_COLOR).value<QColor>());
        gl_widget->update();
    }
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

void MainWindow::on_actionRandom_Scene_triggered() {
    gl_widget->randomScene();
    gl_widget->update();
}

void MainWindow::on_actionBackground_Color_triggered() {
    QColor color = QColorDialog::getColor(gl_widget->getBackgroundColor(), this,
                                          "Choose background color",
                                          QColorDialog::DontUseNativeDialog);
    if (color.isValid()) {
        gl_widget->setBackgroundColor(color);
        gl_widget->update();
        appSettings.setValue(BG_COLOR, color);
    }
}

void MainWindow::on_actionClear_Scene_triggered() {
    gl_widget->clearScene();
    gl_widget->update();
}

void MainWindow::on_actionAdd_Random_Object_triggered() {
    gl_widget->addRandomObject();
    gl_widget->update();
}

void MainWindow::on_actionShow_Toolbar_toggled(bool show) {
    ui->mainToolBar->setVisible(show);
    appSettings.setValue(SHOW_TOOLBAR, show);
}

void MainWindow::on_actionEnable_Transparency_toggled(bool enabled) {
    gl_widget->enableTransparency(enabled);
    gl_widget->update();
    appSettings.setValue(ENABLE_TRASNSPARENCY, enabled);
}
