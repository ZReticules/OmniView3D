#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QDebug>
#include <QTimer>
#include <QTime>
#include <QWheelEvent>
#include <QScrollBar>
#include <QAction>
#include <functional>
#include <QFileInfo>
#include <QFileDialog>
#include <QColorDialog>
#include <QToolButton>
#include <QStyle>
#include <QPushButton>

#include "lightdialog.h"
#include "fileparser.h"
#include "glwidget.h"
#include "Convert3D.h"
#include "IconTools.h"

using namespace IconTools;

using std::vector;

const std::map<QString, FileParser::ParserFunction> MainWindow::parsers = {
    {"Binary STL (*.stl)", FileParser::ParseStlGL},
    {"ASCII STL (*.stl)", FileParser::ParseStlAsciiGL},
    {"ASCII Raw triangles (*.*)", FileParser::ParseTxtGL}
};

const QString MainWindow::fopenFilters = []()->QString{
    QString result = "";
    for(const auto& [key, val] : parsers)
        result += key + ";;";
    result.truncate(result.length() - 2);
    return result;
}();

inline std::function<void(bool checked)> MainWindow::positionHandler(MainWindow::PositionState state)
{
    return [this, state] (bool checked){
        if(this->currentBase == state)
            return;
        this->currentBase = state;
        this->glWidget->setCenter(this->bases[this->currentBase]);
        this->glWidget->update();
    };
}

