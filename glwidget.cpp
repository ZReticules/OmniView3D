#include "glwidget.h"
#include <QDebug>
#include <QVector3D>
#include <cmath>

GLWidget::GLWidget(QWidget *parent) : QOpenGLWidget(parent)
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    QSurfaceFormat format;
    format.setDepthBufferSize(24);
    format.setProfile(QSurfaceFormat::NoProfile); // Для совместимости
    format.setVersion(2, 0); // Используем GLSL 1.10 (простой вариант)
    setFormat(format);
}

float GLWidget::ComputeSignedVolume(const std::vector<float>& vertices)
{
    float volume = 0.0f;
    for (size_t i = 0; i < vertices.size(); i += 9) {
        QVector3D A(vertices[i], vertices[i+1], vertices[i+2]);
        QVector3D B(vertices[i+3], vertices[i+4], vertices[i+5]);
        QVector3D C(vertices[i+6], vertices[i+7], vertices[i+8]);
        volume += QVector3D::dotProduct(A, QVector3D::crossProduct(B, C)) / 6.0f;
    }
    return volume;
}

void GLWidget::setData(const std::vector<float>& vertices)
{
    if (!context() || !context()->isValid() || !m_vao.isCreated()) {
        qWarning() << "OpenGL not ready";
        return;
    }

    this->makeCurrent();

    this->m_vertexCount = vertices.size() / 3;

    this->m_vao.bind();
    this->m_vbo.bind();

    float volume = ComputeSignedVolume(vertices);

    std::vector<float> normalizedVertices;
    normalizedVertices.reserve(vertices.size() * 2);// Хеш-функция для поиска одинаковых вершин (с учётом погрешности)

    // struct Vec3Hash {
    //     size_t operator()(const QVector3D& v) const {
    //         // Простой хеш: масштабируем и приводим к int
    //         int scale = 10000;
    //         return std::hash<int>()((int)(v.x()*scale)) ^
    //                (std::hash<int>()((int)(v.y()*scale)) << 1) ^
    //                (std::hash<int>()((int)(v.z()*scale)) << 2);
    //     }
    // };

    // std::unordered_map<QVector3D, QVector3D, Vec3Hash> normals;

    for(int i = 0; i < vertices.size(); i += 9)
    {
        QVector3D a = QVector3D(vertices[i], vertices[i + 1], vertices[i + 2]);
        QVector3D b = QVector3D(vertices[i + 3], vertices[i + 3 + 1], vertices[i + 3 + 2]);
        QVector3D c = QVector3D(vertices[i + 6], vertices[i + 6 + 1], vertices[i + 6 + 2]);

        QVector3D u = QVector3D(
            b.x() - a.x(),
            b.y() - a.y(),
            b.z() - a.z()
        );
        QVector3D v = QVector3D(
            c.x() - a.x(),
            c.y() - a.y(),
            c.z() - a.z()
        );
        QVector3D product = QVector3D::crossProduct(u, v).normalized();
        if(volume < 0.0)
            product = -product;

        // if(!normals.contains(a))
        //     normals[a] = product;
        // else
        //     normals[a] += product;
        // if(!normals.contains(b))
        //     normals[b] = product;
        // else
        //     normals[b] += product;
        // if(!normals.contains(c))
        //     normals[c] = product;
        // else
        //     normals[c] += product;
        for(int j = 0; j < 3; j++)
        {
            normalizedVertices.push_back(vertices[i + j * 3]);
            normalizedVertices.push_back(vertices[i + j * 3 + 1]);
            normalizedVertices.push_back(vertices[i + j * 3 + 2]);
            normalizedVertices.push_back(product.x());
            normalizedVertices.push_back(product.y());
            normalizedVertices.push_back(product.z());
        }
    }

    // for(auto &i : normals)
    //     i.second.normalize();
    // for(int i = 0; i < vertices.size(); i += 9)
    // {
    //     std::array<QVector3D, 3> l_vertices = {
    //         QVector3D(vertices[i], vertices[i + 1], vertices[i + 2]),
    //         QVector3D(vertices[i + 3], vertices[i + 3 + 1], vertices[i + 3 + 2]),
    //         QVector3D(vertices[i + 6], vertices[i + 6 + 1], vertices[i + 6 + 2])
    //     };
    //     for(int j = 0; j < 3; j++)
    //     {
    //         QVector3D nm = normals[l_vertices[j]];
    //         normalizedVertices.push_back(l_vertices[j].x());
    //         normalizedVertices.push_back(l_vertices[j].y());
    //         normalizedVertices.push_back(l_vertices[j].z());
    //         normalizedVertices.push_back(nm.x());
    //         normalizedVertices.push_back(nm.y());
    //         normalizedVertices.push_back(nm.z());
    //     }
    // }

    this->m_vbo.allocate(normalizedVertices.data(), normalizedVertices.size() * sizeof(float));

    this->m_vbo.release();
    this->m_vao.release();

    this->doneCurrent();
}

