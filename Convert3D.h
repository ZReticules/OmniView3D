#ifndef CONVERT3D_H
#define CONVERT3D_H

#include <qvector4d.h>

namespace Convert3D
{

using std::sin;
using std::cos;
using std::abs;

QVector3D ToPerspective(const QPoint& screenPos, const QSize& viewport,
                                    const QMatrix4x4& invViewProj, float fixedY)
{
    float nx = 2.0f * screenPos.x() / viewport.width() - 1.0f;
    float ny = 1.0f - 2.0f * screenPos.y() / viewport.height();

    QVector4D pNear = invViewProj * QVector4D(nx, ny, -1.0f, 1.0f);
    QVector4D pFar = invViewProj * QVector4D(nx, ny, 1.0f, 1.0f);
    if (qAbs(pNear.w()) > 1e-6f)
        pNear /= pNear.w();
    if (qAbs(pFar.w()) > 1e-6f)
        pFar /= pFar.w();

    QVector3D rayOrigin = pNear.toVector3D();
    QVector3D rayDir = (pFar.toVector3D() - rayOrigin).normalized();

    if (qAbs(rayDir.y()) < 1e-6f)
        return QVector3D(rayOrigin.x() + rayDir.x() * 5.0f, fixedY, rayOrigin.z() + rayDir.z() * 5.0f);

    float t = (fixedY - rayOrigin.y()) / rayDir.y();
    QVector3D hitPoint = rayOrigin + rayDir * t;

    hitPoint.setY(fixedY);
    return hitPoint;
}


QVector3D ToOrtho(const QPoint& screenPos, float cameraDistance, float sizeScale, float rotDeg, float tiltDeg)
{

    float dx = float(screenPos.x()) / sizeScale * cameraDistance;
    float dy = float(screenPos.y()) / sizeScale * cameraDistance;
    dy = abs(tiltDeg) >= 0.01 ? dy / sin(tiltDeg * M_PI / 180) : 0;
    return QVector3D(
        (-dx * cos(rotDeg * M_PI / 180) + -dy * sin(rotDeg * M_PI / 180)), 0.0,
        (dx * sin(rotDeg * M_PI / 180 ) + -dy * cos(rotDeg * M_PI / 180)));
};

}

#endif // CONVERT3D_H
