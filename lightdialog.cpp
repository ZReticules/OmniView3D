#include "lightdialog.h"
#include "ui_lightdialog.h"

#include <QPushButton>
#include <QColorDialog>

#include "IconTools.h"

LightDialog::LightDialog(QWidget *parent, LightContext& context)
    : QDialog(parent)
    , ui(new Ui::LightDialog), context(context)
{
    ui->setupUi(this);
    buttons = {ui->radioOff, ui->radioFlat, ui->radioSpecular};

    QObject::connect(ui->radioOff, &QRadioButton::clicked, [this](bool checked=false){
        this->ui->spinAmbient->setEnabled(false);
        this->ui->spinShine->setEnabled(false);
        this->ui->spinSpec->setEnabled(false);
        this->ui->dialPos->setEnabled(false);
        this->ui->btColor->setEnabled(false);
        this->ui->sliderHeight->setEnabled(false);
        if(this->context.mode != GLWidget::OFF)
            emit this->modeSwitched(GLWidget::OFF);
    });

    QObject::connect(ui->radioFlat, &QRadioButton::clicked, [this](bool checked=false){
        this->ui->spinAmbient->setEnabled(true);
        this->ui->spinShine->setEnabled(false);
        this->ui->spinSpec->setEnabled(false);
        this->ui->dialPos->setEnabled(true);
        this->ui->btColor->setEnabled(true);
        this->ui->sliderHeight->setEnabled(true);
        if(this->context.mode != GLWidget::FLAT)
            emit this->modeSwitched(GLWidget::FLAT);
    });

    QObject::connect(ui->radioSpecular, &QRadioButton::clicked, [this](bool checked=false){
        this->ui->spinAmbient->setEnabled(true);
        this->ui->spinShine->setEnabled(true);
        this->ui->spinSpec->setEnabled(true);
        this->ui->dialPos->setEnabled(true);
        this->ui->btColor->setEnabled(true);
        this->ui->sliderHeight->setEnabled(true);
        if(this->context.mode != GLWidget::SPECULAR)
            emit this->modeSwitched(GLWidget::SPECULAR);
    });

    QObject::connect(ui->dialPos, &QDial::valueChanged, [this](int value){
        this->ui->labelPos->setText(QString::asprintf("Light direction: %d deg", value));
        if(this->context.ambient != value)
            emit this->angleChanged(value);
    });

    QObject::connect(&this->colorDialog, &QColorDialog::currentColorChanged, [this](const QColor &newColor){
        IconTools::IconFill(this->ui->btColor, newColor);
        if(this->context.color != newColor)
            emit this->colorChanged(newColor);
    });

    QObject::connect(&this->colorDialog, &QColorDialog::rejected, [this](){
        IconTools::IconFill(this->ui->btColor, this->prevColor);
        if(this->context.color != this->prevColor)
            emit this->colorChanged(this->prevColor);
    });

    QObject::connect(ui->btColor, &QToolButton::clicked, [this] (bool checked){
        this->prevColor = this->context.color;
        this->colorDialog.setCurrentColor(this->prevColor);
        this->colorDialog.show();
    });

    QObject::connect(ui->spinAmbient, &QDoubleSpinBox::valueChanged, [this](float value){ emit this->ambientChanged(value); });
    QObject::connect(ui->spinShine, &QDoubleSpinBox::valueChanged, [this](float value){ emit this->shininessChanged(value); });
    QObject::connect(ui->spinSpec, &QDoubleSpinBox::valueChanged, [this](float value){ emit this->specularChanged(value); });

    QObject::connect(ui->sliderHeight, &QSlider::valueChanged, [this](int value){
        float fvalue = (float)value / 10;
        this->ui->labelHeight->setText(QString::asprintf("Light height %4.1f", fvalue));
        if(this->context.height != fvalue)
            emit this->heightChanged(fvalue);
    });
}

void LightDialog::loadContext()
{
    emit this->buttons[this->context.mode]->setChecked(true);
    emit this->buttons[this->context.mode]->clicked(true);
    this->ui->spinAmbient->setValue(this->context.ambient);
    this->ui->spinShine->setValue(this->context.shininess);
    this->ui->spinSpec->setValue(this->context.specularStrength);
    IconTools::IconFill(this->ui->btColor, this->context.color);
    this->ui->dialPos->setValue(this->context.angle);
    this->ui->sliderHeight->setValue((int)(this->context.height * 10));
}

void LightDialog::show()
{
    this->loadContext();
    this->QDialog::show();
}

LightDialog::~LightDialog()
{
    delete ui;
}