void GLWidget::initializeGL()
{
    this->baseHeight = this->height();

    this->initializeOpenGLFunctions();
    this->glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    this->glEnable(GL_DEPTH_TEST);

    static const char vertex_shader[] = {
        #embed "base_shader.vert" suffix(, 0) if_empty(0)
    };
    this->m_prog.addShaderFromSourceCode(QOpenGLShader::Vertex,
                                      vertex_shader);

    static const char fragment_shader[] = {
        #embed "base_shader.frag" suffix(, 0) if_empty(0)
    };
    this->m_prog.addShaderFromSourceCode(QOpenGLShader::Fragment,
                                      fragment_shader);

    this->m_prog.link();

    this->m_vao.create();
    this->m_vbo.create();

    this->m_vao.bind();
    this->m_vbo.bind();

    this->m_prog.enableAttributeArray("vertexPosition");
    this->m_prog.setAttributeBuffer("vertexPosition", GL_FLOAT, 0, 3, 6 * sizeof(float));

    this->m_prog.enableAttributeArray("vertexNormal");
    this->m_prog.setAttributeBuffer("vertexNormal", GL_FLOAT, 3 * sizeof(float), 3, 6 * sizeof(float));

    this->m_vbo.release();
    this->m_vao.release();

    static const char axes_vertex_shader[] = {
        #embed "axes_shader.vert" suffix(, 0) if_empty(0)
    };
    this->m_progAxes.addShaderFromSourceCode(QOpenGLShader::Vertex, axes_vertex_shader);

    static const char axes_fragment_shader[] = {
        #embed "axes_shader.frag" suffix(, 0) if_empty(0)
    };
    this->m_progAxes.addShaderFromSourceCode(QOpenGLShader::Fragment, axes_fragment_shader);

    this->m_progAxes.link();

    this->m_vaoAxes.create();
    this->m_vboAxes.create();

    this->m_vaoAxes.bind();
    this->m_vboAxes.bind();

    this->m_progAxes.enableAttributeArray("vertexPosition");
    this->m_progAxes.setAttributeBuffer("vertexPosition", GL_FLOAT, 0, 3, 6 * sizeof(float));
    this->m_progAxes.enableAttributeArray("vertexColor");
    this->m_progAxes.setAttributeBuffer("vertexColor", GL_FLOAT, 3 * sizeof(float), 3, 6 * sizeof(float));

    this->m_vboAxes.allocate(this->axesData.data(), this->axesData.size() * sizeof(float));

    this->m_vboAxes.release();
    this->m_vaoAxes.release();

    // this->glEnable(GL_CULL_FACE);
    // this->glCullFace(GL_FRONT);
};

void GLWidget::updateProjection()
{

    this->projection_.setToIdentity();

    if(this->perspective)
    {
        this->projection_.perspective(45.0, this->aspectRatio, 0.01f, 10000.0f);
        return;
    }


    float viewHeight = this->baseDistance * this->scaleDistance * this->scaleSize;
    float viewWidth = viewHeight * this->aspectRatio;

    // left, right, bottom, top, near, far
    this->projection_.ortho(
        -viewWidth / 2.0f,  viewWidth / 2.0f,
        -viewHeight / 2.0f, viewHeight / 2.0f,
        -10000.0f, 10000.0f
        );
}

void GLWidget::updateModel()
{
    this->m_model = {};
    this->m_model.rotate(this->m_tiltAngle, 1, 0, 0);
    this->m_model.rotate(-this->m_rotAngle, 0, 1, 0);
}

void GLWidget::updateView()
{
    this->m_view = {};
    this->m_view.lookAt(
        QVector3D(0, 0, this->cameraDistance()),
        QVector3D(0, 0, 0),
        this->direction
    );
}

void GLWidget::updatePvm()
{
    this->m_pvm = this->projection_ * this->m_view * this->m_model;
    this->m_invPvm = this->m_pvm.inverted();
}

void GLWidget::setCameraAngles(double rotAngle, double tiltAngle)
{
    this->m_rotAngle = rotAngle;
    this->m_tiltAngle = tiltAngle;
    this->updateModel();
    this->updatePvm();
}

