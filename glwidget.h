#ifndef GLWIDGET_H
#define GLWIDGET_H

#include <QOpenGLFunctions>
#include <functional>
#include <QMatrix4x4>
#include <QOpenGLBuffer>
#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLWidget>
#include <qvectornd.h>

class GLWidget : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT

public:
    enum LightMode{ OFF=0, FLAT, SPECULAR };

public:
    std::function<void(QMouseEvent *event)> mousePressHandler = [](QMouseEvent *event){};
    std::function<void(QMouseEvent *event)> mouseMoveHandler = [](QMouseEvent *event){};
    std::function<void(QMouseEvent *event)> mouseReleaseHandler = [](QMouseEvent *event){};

private:
    QVector3D position = QVector3D(0, 0, 5), direction = QVector3D(0, 1, 0);
    QMatrix4x4 projection_, m_view, m_model, m_pvm, m_invPvm;
    int baseHeight = 0;
    const float baseDistance = 100.0;
    float scaleDistance = 1.0, aspectRatio = 1.0, scaleSize = 1.0;
    bool perspective = false, needAxes = true, needFaces = true, needEdges = true;

    QVector3D m_center = QVector3D(0, 0, 0);
    QOpenGLBuffer m_vbo, m_vboAxes; // vertex buffer object
    QOpenGLVertexArrayObject m_vao, m_vaoAxes;
    QOpenGLShaderProgram m_prog, m_progAxes;
    std::array<float, 3 * 2 * (3 + 3)> axesData = {
        // X1, Y1, Z1,  R, G, B
        -axisLen(), 0.0f, 0.0f,  1.0f, 0.0f, 0.0f,  // Начало оси X (красный)
        axisLen(), 0.0f, 0.0f,  1.0f, 0.0f, 0.0f,  // Конец оси X

        0.0f, -axisLen(), 0.0f,  0.0f, 1.0f, 0.0f,  // Начало оси Y (зеленый)
        0.0f, axisLen(), 0.0f,  0.0f, 1.0f, 0.0f,  // Конец оси Y

        0.0f, 0.0f, -axisLen(),  0.0f, 0.0f, 1.0f,  // Начало оси Z (синий)
        0.0f, 0.0f, axisLen(),  0.0f, 0.0f, 1.0f   // Конец оси Z
    };
    int m_vertexCount = 0;

    QColor edgesColor = QColor::fromRgbF(1.0, 0.0, 0.0, 1.0);
    QColor facesColor = QColor::fromRgbF(0.0, 0.5, 0.5, 1.0);

    float m_rotAngle = 0, m_tiltAngle = 0;

    // освещение
    QColor m_lightColor = QColor::fromRgbF(1.0, 1.0, 1.0, 0.0);
    QVector3D m_lightDirection = QVector3D(0.5, 0.5, 0.5);
    float m_ambientStrength = 0.2f;
    float m_shininess = 16.0f;
    float m_specularStrength = 2.0f;
    float m_lightAngle = 45.0;
    LightMode m_lightMode = SPECULAR;

public:
    GLWidget(QWidget *parent);
    void setData(const std::vector<float>& vertices);
    ~GLWidget();

    void setCameraAngles(double rotAngle, double tiltAngle);
    void shiftCameraAngles(double rotAngle, double tiltAngle);
    inline float tiltAngle() { return m_tiltAngle; }
    inline float rotAngle() { return m_rotAngle; }
    inline std::pair<float, float> cameraAngles() { return {m_rotAngle, m_tiltAngle}; }

    void setBgColor(QColor color);
    inline void setEdgeColor(QColor color) { edgesColor = color; }
    void usePerspective(bool need);
    inline bool isPerspectiveUsed() { return perspective; }
    void setScale(float scale);
    inline float scale() { return 1 / this->scaleSize; }
    inline void useAxes(bool need) { needAxes = need; };
    inline bool isAxesUsed() { return needAxes; };
    inline void useFaces(bool need) { needFaces = need; };
    inline bool isFacesUsed() { return needFaces; };
    inline void useEdges(bool need) { needEdges = need; };
    inline bool isEdgesUsed() { return needEdges; };
    inline void setCenter(QVector3D center) { m_center = center; }
    inline QVector3D center() { return m_center; }
    inline QMatrix4x4 projection() { return projection_; }
    inline QMatrix4x4 view() { return m_view; }
    inline QMatrix4x4 model() { return m_model; }
    inline QMatrix4x4 pvm() { return m_pvm; }
    inline QMatrix4x4 invPvm() { return m_invPvm; }
    inline const float axisLen() { return baseDistance * 4 / 10; }
    inline void setFaceColor(QColor color){ facesColor = color; }
    inline float cameraDistance() { return baseDistance * scaleDistance * scaleSize; }
    inline LightMode lightMode(){ return m_lightMode; };
    inline void setLightMode(LightMode newMode){ m_lightMode = newMode; };
    inline void setLightColor(QColor color){ m_lightColor = color; };
    inline float lightAngle(){ return m_lightAngle; };
    void setLightAngle(float angle);
    inline float lightHeight(){ return m_lightDirection.y(); };
    inline void setLightHeight(float height) { m_lightDirection.setY(height); };
    inline void setAmbientStrengt(float ambientStrength){ m_ambientStrength = ambientStrength; };
    inline float ambientStrength(){ return m_ambientStrength; };
    inline void setShininess(float shininess){ m_shininess = shininess; };
    inline float shininess(){ return m_shininess; };
    inline void setSpecularStrength(float specularStrength){ m_specularStrength = specularStrength; };
    inline float specularStrength(){ return m_specularStrength; };

private:

    void updateCamera();
    void updateProjection();
    void updateView();
    void updateModel();
    void updateAxes();
    void updatePvm();

    static float ComputeSignedVolume(const std::vector<float>& vertices);

protected:
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;

    void mousePressEvent(QMouseEvent *event) override { mousePressHandler(event); };
    void mouseMoveEvent(QMouseEvent *event) override { mouseMoveHandler(event); };
    void mouseReleaseEvent(QMouseEvent *event) override { mouseReleaseHandler(event); };
};

#endif // GLWIDGET_H
