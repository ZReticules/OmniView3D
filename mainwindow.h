#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSettings>
#include <QDoubleSpinBox>
#include <QToolButton>
#include "fileparser.h"
#include "ui_mainwindow.h"
#include "glwidget.h"
#include "lightdialog.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class NonTogglingButton : public QToolButton {
    Q_OBJECT
public:
    using QToolButton::QToolButton;
protected:
    void nextCheckState() override {}
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private: // types
    class LineWidget : public QFrame{
    public:
        LineWidget(QWidget *parent=nullptr, Shape shape = QFrame::VLine) : QFrame(parent){
            setFrameShape(shape);
            setFrameShadow(QFrame::Sunken);
        };
        LineWidget(Shape shape) : LineWidget(nullptr, shape){};
    };
    enum PositionState {
        ABSOLUTE,
        GROUNDED,
        ORIGIN,
        DEFAULT
    };
    enum DragState {
        NO_DRAG,
        LEFT_DRAG,
        RIGHT_DRAG
    };

private: // fields
    Ui::MainWindow *ui;
    QColor bgColor = QColor(85, 0, 0);
    QColor edgeColor = QColor(255, 255, 127);
    QColor faceColor = QColor(85, 85, 0);
    GLWidget *glWidget;
    QDoubleSpinBox *scaleSpin;
    NonTogglingButton *lightButton;
    LightDialog *lightDialog;
    LightDialog::LightContext lightContext, oldContext;
    // QVector3D absoluteBase, groundedBase, originBase;
    QVector3D lastHit;
    PositionState currentBase = ABSOLUTE;
    QVector3D bases[4] = {
        QVector3D(0, 0, 0),
        QVector3D(0, 0, 0),
        QVector3D(0, 0, 0),
        QVector3D(0, 0, 0)
    };

    QString fpath = "no file";
    QString filter = "";

    float m_scale = 1.0, m_rotation = 0.0, m_tilt = 0.0;
    float dragSensitivity = 0.25f;
    DragState dragState = NO_DRAG;
    float startDragAngle;
    QPoint lastDragPos;
    float scaleStep = 0.1;
    bool firstShow = true;

    QSettings settings;

    static constexpr float inf = std::numeric_limits<float>::infinity();
    static constexpr QVector3D InfVector = QVector3D(inf, inf, inf);

    static const std::map<QString, FileParser::ParserFunction> parsers;
    static const QString fopenFilters;

private: // methods
    bool loadFile(const QString& path, const QString &filter);
    void loadTriangles(const std::vector<float>&);

    std::pair<double, double> updateState();

    QVector3D getWorldPointUnderCursor(const QPoint& screenPos);

    inline std::function<void(bool checked)> positionHandler(PositionState t);
    template <typename T>
    inline std::function<void(bool checked)> colorHandler(T *widget, QColor *color,
                                                          void (GLWidget::*setColor)(QColor color), const QString& title);

    inline float scale(){ return m_scale; };
    void setScale(float);
    inline float rotation(){ return m_rotation; }
    void setRotation(float);
    inline float tilt(){ return m_tilt; }
    void setTilt(float);

    void loadLightContext();

protected:
    void showEvent(QShowEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
    void loadSettings();
    void storeSettings();

    void setupUi();
};

#endif // MAINWINDOW_H
