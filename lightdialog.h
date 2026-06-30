#ifndef LIGHTDIALOG_H
#define LIGHTDIALOG_H

#include <array>
#include <QDialog>
#include <QRadioButton>
#include <QColorDialog>
#include <QSettings>

#include "glwidget.h"
#include "IconTools.h"

namespace Ui {
class LightDialog;
}

class LightDialog : public QDialog
{
    Q_OBJECT

public:
    struct LightContext
    {
        GLWidget::LightMode mode;
        QColor color;
        float ambient, shininess, specularStrength, angle, height;

        void storeSettings(QSettings& settings)
        {
            settings.setValue("light/mode", mode);
            settings.setValue("light/color", color);
            settings.setValue("light/ambient", ambient);
            settings.setValue("light/shininess", shininess);
            settings.setValue("light/specularStrength", specularStrength);
            settings.setValue("light/angle", angle);
            settings.setValue("light/height", height);
        }

        void loadSettings(const QSettings& settings)
        {
            mode = (GLWidget::LightMode)settings.value("light/mode", mode).toInt();
            color = settings.value("light/color", color).value<QColor>();
            ambient = settings.value("light/ambient", ambient).toFloat();
            shininess = settings.value("light/shininess", shininess).toFloat();
            specularStrength = settings.value("light/specularStrength", specularStrength).toFloat();
            angle = settings.value("light/angle", angle).toFloat();
            height = settings.value("light/height", height).toFloat();
        }
    };

public:
    explicit LightDialog(QWidget *parent, LightContext& context);
    ~LightDialog();
    void show();

signals:
    void modeSwitched(GLWidget::LightMode mode);
    void colorChanged(QColor color);
    void ambientChanged(float ambient);
    void shininessChanged(float shininess);
    void specularChanged(float specularStrength);
    void angleChanged(float angle);
    void heightChanged(float height);

private:
    Ui::LightDialog *ui;
    IconTools::IconizedColorDialog colorDialog = IconTools::IconizedColorDialog(this);
    LightContext& context;
    QColor prevColor;
    std::array<QRadioButton*, 3> buttons;

private:
    void loadContext();
};

#endif // LIGHTDIALOG_H