void GLWidget::shiftCameraAngles(double rotAngle, double tiltAngle)
{
    this->m_rotAngle += rotAngle;
    if(this->m_rotAngle > 180)
        this->m_rotAngle -= 360;
    if(this->m_rotAngle < -180)
        this->m_rotAngle += 360;
    this->m_tiltAngle += tiltAngle;
    if(this->m_tiltAngle > 180)
        this->m_tiltAngle -= 360;
    if(this->m_tiltAngle < -180)
        this->m_tiltAngle += 360;
    this->updateModel();
    this->updatePvm();
}

void GLWidget::setScale(float scale)
{
    this->scaleSize = 1 / scale;
    this->updateAxes();
    this->updateView();
    this->updateProjection();
    this->updatePvm();
}

void GLWidget::usePerspective(bool need)
{
    this->perspective = need;
    this->updateAxes();
    this->updateProjection();
    this->updatePvm();
}

void GLWidget::setLightAngle(float angle)
{
    if(m_lightAngle != angle)
    {
        using namespace std;
        this->m_lightAngle = angle;
        this->m_lightDirection = QVector3D(sin(angle * M_PI / 180), 0.5, cos(angle * M_PI / 180));
    }
};

void GLWidget::updateAxes()
{
    auto newAxesData = this->axesData;
    for(int i = 0; i < 6; i++)
        for(int j = 0; j < 3; j++)
            newAxesData[i * 6 + j] *= this->scaleDistance * this->scaleSize;
    this->makeCurrent();
    this->m_vaoAxes.bind();
    this->m_vboAxes.bind();

    this->m_vboAxes.allocate(newAxesData.data(), newAxesData.size() * sizeof(float));

    this->m_vboAxes.release();
    this->m_vaoAxes.release();
    this->doneCurrent();
}

void GLWidget::resizeGL(int w, int h)
{
    this->updateAxes();
    this->glViewport(0, 0, w, h);
    this->aspectRatio = float(w) / float(h);
    this->scaleDistance = float(h) / float(this->baseHeight);

    this->updateAxes();
    this->updateView();
    this->updateProjection();
    this->updatePvm();
}

void GLWidget::updateCamera()
{
    QMatrix4x4 m_pvm = this->projection_ * this->m_view * this->m_model;

    this->m_prog.bind();
    this->m_prog.setUniformValue("mvpMatrix", this->m_pvm);
    this->m_prog.setUniformValue("modelMatrix", this->m_model);
    this->m_prog.setUniformValue("basePosition", this->m_center);
    this->m_prog.release();

    this->m_progAxes.bind();
    this->m_progAxes.setUniformValue("mvpMatrix", m_pvm);
    this->m_progAxes.release();
}

void GLWidget::setBgColor(QColor color)
{
    this->makeCurrent();
    this->glClearColor(color.redF(), color.greenF(), color.blueF(), color.alphaF());
    this->doneCurrent();
}

void GLWidget::paintGL()
{
    this->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if(this->m_vertexCount == 0)
        return;

    this->updateCamera();

    this->m_prog.bind();
    this->m_vao.bind();

    this->m_prog.setUniformValue("lightPosition", this->m_lightDirection * (this->cameraDistance() * 2));
    this->m_prog.setUniformValue("lightDir", this->m_lightDirection);
    this->m_prog.setUniformValue("viewPosition", QVector3D(0, 0, this->cameraDistance()));
    this->m_prog.setUniformValue("lightColor", this->m_lightColor);
    this->m_prog.setUniformValue("ambientStrength", this->m_ambientStrength);
    this->m_prog.setUniformValue("specularStrength", this->m_specularStrength);
    this->m_prog.setUniformValue("shininess", this->m_shininess);
    this->m_prog.setUniformValue("lightMode", this->m_lightMode);

    if(this->needFaces)
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        this->m_prog.setUniformValue("fillColor", this->facesColor);
        this->glDrawArrays(GL_TRIANGLES, 0, m_vertexCount);
    }

    if(this->needEdges)
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        this->m_prog.setUniformValue("fillColor", this->edgesColor);
        this->glLineWidth(1.5f);
        this->glDrawArrays(GL_TRIANGLES, 0, this->m_vertexCount);
    }
    this->m_vao.release();
    this->m_prog.release();

    if(this->needAxes)
    {
        this->m_progAxes.bind();
        this->m_vaoAxes.bind();

        this->glLineWidth(1.5f);
        this->glDrawArrays(GL_LINES, 0, 6);

        this->m_vaoAxes.release();
        this->m_progAxes.release();
    }
    // this->glEnable(GL_CULL_FACE);
};

GLWidget::~GLWidget()
{
    makeCurrent();
    m_vbo.destroy();
    m_vao.destroy();
    doneCurrent();
}
