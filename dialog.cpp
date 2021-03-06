#include "dialog.h"
#include "ui_dialog.h"

Dialog::Dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog)
{
    ui->setupUi(this);
    sliderActivation(true);
    ui->spinBox_R->setRange(0,100);
    ui->spinBox_G->setRange(0,100);
    ui->spinBox_B->setRange(0,100);

    ui->spinBox_Y->setRange(0,100);
    ui->spinBox_U->setRange(-100,100);
    ui->spinBox_V->setRange(-100,100);

    ui->spinBox_R->setValue(100);
    ui->spinBox_G->setValue(100);
    ui->spinBox_B->setValue(100);

    ui->spinBox_Y->setValue(100);
    ui->spinBox_U->setValue(100);
    ui->spinBox_V->setValue(100);

}

Dialog::~Dialog()
{
    delete ui;
}

void Dialog::sliderActivation(bool isRGBActive)
{
    if(isRGBActive)
    {
        ui->R->setEnabled(true);
        ui->G->setEnabled(true);
        ui->B->setEnabled(true);

        ui->spinBox_R->setEnabled(true);
        ui->spinBox_G->setEnabled(true);
        ui->spinBox_B->setEnabled(true);

        ui->Y->setEnabled(false);
        ui->U->setEnabled(false);
        ui->V->setEnabled(false);

        ui->spinBox_Y->setEnabled(false);
        ui->spinBox_U->setEnabled(false);
        ui->spinBox_V->setEnabled(false);

        ui->Y->setValue(100);
        ui->U->setValue(100);
        ui->V->setValue(100);
    }
    else
    {
        ui->R->setEnabled(false);
        ui->G->setEnabled(false);
        ui->B->setEnabled(false);

        ui->spinBox_R->setEnabled(false);
        ui->spinBox_G->setEnabled(false);
        ui->spinBox_B->setEnabled(false);

        ui->Y->setEnabled(true);
        ui->U->setEnabled(true);
        ui->V->setEnabled(true);

        ui->spinBox_Y->setEnabled(true);
        ui->spinBox_U->setEnabled(true);
        ui->spinBox_V->setEnabled(true);

        ui->R->setValue(100);
        ui->G->setValue(100);
        ui->B->setValue(100);
    }
}

bool Dialog::getIsRGBActive() const
{
    return isRGBActive;
}

void Dialog::setIsRGBActive(bool value)
{
    isRGBActive = value;
}

void Dialog::on_RGBradioButoon_clicked()
{
    sliderActivation(true);
}

void Dialog::on_YUVradioButton_clicked()
{
    sliderActivation(false);
}

void Dialog::on_pushButton_clicked()
{
   // isPreview = true;
    if(isRGBActive)
         rgbChanged(ui->R->value(), ui->G->value(),ui->B->value());
    else
         yuvChanged(ui->Y->value(),ui->U->value(),ui->V->value());
}

void Dialog::on_R_valueChanged(int action)
{

    rgbChanged(action,ui->G->value(),ui->B->value());
}

void Dialog::on_G_valueChanged(int action)
{
    rgbChanged(ui->R->value(),action,ui->B->value());
}

void Dialog::on_B_valueChanged(int action)
{
    rgbChanged(ui->B->value(),ui->G->value(),action);

}

void Dialog::on_Y_valueChanged(int action)
{
    yuvChanged(action,ui->U->value(),ui->V->value());
}

void Dialog::on_U_valueChanged(int action)
{
    yuvChanged(ui->Y->value(),action,ui->V->value());
}

void Dialog::on_V_valueChanged(int action)
{
    yuvChanged(ui->Y->value(),ui->U->value(),action);
}