template <typename T>
inline std::function<void(bool checked)> MainWindow::colorHandler(
    T *widget, QColor *color,
    void (GLWidget::*setColor)(QColor color),
    const QString& title
)
{
    auto colorDialog = new QColorDialog(this);
    colorDialog->setWindowIcon(QIcon::fromTheme(QIcon::ThemeIcon::DocumentProperties));

    QObject::connect(colorDialog, &QColorDialog::currentColorChanged, [this, widget, color, setColor](const QColor& newColor){
        IconFill(widget, newColor);
        (this->glWidget->*setColor)(newColor);
        this->glWidget->update();
    });

    QObject::connect(colorDialog, &QColorDialog::accepted, [this, color, colorDialog](){
        *color = colorDialog->currentColor();
    });

    QObject::connect(colorDialog, &QColorDialog::rejected, [this, widget, color, setColor](){
        IconFill(widget, *color);
        (this->glWidget->*setColor)(*color);
        this->glWidget->update();
    });

    return [this, color, colorDialog] (bool checked){
        colorDialog->setCurrentColor(*color);
        colorDialog->show();
    };
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , lightContext(GLWidget::SPECULAR, QColor(255, 255, 255), 0.2, 16.0, 1.0, 45.0, 0.5)
    , oldContext(lightContext)
{
    ui->setupUi(this);
    lightDialog = new LightDialog(this, lightContext);
    glWidget = new GLWidget(this);
    scaleSpin = new QDoubleSpinBox;
    lightButton = new NonTogglingButton;
    currentBase = DEFAULT;
    this->loadSettings();
    setupUi();
}

void MainWindow::loadLightContext()
{
    this->glWidget->setLightMode(this->lightContext.mode);
    this->glWidget->setLightAngle(this->lightContext.angle);
    this->glWidget->setLightHeight(this->lightContext.height);
    this->lightButton->setChecked(glWidget->lightMode());
    this->glWidget->setAmbientStrengt(this->lightContext.ambient);
    this->glWidget->setShininess(this->lightContext.shininess);
    this->glWidget->setSpecularStrength(this->lightContext.specularStrength);
    this->glWidget->setLightColor(this->lightContext.color);
}

void MainWindow::setupUi()
{
    this->ui->glContainer->layout()->addWidget(this->glWidget);
    this->ui->actionAs_is->setIconVisibleInMenu(true);

    QObject::connect(this->lightDialog, &LightDialog::modeSwitched, [this](GLWidget::LightMode mode){
        if(this->lightContext.mode != mode)
        {
            this->lightContext.mode = mode;
            this->glWidget->setLightMode(mode);
            this->lightButton->setChecked(mode);
            this->glWidget->update();
        }
    });

    QObject::connect(this->lightDialog, &LightDialog::angleChanged, [this](float angle){
        if(this->lightContext.angle != angle)
        {
            this->lightContext.angle = angle;
            this->glWidget->setLightAngle(angle);
            this->glWidget->update();
        }
    });

    QObject::connect(this->lightDialog, &LightDialog::colorChanged, [this](QColor color){
        if(this->lightContext.color != color)
        {
            this->lightContext.color = color;
            this->glWidget->setLightColor(color);
            this->glWidget->update();
        }
    });

    QObject::connect(this->lightDialog, &LightDialog::ambientChanged, [this](float ambient){
        if(this->lightContext.ambient != ambient)
        {
            this->lightContext.ambient = ambient;
            this->glWidget->setAmbientStrengt(ambient);
            this->glWidget->update();
        }
    });

    QObject::connect(this->lightDialog, &LightDialog::shininessChanged, [this](float shininess){
        if(this->lightContext.shininess != shininess)
        {
            this->lightContext.shininess = shininess;
            this->glWidget->setShininess(shininess);
            this->glWidget->update();
        }
    });

    QObject::connect(this->lightDialog, &LightDialog::specularChanged, [this](float specularStrength){
        if(this->lightContext.specularStrength != specularStrength)
        {
            this->lightContext.specularStrength = specularStrength;
            this->glWidget->setSpecularStrength(specularStrength);
            this->glWidget->update();
        }
    });

    QObject::connect(this->lightDialog, &LightDialog::heightChanged, [this](float height){
        if(this->lightContext.height != height)
        {
            this->lightContext.height = height;
            this->glWidget->setLightHeight(height);
            this->glWidget->update();
        }
    });

    QObject::connect(this->lightDialog, &QDialog::accepted, [this](){
        this->oldContext = this->lightContext;
    });

    QObject::connect(this->lightDialog, &QDialog::rejected, [this](){
        this->lightContext = this->oldContext;
        this->loadLightContext();
        this->glWidget->update();
    });

    // this->ui->actionToolbar->setIcon(this->style()->standardIcon(QStyle::SP_DialogApplyButton));
    QObject::connect(this->ui->actionToolbar, &QAction::triggered, [this, lastChecked=false](bool checked=false) mutable {
        this->ui->actionToolbar->blockSignals(true);
        lastChecked = !lastChecked;
        this->ui->toolBar->setVisible(lastChecked);
        this->ui->actionToolbar->setIconVisibleInMenu(lastChecked);
        this->ui->actionToolbar->blockSignals(false);
    });

    QObject::connect(this->ui->toolBar, &QToolBar::visibilityChanged, [this](bool visible){
        emit this->ui->actionToolbar->triggered();
    });

    auto facesColorButton = new QToolButton;
    IconFill(facesColorButton, faceColor);
    facesColorButton->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    facesColorButton->setText("Faces");
    QObject::connect(facesColorButton, &QToolButton::clicked,
                     this->colorHandler(facesColorButton, &this->faceColor, &GLWidget::setFaceColor, "Faces color"));
    this->ui->toolBar->addWidget(facesColorButton);

    auto edgesColorButton = new QToolButton;
    IconFill(edgesColorButton, edgeColor);
    edgesColorButton->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    edgesColorButton->setText("Edges");
    QObject::connect(edgesColorButton, &QToolButton::clicked,
                     this->colorHandler(edgesColorButton, &this->edgeColor, &GLWidget::setEdgeColor, "Edges color"));
    this->ui->toolBar->addWidget(edgesColorButton);

    auto bgColorButton = new QToolButton;
    IconFill(bgColorButton, bgColor);
    bgColorButton->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    bgColorButton->setText("Back");
    QObject::connect(bgColorButton, &QToolButton::clicked,
                     this->colorHandler(bgColorButton, &this->bgColor, &GLWidget::setBgColor, "Background color"));
    this->ui->toolBar->addWidget(bgColorButton);

    this->ui->toolBar->addWidget(new LineWidget(QFrame::VLine));

    auto axesButton = new QToolButton;
    axesButton->setIcon(QIcon::fromTheme(QIcon::ThemeIcon::ListAdd));
    axesButton->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    axesButton->setText("Axes");
    axesButton->setCheckable(true);
    axesButton->setChecked(glWidget->isAxesUsed());
    axesButton->setToolTip("Show axes");
    QObject::connect(axesButton, &QToolButton::clicked, [this](bool checked){
        this->glWidget->useAxes(checked);
        this->glWidget->update();
    });
    this->ui->toolBar->addWidget(axesButton);

    auto perspectiveButton = new QToolButton;
    perspectiveButton->setIcon(QIcon::fromTheme(QIcon::ThemeIcon::ZoomFitBest));
    perspectiveButton->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    perspectiveButton->setText("Perspective");
    perspectiveButton->setCheckable(true);
    perspectiveButton->setChecked(glWidget->isPerspectiveUsed());
    perspectiveButton->setToolTip("Use perspective");
    QObject::connect(perspectiveButton, &QToolButton::clicked, [this](bool checked){
        this->glWidget->usePerspective(checked);
        this->glWidget->update();
    });
    this->ui->toolBar->addWidget(perspectiveButton);

    auto edgesButton = new QToolButton;
    edgesButton->setIcon(QIcon::fromTheme(QIcon::ThemeIcon::MediaPlaybackStop));
    edgesButton->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    edgesButton->setText("Edges");
    edgesButton->setCheckable(true);
    edgesButton->setChecked(glWidget->isEdgesUsed());
    edgesButton->setToolTip("Show edges");
    QObject::connect(edgesButton, &QToolButton::clicked, [this](bool checked){
        this->glWidget->useEdges(checked);
        this->glWidget->update();
    });
    this->ui->toolBar->addWidget(edgesButton);

    auto facesButton = new QToolButton;
    facesButton->setIcon(QIcon::fromTheme(QIcon::ThemeIcon::AppointmentNew));
    facesButton->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    facesButton->setText("Faces");
    facesButton->setCheckable(true);
    facesButton->setChecked(glWidget->isFacesUsed());
    facesButton->setToolTip("Show faces");
    QObject::connect(facesButton, &QToolButton::clicked, [this](bool checked){
        this->glWidget->useFaces(checked);
        this->glWidget->update();
    });
    this->ui->toolBar->addWidget(facesButton);

    this->ui->toolBar->addWidget(new LineWidget(QFrame::VLine));

    auto spinWid = new QWidget;
    auto spinLay = new QHBoxLayout;
    spinLay->setSpacing(0);
    spinWid->setLayout(spinLay);
    auto scaleLab = new QLabel("Scale");
    spinLay->addWidget(scaleLab, 0, Qt::AlignCenter);
    spinLay->addWidget(this->scaleSpin);
    this->scaleSpin->setMinimum(0.1);
    this->scaleSpin->setMaximum(1000.0);
    this->scaleSpin->setSingleStep(this->scaleStep);
    this->scaleSpin->setStepType(QAbstractSpinBox::StepType::AdaptiveDecimalStepType);
    QObject::connect(this->scaleSpin, &QDoubleSpinBox::valueChanged, [this](double val){
        if(val == this->scale())
            return;
        this->scaleSpin->blockSignals(true);
        this->glWidget->setScale(val);
        this->glWidget->update();
        this->scaleSpin->blockSignals(false);
    });
    this->ui->toolBar->addWidget(spinWid);

    this->ui->toolBar->addWidget(new LineWidget(QFrame::VLine));

    lightButton->setIcon(QIcon::fromTheme(QIcon::ThemeIcon::WeatherClear));
    lightButton->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    lightButton->setText("Light");
    lightButton->setCheckable(true);
    lightButton->setChecked(glWidget->lightMode());
    lightButton->setToolTip("Change light settings");
    QObject::connect(lightButton, &QToolButton::clicked, [this](bool checked){
        this->lightDialog->show();
    });
    this->ui->toolBar->addWidget(lightButton);

    this->ui->toolBar->addWidget(new LineWidget(QFrame::VLine));

    auto posButton = new QPushButton;
    auto posMenu = new QMenu;
    posButton->setMenu(posMenu);
    posButton->setText("positioning");

    auto centerAction = new QAction("absolute center");
    centerAction->setToolTip("Place the model in the center");
    QObject::connect(centerAction, &QAction::triggered, this->positionHandler(ABSOLUTE));
    posMenu->addAction(centerAction);

    auto groundAction = new QAction("ground center");
    groundAction->setToolTip("Place the model in the center over ground plane");
    QObject::connect(groundAction, &QAction::triggered, this->positionHandler(GROUNDED));
    posMenu->addAction(groundAction);

    auto originAction = new QAction("coordinates origin");
    originAction->setToolTip("Place model in the coordinates origin");
    QObject::connect(originAction, &QAction::triggered, this->positionHandler(ORIGIN));
    posMenu->addAction(originAction);

    auto defaultPosAction = new QAction("default");
    defaultPosAction->setToolTip("Place the model in the default coordinates(as in the file)");
    QObject::connect(defaultPosAction, &QAction::triggered,
                     this->positionHandler(DEFAULT));
    posMenu->addAction(defaultPosAction);

    this->ui->toolBar->addWidget(posButton);

    QObject::connect(this->ui->actionLight_settings, &QAction::triggered, [this, lastChecked=false](bool checked=false) {
        if(this->lightDialog->isHidden())
            this->lightDialog->show();
        else
            this->lightDialog->close();
    });

    QObject::connect(this->ui->actionOpen_file, &QAction::triggered, [this](bool checked=false){
        auto selectedFilter = QString();
        auto fileName = QFileDialog::getOpenFileName(this, "Open file to extract model", "", this->fopenFilters, &selectedFilter);
        this->loadFile(fileName, selectedFilter);
    });

    this->ui->labelRotY->setText(QString::asprintf("Rotation: %6.1f°", this->glWidget->rotAngle()));
    QObject::connect(this->ui->sliderRotY, &QSlider::valueChanged, [this](int value){
        if(this->dragState != NO_DRAG)
            return;
        this->ui->sliderRotY->blockSignals(true);
        this->setRotation(-((float)value / 10 - 180));
        this->glWidget->update();
        this->ui->sliderRotY->blockSignals(false);
    });

    this->ui->labelTilt->setText(QString::asprintf("Tilt: %6.1f°", this->glWidget->tiltAngle()));
    QObject::connect(this->ui->sliderTilt, &QSlider::valueChanged, [this](int value){
        if(this->dragState != NO_DRAG)
            return;
        this->ui->sliderTilt->blockSignals(true);
        this->setTilt(-((float)value / 10 - 180));
        this->glWidget->update();
        this->ui->sliderTilt->blockSignals(false);
    });

    this->glWidget->mousePressHandler = [this](QMouseEvent *event){
        this->lastDragPos = event->pos();
        this->startDragAngle = this->glWidget->tiltAngle();
        switch(event->button())
        {
        case Qt::MouseButton::LeftButton:
            this->dragState = LEFT_DRAG;
            break;
        case Qt::MouseButton::RightButton:
            do {
                this->dragState = RIGHT_DRAG;
                auto [rotDeg, tiltDeg] = this->glWidget->cameraAngles();
                this->lastHit = Convert3D::ToPerspective(
                    event->pos(), this->glWidget->size(),
                    this->glWidget->invPvm(), 0
                    );
            } while(0);
            break;
        default:;
        }
        return;
    };

    this->glWidget->mouseMoveHandler = [this](QMouseEvent *event){
        switch(this->dragState)
        {
        case LEFT_DRAG:
            do {
                QPoint delta = event->pos() - this->lastDragPos;

                float deltaRot = -delta.x() * this->dragSensitivity;
                if(this->startDragAngle >= 90 || this->startDragAngle <= -90)
                    deltaRot = -deltaRot;
                float deltaTilt = delta.y() * this->dragSensitivity;

                this->glWidget->shiftCameraAngles(deltaRot, deltaTilt);
                this->setRotation(this->glWidget->rotAngle());
                this->setTilt(this->glWidget->tiltAngle());

                this->lastDragPos = event->pos();
                this->glWidget->update();
                return;
            } while(0);
            break;
        case RIGHT_DRAG:
            do {
                QVector3D localDelta;
                if(this->glWidget->isPerspectiveUsed())
                {
                    auto newHit = Convert3D::ToPerspective(
                        event->pos(), this->glWidget->size(),
                        this->glWidget->invPvm(), 0
                        );
                    localDelta = this->lastHit - newHit;
                    this->lastHit = newHit;
                }
                else
                {
                    QPoint lastPos = this->lastDragPos, curPos = event->pos();
                    this->lastDragPos = event->pos();

                    auto [rotDeg, tiltDeg] = this->glWidget->cameraAngles();
                    float sizeScale = std::min(this->glWidget->height(), this->glWidget->width());
                    QPoint delta = curPos - lastPos;
                    localDelta = Convert3D::ToOrtho(
                        delta, this->glWidget->cameraDistance(),
                        sizeScale, rotDeg, tiltDeg
                        );
                }
                this->glWidget->setCenter(this->glWidget->center() + localDelta);
                this->glWidget->update();
            } while(0);
            break;
        default:
            break;
        }
    };

    this->glWidget->mouseReleaseHandler = [this](QMouseEvent *event){
        this->dragState = NO_DRAG;
    };
}

void MainWindow::setScale(float scale)
{
    if(scale < 0.1)
        scale = 0.1;
    if(scale > 1000.0)
        scale = 1000.0;
    this->m_scale = scale;
    this->scaleSpin->setValue(scale);
    this->glWidget->setScale(this->m_scale);
}

void MainWindow::setRotation(float rot)
{
    this->m_rotation = rot;
    auto [_, tiltAngle] = this->glWidget->cameraAngles();
    this->ui->sliderRotY->setValue((-rot + 180) * 10);
    this->ui->labelRotY->setText(QString::asprintf("Rotation: %6.1f°", rot));
    this->glWidget->setCameraAngles(rot, tiltAngle);
}

void MainWindow::setTilt(float tilt)
{
    this->m_tilt = tilt;
    auto [rotAngle, _] = this->glWidget->cameraAngles();
    this->ui->sliderTilt->setValue((-tilt + 180) * 10);
    this->ui->labelTilt->setText(QString::asprintf("Tilt: %6.1f°", tilt));
    this->glWidget->setCameraAngles(rotAngle, tilt);
}

std::pair<double, double> MainWindow::updateState()
{
    this->loadLightContext();
    const int rotMaxVal = this->ui->sliderRotY->maximum();
    const int tiltMaxVal = this->ui->sliderTilt->maximum();
    int rotVal = this->ui->sliderRotY->value();
    int tiltVal = this->ui->sliderTilt->value();
    auto rotAngle = -(rotVal / 10 - 180.0);
    auto tiltAngle = -(tiltVal / 10 - 180.0);
    this->glWidget->setCameraAngles(rotAngle, tiltAngle);
    this->glWidget->update();
    return {rotAngle, tiltAngle};
}

void MainWindow::loadTriangles(const vector<float>& triangles)
{
    this->glWidget->setData(triangles);
    auto maxVec = QVector3D(-INFINITY, -INFINITY, -INFINITY);
    auto minVec = QVector3D(INFINITY, INFINITY, INFINITY);
    for(int i = 0; i < triangles.size() / 3; i++)
        for(int j = 0; j < 3; j++)
        {
            if(triangles[i * 3 + j] > maxVec[j])
                maxVec[j] = triangles[i * 3 + j];
            if(triangles[i * 3 + j] < minVec[j])
                minVec[j] = triangles[i * 3 + j];
        }
    // qDebug() << maxVec;
    // qDebug() << minVec - maxVec;
    auto diffVec = maxVec - minVec;
    auto figureCenter = minVec + diffVec / 2;
    double maxDiff = -INFINITY;
    for(int i = 0; i < 3; i++)
        if(maxDiff < diffVec[i])
            maxDiff = diffVec[i];

    this->bases[ABSOLUTE] = figureCenter;
    figureCenter[1] = minVec[1];
    this->bases[GROUNDED] = figureCenter;
    this->bases[ORIGIN] = minVec;
    this->glWidget->setCenter(bases[this->currentBase]);

    this->setScale(this->glWidget->axisLen() / maxDiff);
    this->ui->labelPolygonsCount->setText(QString::asprintf("Polygons : %lld", triangles.size() / 9));
}

bool MainWindow::loadFile(const QString& path, const QString& filter)
{
    QFileInfo finfo(path);
    if(!finfo.exists() || !finfo.isFile())
        return false;
    auto triangles = vector<float>();
    if(!this->parsers.contains(filter))
        return false;
    const auto& parser = this->parsers.at(filter);
    if(!parser(triangles, path.toStdWString(), true))
        return false;
    if(!triangles.size())
        return false;
    this->filter = filter;
    this->fpath = path;
    this->loadTriangles(triangles);
    this->glWidget->update();
    this->ui->fnameLabel->setText(finfo.fileName());
    return true;
}

void MainWindow::showEvent(QShowEvent* event)
{
    this->QMainWindow::showEvent(event);
    if(this->firstShow)
    {
        this->firstShow = false;
        this->glWidget->setBgColor(this->bgColor);
        this->glWidget->setEdgeColor(this->edgeColor);
        this->glWidget->setFaceColor(this->faceColor);

        float oldScale = this->scale();
        QVector3D oldCenter = this->glWidget->center();
        if(this->fpath == "no file" || !this->loadFile(this->fpath, this->filter))
        {
            this->loadTriangles(vector<float>{
                0.0f,  1.0f,  0.0f,
                0.943f, -0.333f, 0.0f,
                -0.471f, -0.333f, 0.816f,

                0.0f,  1.0f,  0.0f,
                -0.471f, -0.333f, 0.816f,
                -0.471f, -0.333f, -0.816f,

                0.0f,  1.0f,  0.0f,
                -0.471f, -0.333f, -0.816f,
                0.943f, -0.333f, 0.0f,

                0.943f, -0.333f, 0.0f,
                -0.471f, -0.333f, -0.816f,
                -0.471f, -0.333f, 0.816f
            });
            oldScale = -1.0;
            oldCenter = InfVector;
        }
        if(oldScale != -1.0)
            this->setScale(oldScale);
        if(oldCenter != InfVector)
            this->glWidget->setCenter(oldCenter);
        this->loadLightContext();
    }
}

void MainWindow::wheelEvent(QWheelEvent *event)
{
    double increase = this->scaleStep;
    if(event->modifiers() & Qt::ControlModifier)
        increase *= 10;
    if(event->modifiers() & Qt::ShiftModifier)
        increase *= 100;
    if (event->angleDelta().y() < 0)
        increase = -increase;
    this->setScale(this->scale() + increase);
}

void MainWindow::loadSettings()
{
    this->oldContext.loadSettings(this->settings);
    this->lightContext = this->oldContext;
    this->faceColor = this->settings.value("view/faceColor", this->faceColor).value<QColor>();
    this->edgeColor = this->settings.value("view/edgeColor", this->edgeColor).value<QColor>();
    this->bgColor = this->settings.value("view/bgColor", this->bgColor).value<QColor>();
    this->setRotation(this->settings.value("view/rotation", 0).toFloat());
    this->setTilt(this->settings.value("view/tilt", 0).toFloat());
    this->setScale(this->settings.value("view/scale", -1.0).toFloat());
    this->glWidget->useFaces(settings.value("view/facesUsed", this->glWidget->isFacesUsed()).toBool());
    this->glWidget->useEdges(settings.value("view/edgesUsed", this->glWidget->isEdgesUsed()).toBool());
    this->glWidget->useAxes(settings.value("view/axesUsed", this->glWidget->isAxesUsed()).toBool());
    this->glWidget->usePerspective(settings.value("view/perspectiveUsed", this->glWidget->isPerspectiveUsed()).toBool());
    this->glWidget->setCenter(settings.value("view/center", InfVector).value<QVector3D>());
    this->fpath = this->settings.value("lastFile", "no file").toString();
    this->filter = this->settings.value("lastFileFilter", "").toString();
}

void MainWindow::storeSettings()
{
    this->oldContext.storeSettings(this->settings);
    this->settings.setValue("view/faceColor", this->faceColor);
    this->settings.setValue("view/edgeColor", this->edgeColor);
    this->settings.setValue("view/bgColor", this->bgColor);
    this->settings.setValue("view/rotation", this->rotation());
    this->settings.setValue("view/tilt", this->tilt());
    this->settings.setValue("view/scale", this->scale());
    this->settings.setValue("view/facesUsed", this->glWidget->isFacesUsed());
    this->settings.setValue("view/edgesUsed", this->glWidget->isEdgesUsed());
    this->settings.setValue("view/axesUsed", this->glWidget->isAxesUsed());
    this->settings.setValue("view/perspectiveUsed", this->glWidget->isPerspectiveUsed());
    this->settings.setValue("view/center", this->glWidget->center());
    this->settings.setValue("lastFile", this->fpath);
    this->settings.setValue("lastFileFilter", this->filter);
    this->settings.sync();
}

MainWindow::~MainWindow()
{
    this->storeSettings();
    delete ui;
}
